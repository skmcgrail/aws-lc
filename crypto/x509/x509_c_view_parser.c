// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "x509_view.h"

#include <limits.h>

#include <openssl/bytestring.h>

#include "../bytestring/internal.h"
#include "../internal.h"

// The view parser is built on CBS, which reads DER without allocating and
// returns sub-views of the input. It records each field as an
// |AWSLC_X509_DER_RANGE| relative to the start of the input.
//
// Its accepted language must remain a subset of the legacy template decoder's,
// so every check below mirrors one the legacy decoder or a deferred
// materialization step performs. Anything outside that subset is rejected and
// left to the compatibility fallback in d2i_X509.

#define VIEW_TAG_VERSION (CBS_ASN1_CONTEXT_SPECIFIC | CBS_ASN1_CONSTRUCTED | 0)
#define VIEW_TAG_ISSUER_UID (CBS_ASN1_CONTEXT_SPECIFIC | 1)
#define VIEW_TAG_SUBJECT_UID (CBS_ASN1_CONTEXT_SPECIFIC | 2)
#define VIEW_TAG_EXTENSIONS \
  (CBS_ASN1_CONTEXT_SPECIFIC | CBS_ASN1_CONSTRUCTED | 3)

// Keep the view parser within the bound enforced by x_name.c's legacy decoder.
#define VIEW_X509_NAME_MAX (1024u * 1024u)

// ASN1_mbstring_ncopy rejects a byte-order mark as the first character of a
// BMPString or UniversalString, and X509_NAME canonicalization always runs it
// for these types. Reject it here too: otherwise d2i_X509 accepts a
// certificate whose issuer or subject can never be materialized.
#define VIEW_BYTE_ORDER_MARK 0xfeffu

static AWSLC_X509_DER_RANGE view_range(const uint8_t *root, const CBS *cbs) {
  AWSLC_X509_DER_RANGE range = {(uint32_t)(CBS_data(cbs) - root),
                                (uint32_t)CBS_len(cbs)};
  return range;
}

// view_get_asn1 reads a DER element with tag |tag| from |cbs|. It sets
// |*out_body| to the contents and, if |out_element| is non-NULL,
// |*out_element| to the whole element. CBS reports only success or failure, so
// a failure is classified here for the fallback counters in x_x509.c.
static uint32_t view_get_asn1(CBS *cbs, CBS *out_body, CBS *out_element,
                              CBS_ASN1_TAG tag) {
  const CBS start = *cbs;
  if (!CBS_get_asn1(cbs, out_body, tag)) {
    if (CBS_len(&start) == 0) {
      return AWSLC_X509_PARSE_TRUNCATED;
    }
    return CBS_peek_asn1_tag(&start, tag) ? AWSLC_X509_PARSE_INVALID_LENGTH
                                          : AWSLC_X509_PARSE_INVALID_TAG;
  }
  if (out_element != NULL) {
    CBS_init(out_element, CBS_data(&start), CBS_len(&start) - CBS_len(cbs));
  }
  return AWSLC_X509_PARSE_OK;
}

static int is_valid_time(CBS_ASN1_TAG tag, const CBS *body) {
  // Only the RFC 5280 forms are accepted. The legacy decoder also tolerates
  // UTCTime timezone offsets; those certificates take the fallback path.
  switch (tag) {
    case CBS_ASN1_UTCTIME:
      return CBS_parse_utc_time(body, NULL, /*allow_timezone_offset=*/0);
    case CBS_ASN1_GENERALIZEDTIME:
      return CBS_parse_generalized_time(body, NULL,
                                        /*allow_timezone_offset=*/0);
    default:
      return 0;
  }
}

// is_valid_any reports whether an AlgorithmIdentifier parameter with tag |tag|
// and contents |body| is accepted by the legacy ASN1_ANY decoder.
static int is_valid_any(CBS_ASN1_TAG tag, const CBS *body) {
  // ASN1_ANY stores non-universal values as V_ASN1_OTHER in encoded form.
  if ((tag & CBS_ASN1_CLASS_MASK) != CBS_ASN1_UNIVERSAL) {
    return 1;
  }

  switch (tag) {
    case CBS_ASN1_BOOLEAN:
      return CBS_len(body) == 1;
    case CBS_ASN1_INTEGER:
    case CBS_ASN1_ENUMERATED:
      return CBS_is_valid_asn1_integer(body, NULL);
    case CBS_ASN1_BITSTRING:
      return CBS_is_valid_asn1_bitstring(body);
    case CBS_ASN1_NULL:
      return CBS_len(body) == 0;
    case CBS_ASN1_OBJECT:
      return CBS_is_valid_asn1_oid(body);
    case CBS_ASN1_SEQUENCE & ~CBS_ASN1_CONSTRUCTED:
    case CBS_ASN1_SET & ~CBS_ASN1_CONSTRUCTED:
      return 0;
    case CBS_ASN1_UTCTIME:
    case CBS_ASN1_GENERALIZEDTIME:
      return is_valid_time(tag, body);
    case CBS_ASN1_UNIVERSALSTRING:
      return CBS_len(body) % 4 == 0;
    case CBS_ASN1_BMPSTRING:
      return CBS_len(body) % 2 == 0;
    case CBS_ASN1_SEQUENCE:
    case CBS_ASN1_SET:
      return 1;
    default:
      // The legacy decoder accepts other primitive universal values as
      // ASN1_STRINGs. High tag numbers are not needed by AlgorithmIdentifier,
      // and constructed strings need recursive BER collection, so leave both to
      // the compatibility fallback.
      return (tag & CBS_ASN1_CONSTRUCTED) == 0 && tag < 0x1f;
  }
}

static int is_valid_algorithm(CBS algorithm) {
  CBS oid;
  if (!CBS_get_asn1(&algorithm, &oid, CBS_ASN1_OBJECT) ||
      !CBS_is_valid_asn1_oid(&oid)) {
    return 0;
  }
  if (CBS_len(&algorithm) != 0) {
    CBS parameters;
    CBS_ASN1_TAG tag;
    if (!CBS_get_any_asn1(&algorithm, &parameters, &tag) ||
        !is_valid_any(tag, &parameters)) {
      return 0;
    }
  }
  return CBS_len(&algorithm) == 0;
}

// is_valid_string reports whether |str| decodes as a sequence of code points
// with |get_char|. If |reject_bom| is non-zero, a leading byte-order mark is
// rejected.
static int is_valid_string(CBS str, int (*get_char)(CBS *, uint32_t *),
                           int reject_bom) {
  for (int first = 1; CBS_len(&str) != 0; first = 0) {
    uint32_t c;
    if (!get_char(&str, &c) ||
        (reject_bom && first && c == VIEW_BYTE_ORDER_MARK)) {
      return 0;
    }
  }
  return 1;
}

static int is_valid_name_value(CBS_ASN1_TAG tag, const CBS *body) {
  // X509_NAME_ENTRY uses ASN1_PRINTABLE, an OpenSSL-specific MSTRING. Keep
  // this list in sync with B_ASN1_PRINTABLE and tag2bit in tasn_dec.c. Only
  // primitive string encodings are accepted, except for SEQUENCE.
  switch (tag) {
    case CBS_ASN1_BITSTRING:
      return CBS_is_valid_asn1_bitstring(body);
    case CBS_ASN1_ENUMERATED:  // Maps to B_ASN1_UNKNOWN.
      return CBS_is_valid_asn1_integer(body, NULL);
    case CBS_ASN1_UTF8STRING:
      return is_valid_string(*body, cbs_get_utf8, /*reject_bom=*/0);
    case CBS_ASN1_UNIVERSALSTRING:
      return is_valid_string(*body, cbs_get_utf32_be, /*reject_bom=*/1);
    case CBS_ASN1_BMPSTRING:
      return is_valid_string(*body, cbs_get_ucs2_be, /*reject_bom=*/1);
    case CBS_ASN1_SEQUENCE:
    case 0x07:  // ObjectDescriptor
    case 0x08:  // EXTERNAL
    case 0x09:  // REAL
    case 0x0b:  // EMBEDDED PDV
    case 0x0d:  // RELATIVE-OID
    case 0x0e:  // TIME
    case 0x0f:  // Reserved
    case CBS_ASN1_NUMERICSTRING:
    case CBS_ASN1_PRINTABLESTRING:
    case CBS_ASN1_T61STRING:
    case CBS_ASN1_IA5STRING:
    case 0x1d:  // CHARACTER STRING
      return 1;
    default:
      return 0;
  }
}

// der_less_or_equal reports whether DER element |a| sorts at or before |b| in
// a DER SET OF. X.690 compares encodings after zero-padding the shorter one;
// for equal tags, unequal lengths differ in the length octets before padding
// can affect the result.
static int der_less_or_equal(const CBS *a, const CBS *b) {
  const size_t a_len = CBS_len(a), b_len = CBS_len(b);
  const int order =
      OPENSSL_memcmp(CBS_data(a), CBS_data(b), a_len < b_len ? a_len : b_len);
  return order < 0 || (order == 0 && a_len <= b_len);
}

static int is_valid_rdn(CBS rdn) {
  if (CBS_len(&rdn) == 0) {
    return 0;
  }
  CBS previous;
  int have_previous = 0;
  while (CBS_len(&rdn) != 0) {
    CBS attribute, element, oid, value;
    CBS_ASN1_TAG value_tag;
    if (view_get_asn1(&rdn, &attribute, &element, CBS_ASN1_SEQUENCE) !=
            AWSLC_X509_PARSE_OK ||
        (have_previous && !der_less_or_equal(&previous, &element)) ||
        !CBS_get_asn1(&attribute, &oid, CBS_ASN1_OBJECT) ||
        !CBS_is_valid_asn1_oid(&oid) ||
        !CBS_get_any_asn1(&attribute, &value, &value_tag) ||
        !is_valid_name_value(value_tag, &value) || CBS_len(&attribute) != 0) {
      return 0;
    }
    previous = element;
    have_previous = 1;
  }
  return 1;
}

static int is_valid_name(const CBS *element, CBS name) {
  if (CBS_len(element) > VIEW_X509_NAME_MAX) {
    return 0;
  }
  while (CBS_len(&name) != 0) {
    CBS rdn;
    if (!CBS_get_asn1(&name, &rdn, CBS_ASN1_SET) || !is_valid_rdn(rdn)) {
      return 0;
    }
  }
  return 1;
}

static int is_valid_validity(CBS validity) {
  for (int i = 0; i < 2; i++) {
    CBS time;
    CBS_ASN1_TAG tag;
    if (!CBS_get_any_asn1(&validity, &time, &tag) ||
        !is_valid_time(tag, &time)) {
      return 0;
    }
  }
  return CBS_len(&validity) == 0;
}

static const uint8_t kNetscapeCertTypeOID[] = {0x60, 0x86, 0x48, 0x01, 0x86,
                                               0xf8, 0x42, 0x01, 0x01};

// cached_extension_slot returns the |AWSLC_X509_EXTENSION_*| slot for |oid|,
// or -1 if x509v3_cache_extensions does not cache it.
static int cached_extension_slot(const CBS *oid) {
  if (CBS_len(oid) == 3 && CBS_data(oid)[0] == 0x55 &&
      CBS_data(oid)[1] == 0x1d) {
    switch (CBS_data(oid)[2]) {
      case 0x13:
        return AWSLC_X509_EXTENSION_BASIC_CONSTRAINTS;
      case 0x0f:
        return AWSLC_X509_EXTENSION_KEY_USAGE;
      case 0x25:
        return AWSLC_X509_EXTENSION_EXTENDED_KEY_USAGE;
      case 0x0e:
        return AWSLC_X509_EXTENSION_SUBJECT_KEY_IDENTIFIER;
      case 0x23:
        return AWSLC_X509_EXTENSION_AUTHORITY_KEY_IDENTIFIER;
      case 0x11:
        return AWSLC_X509_EXTENSION_SUBJECT_ALT_NAME;
      case 0x1e:
        return AWSLC_X509_EXTENSION_NAME_CONSTRAINTS;
      case 0x1f:
        return AWSLC_X509_EXTENSION_CRL_DISTRIBUTION_POINTS;
      default:
        return -1;
    }
  }
  if (CBS_mem_equal(oid, kNetscapeCertTypeOID, sizeof(kNetscapeCertTypeOID))) {
    return AWSLC_X509_EXTENSION_NETSCAPE_CERT_TYPE;
  }
  return -1;
}

// supported_critical_extension mirrors X509_supported_extension.
static int supported_critical_extension(const CBS *oid) {
  if (CBS_len(oid) == 3 && CBS_data(oid)[0] == 0x55 &&
      CBS_data(oid)[1] == 0x1d) {
    switch (CBS_data(oid)[2]) {
      case 0x0f:
      case 0x11:
      case 0x13:
      case 0x20:
      case 0x1f:
      case 0x25:
      case 0x24:
      case 0x1e:
      case 0x21:
      case 0x36:
        return 1;
      default:
        return 0;
    }
  }
  return CBS_mem_equal(oid, kNetscapeCertTypeOID, sizeof(kNetscapeCertTypeOID));
}

static int parse_extension(CBS extension, const uint8_t *root,
                           AWSLC_X509_CERTIFICATE_VIEW *view) {
  CBS oid, value;
  int critical = 0;
  if (!CBS_get_asn1(&extension, &oid, CBS_ASN1_OBJECT) ||
      !CBS_is_valid_asn1_oid(&oid) ||
      // DER forbids encoding the DEFAULT FALSE value of |critical|.
      (CBS_peek_asn1_tag(&extension, CBS_ASN1_BOOLEAN) &&
       (!CBS_get_asn1_bool(&extension, &critical) || !critical)) ||
      !CBS_get_asn1(&extension, &value, CBS_ASN1_OCTETSTRING) ||
      CBS_len(&extension) != 0) {
    return 0;
  }

  const int slot = cached_extension_slot(&oid);
  if (slot >= 0) {
    const uint32_t present = 1u << (uint32_t)slot;
    if ((view->extension_flags & present) != 0) {
      view->extension_flags |=
          1u << (AWSLC_X509_EXTENSION_DUPLICATE_SHIFT + (uint32_t)slot);
    } else {
      view->extension_flags |= present;
      view->extension_values[slot] = view_range(root, &value);
      if (critical) {
        view->extension_flags |=
            1u << (AWSLC_X509_EXTENSION_CRITICAL_SHIFT + (uint32_t)slot);
      }
    }
  }
  if (critical && !supported_critical_extension(&oid)) {
    view->extension_flags |= AWSLC_X509_EXTENSION_UNSUPPORTED_CRITICAL;
  }
  return 1;
}

static int parse_extensions(CBS explicit_extensions, const uint8_t *root,
                            AWSLC_X509_CERTIFICATE_VIEW *view) {
  CBS extensions;
  if (!CBS_get_asn1(&explicit_extensions, &extensions, CBS_ASN1_SEQUENCE) ||
      CBS_len(&explicit_extensions) != 0) {
    return 0;
  }
  while (CBS_len(&extensions) != 0) {
    CBS extension;
    if (!CBS_get_asn1(&extensions, &extension, CBS_ASN1_SEQUENCE) ||
        !parse_extension(extension, root, view)) {
      return 0;
    }
  }
  return 1;
}

static uint32_t parse_tbs(CBS tbs, const uint8_t *root,
                          AWSLC_X509_CERTIFICATE_VIEW *view) {
  CBS body, element;
  uint32_t result;

  if (CBS_peek_asn1_tag(&tbs, VIEW_TAG_VERSION)) {
    uint64_t version;
    result = view_get_asn1(&tbs, &body, NULL, VIEW_TAG_VERSION);
    if (result != AWSLC_X509_PARSE_OK) {
      return result;
    }
    if (!CBS_get_asn1_uint64(&body, &version) || CBS_len(&body) != 0 ||
        version > 2) {
      return AWSLC_X509_PARSE_INVALID_VERSION;
    }
    view->version = (uint8_t)version;
  }

  result = view_get_asn1(&tbs, &body, &element, CBS_ASN1_INTEGER);
  if (result != AWSLC_X509_PARSE_OK) {
    return result;
  }
  if (!CBS_is_valid_asn1_integer(&body, NULL)) {
    return AWSLC_X509_PARSE_INVALID_VALUE;
  }
  view->serial = view_range(root, &element);

  result = view_get_asn1(&tbs, &body, &element, CBS_ASN1_SEQUENCE);
  if (result != AWSLC_X509_PARSE_OK) {
    return result;
  }
  if (!is_valid_algorithm(body)) {
    return AWSLC_X509_PARSE_INVALID_ALGORITHM;
  }
  view->tbs_signature_algorithm = view_range(root, &element);

  result = view_get_asn1(&tbs, &body, &element, CBS_ASN1_SEQUENCE);
  if (result != AWSLC_X509_PARSE_OK) {
    return result;
  }
  if (!is_valid_name(&element, body)) {
    return AWSLC_X509_PARSE_INVALID_NAME;
  }
  view->issuer = view_range(root, &element);

  result = view_get_asn1(&tbs, &body, &element, CBS_ASN1_SEQUENCE);
  if (result != AWSLC_X509_PARSE_OK) {
    return result;
  }
  if (!is_valid_validity(body)) {
    return AWSLC_X509_PARSE_INVALID_TIME;
  }
  view->validity = view_range(root, &element);

  result = view_get_asn1(&tbs, &body, &element, CBS_ASN1_SEQUENCE);
  if (result != AWSLC_X509_PARSE_OK) {
    return result;
  }
  if (!is_valid_name(&element, body)) {
    return AWSLC_X509_PARSE_INVALID_NAME;
  }
  view->subject = view_range(root, &element);

  CBS spki;
  result = view_get_asn1(&tbs, &spki, &element, CBS_ASN1_SEQUENCE);
  if (result != AWSLC_X509_PARSE_OK) {
    return result;
  }
  view->spki = view_range(root, &element);
  result = view_get_asn1(&spki, &body, NULL, CBS_ASN1_SEQUENCE);
  if (result != AWSLC_X509_PARSE_OK) {
    return result;
  }
  if (!is_valid_algorithm(body)) {
    return AWSLC_X509_PARSE_INVALID_ALGORITHM;
  }
  result = view_get_asn1(&spki, &body, NULL, CBS_ASN1_BITSTRING);
  if (result != AWSLC_X509_PARSE_OK) {
    return result;
  }
  if (!CBS_is_valid_asn1_bitstring(&body)) {
    return AWSLC_X509_PARSE_INVALID_VALUE;
  }
  if (CBS_len(&spki) != 0) {
    return AWSLC_X509_PARSE_TRAILING_DATA;
  }

  if (CBS_peek_asn1_tag(&tbs, VIEW_TAG_ISSUER_UID)) {
    result = view_get_asn1(&tbs, &body, &element, VIEW_TAG_ISSUER_UID);
    if (result != AWSLC_X509_PARSE_OK) {
      return result;
    }
    if (!CBS_is_valid_asn1_bitstring(&body)) {
      return AWSLC_X509_PARSE_INVALID_VALUE;
    }
    view->flags |= AWSLC_X509_FLAG_ISSUER_UID;
    view->issuer_uid = view_range(root, &element);
  }
  if (CBS_peek_asn1_tag(&tbs, VIEW_TAG_SUBJECT_UID)) {
    result = view_get_asn1(&tbs, &body, &element, VIEW_TAG_SUBJECT_UID);
    if (result != AWSLC_X509_PARSE_OK) {
      return result;
    }
    if (!CBS_is_valid_asn1_bitstring(&body)) {
      return AWSLC_X509_PARSE_INVALID_VALUE;
    }
    view->flags |= AWSLC_X509_FLAG_SUBJECT_UID;
    view->subject_uid = view_range(root, &element);
  }
  if (CBS_peek_asn1_tag(&tbs, VIEW_TAG_EXTENSIONS)) {
    result = view_get_asn1(&tbs, &body, &element, VIEW_TAG_EXTENSIONS);
    if (result != AWSLC_X509_PARSE_OK) {
      return result;
    }
    if (!parse_extensions(body, root, view)) {
      return AWSLC_X509_PARSE_INVALID_EXTENSIONS;
    }
    view->flags |= AWSLC_X509_FLAG_EXTENSIONS;
    view->extensions = view_range(root, &element);
  }

  if (CBS_len(&tbs) != 0) {
    return AWSLC_X509_PARSE_TRAILING_DATA;
  }
  if (view->version == 0 &&
      (view->flags &
       (AWSLC_X509_FLAG_ISSUER_UID | AWSLC_X509_FLAG_SUBJECT_UID)) != 0) {
    return AWSLC_X509_PARSE_INVALID_FIELD_FOR_VERSION;
  }
  if (view->version != 2 && (view->flags & AWSLC_X509_FLAG_EXTENSIONS) != 0) {
    return AWSLC_X509_PARSE_INVALID_FIELD_FOR_VERSION;
  }
  return AWSLC_X509_PARSE_OK;
}

uint32_t x509_parse_der_view(const uint8_t *der, size_t der_len, uint8_t exact,
                             AWSLC_X509_CERTIFICATE_VIEW *out) {
  if (out == NULL || (der == NULL && der_len != 0)) {
    return AWSLC_X509_PARSE_NULL_POINTER;
  }
  if (der_len > UINT32_MAX || der_len > INT_MAX / 2) {
    return AWSLC_X509_PARSE_INPUT_TOO_LARGE;
  }

  AWSLC_X509_CERTIFICATE_VIEW view;
  OPENSSL_memset(&view, 0, sizeof(view));

  CBS input, certificate, cert_element, tbs, body, element;
  CBS_init(&input, der, der_len);
  uint32_t result =
      view_get_asn1(&input, &certificate, &cert_element, CBS_ASN1_SEQUENCE);
  if (result != AWSLC_X509_PARSE_OK) {
    return result;
  }
  view.certificate = view_range(der, &cert_element);

  result = view_get_asn1(&certificate, &tbs, &element, CBS_ASN1_SEQUENCE);
  if (result != AWSLC_X509_PARSE_OK) {
    return result;
  }
  view.tbs_certificate = view_range(der, &element);
  result = parse_tbs(tbs, der, &view);
  if (result != AWSLC_X509_PARSE_OK) {
    return result;
  }

  result = view_get_asn1(&certificate, &body, &element, CBS_ASN1_SEQUENCE);
  if (result != AWSLC_X509_PARSE_OK) {
    return result;
  }
  if (!is_valid_algorithm(body)) {
    return AWSLC_X509_PARSE_INVALID_ALGORITHM;
  }
  view.signature_algorithm = view_range(der, &element);

  result = view_get_asn1(&certificate, &body, &element, CBS_ASN1_BITSTRING);
  if (result != AWSLC_X509_PARSE_OK) {
    return result;
  }
  if (!CBS_is_valid_asn1_bitstring(&body)) {
    return AWSLC_X509_PARSE_INVALID_VALUE;
  }
  view.signature = view_range(der, &element);

  if (CBS_len(&certificate) != 0 || (exact && CBS_len(&input) != 0)) {
    return AWSLC_X509_PARSE_TRAILING_DATA;
  }
  *out = view;
  return AWSLC_X509_PARSE_OK;
}

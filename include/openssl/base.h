/* ====================================================================
 * Copyright (c) 1998-2001 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com). */

#ifndef OPENSSL_HEADER_BASE_H
#define OPENSSL_HEADER_BASE_H


/**
 * @file
 * @brief This file should be the first included by all AWS-LC headers.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

#if defined(__MINGW32__)
// stdio.h is needed on MinGW for __MINGW_PRINTF_FORMAT.
#include <stdio.h>
#endif

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

// Include an AWS-LC-only header so consumers including this header without
// setting up include paths do not accidentally pick up the system
// opensslconf.h.
#include <openssl/is_awslc.h>
#include <openssl/opensslconf.h>
#include <openssl/target.h>  // IWYU pragma: export

#include <openssl/boringssl_prefix_symbols.h>

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(BORINGSSL_FIPS)
#define AWSLC_FIPS
#endif

#if defined(__APPLE__)
// Note |TARGET_OS_MAC| is set for all Apple OS variants. |TARGET_OS_OSX|
// targets macOS specifically.
#if defined(TARGET_OS_OSX) && TARGET_OS_OSX
#define OPENSSL_MACOS
#endif
#if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#define OPENSSL_IOS
#endif
#endif

#define AWSLC_VERSION_NAME "AWS-LC"
#define OPENSSL_IS_AWSLC
// |OPENSSL_VERSION_NUMBER| should match the version number in opensslv.h.
#define OPENSSL_VERSION_NUMBER 0x1010107f
#define SSLEAY_VERSION_NUMBER OPENSSL_VERSION_NUMBER

// BORINGSSL_API_VERSION is replaced with AWSLC_API_VERSION to avoid users interpreting AWSLC as BoringSSL.
// Below are BoringSSL's comments on BORINGSSL_API_VERSION.
// BORINGSSL_API_VERSION is a positive integer that increments as BoringSSL
// changes over time. The value itself is not meaningful. It will be incremented
// whenever is convenient to coordinate an API change with consumers. This will
// not denote any special point in development.
//
// A consumer may use this symbol in the preprocessor to temporarily build
// against multiple revisions of BoringSSL at the same time. It is not
// recommended to do so for longer than is necessary.
#define AWSLC_API_VERSION 35

// This string tracks the most current production release version on Github
// https://github.com/aws/aws-lc/releases.
// When bumping the encoded version number, also update the test fixture:
// ServiceIndicatorTest.AWSLCVersionString
// Note: there are two versions of this test. Only one test is compiled
// depending on FIPS mode.
#define AWSLC_VERSION_NUMBER_STRING "1.69.0"

#if defined(BORINGSSL_SHARED_LIBRARY)

#if defined(OPENSSL_WINDOWS)

#if defined(BORINGSSL_IMPLEMENTATION)
#define OPENSSL_EXPORT __declspec(dllexport)
#else
#define OPENSSL_EXPORT __declspec(dllimport)
#endif

#else  // defined(OPENSSL_WINDOWS)

#if defined(BORINGSSL_IMPLEMENTATION)
#define OPENSSL_EXPORT __attribute__((visibility("default")))
#else
#define OPENSSL_EXPORT
#endif

#endif  // defined(OPENSSL_WINDOWS)

#else  // defined(BORINGSSL_SHARED_LIBRARY)

#if defined(OPENSSL_WINDOWS)
#define OPENSSL_EXPORT
#else
#define OPENSSL_EXPORT __attribute__((visibility("default")))
#endif

#endif  // defined(BORINGSSL_SHARED_LIBRARY)

#if !defined(OPENSSL_WARN_UNUSED_RESULT)
// This should only affect internal usage of functions
#if defined(BORINGSSL_IMPLEMENTATION) || defined(AWS_LC_TEST_ENV)
#if defined(__GNUC__) || defined(__clang__)
# define OPENSSL_WARN_UNUSED_RESULT __attribute__ ((warn_unused_result))
#elif defined(_MSC_VER)
# define OPENSSL_WARN_UNUSED_RESULT _Check_return_
#else
# define OPENSSL_WARN_UNUSED_RESULT
#endif
#else
// The macro is ignored by consumers
# define OPENSSL_WARN_UNUSED_RESULT
#endif
#endif

#if defined(_MSC_VER)

// OPENSSL_DEPRECATED is used to mark a function as deprecated. Use
// of any functions so marked in caller code will produce a warning.
// OPENSSL_BEGIN_ALLOW_DEPRECATED and OPENSSL_END_ALLOW_DEPRECATED
// can be used to suppress the warning in regions of caller code.
#define OPENSSL_DEPRECATED __declspec(deprecated)
#define OPENSSL_BEGIN_ALLOW_DEPRECATED \
  __pragma(warning(push)) __pragma(warning(disable : 4996))
#define OPENSSL_END_ALLOW_DEPRECATED __pragma(warning(pop))

#elif (defined(__GNUC__) && ((__GNUC__ > 4) ||  (__GNUC_MINOR__ >= 6))) || defined(__clang__)
// `_Pragma("GCC diagnostic push")` was added in GCC 4.6
// http://gcc.gnu.org/gcc-4.6/changes.html
#define OPENSSL_DEPRECATED __attribute__((__deprecated__))
#define OPENSSL_BEGIN_ALLOW_DEPRECATED \
  _Pragma("GCC diagnostic push")       \
      _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
#define OPENSSL_END_ALLOW_DEPRECATED _Pragma("GCC diagnostic pop")

#else

#define OPENSSL_DEPRECATED
#define OPENSSL_BEGIN_ALLOW_DEPRECATED
#define OPENSSL_END_ALLOW_DEPRECATED

#endif


#if defined(__GNUC__) || defined(__clang__)
// MinGW has two different printf implementations. Ensure the format macro
// matches the selected implementation. See
// https://sourceforge.net/p/mingw-w64/wiki2/gnu%20printf/.
#if defined(__MINGW_PRINTF_FORMAT)
#define OPENSSL_PRINTF_FORMAT_FUNC(string_index, first_to_check) \
  __attribute__(                                                 \
      (__format__(__MINGW_PRINTF_FORMAT, string_index, first_to_check)))
#else
#define OPENSSL_PRINTF_FORMAT_FUNC(string_index, first_to_check) \
  __attribute__((__format__(__printf__, string_index, first_to_check)))
#endif
#else
#define OPENSSL_PRINTF_FORMAT_FUNC(string_index, first_to_check)
#endif

// OPENSSL_CLANG_PRAGMA emits a pragma on clang and nothing on other compilers.
#if defined(__clang__)
#define OPENSSL_CLANG_PRAGMA(arg) _Pragma(arg)
#else
#define OPENSSL_CLANG_PRAGMA(arg)
#endif

// OPENSSL_MSVC_PRAGMA emits a pragma on MSVC and nothing on other compilers.
#if defined(_MSC_VER)
#define OPENSSL_MSVC_PRAGMA(arg) __pragma(arg)
#else
#define OPENSSL_MSVC_PRAGMA(arg)
#endif

#if defined(__GNUC__) || defined(__clang__)
#define OPENSSL_UNUSED __attribute__((unused))
#else
#define OPENSSL_UNUSED
#endif

// C99-compatible static assertion using bit-field width trick.
// A negative bit-field width causes a compile-time error.
//
// Previously we defined |OPENSSL_STATIC_ASSERT| to use one of two keywords:
// |Static_assert| or |static_assert|. The latter was used if we were compiling
// a C++ translation unit or on Windows (excluding when using a Clang compiler).
// The former was used in other cases. However, these two keywords are not
// defined before C11. So, we can't rely on these when we want to be C99
// compliant. If we at some point decide that we want to only be compliant with
// C11 (and up), we can reintroduce these keywords. Instead, use a method that
// is guaranteed to be C99 compliant and still give us an equivalent static
// assert mechanism.
//
// The solution below defines a struct type containing a bit field.
// The name of that type is |static_assertion_msg|. |msg| is a concatenation of
// a user-chosen error (which should be chosen with respect to actual assertion)
// and the line the assertion is defined. This should ensure name uniqueness.
// The width of the bit field is set to 1 or -1, depending on the evaluation of
// the boolean expression |cond|. If the condition is false, the width requested
// is -1, which is illegal and would cause the compiler to throw an error.
//
// An example of an error thrown during compilation:
// ```
// error: negative width in bit-field
//      'static_assertion_at_line_913_error_is_AEAD_state_is_too_small'
// ```
#define AWSLC_CONCAT(left, right) left##right
#define AWSLC_STATIC_ASSERT_DEFINE(cond, msg) typedef struct { \
        unsigned int AWSLC_CONCAT(static_assertion_, msg) : (cond) ? 1 : -1; \
    } AWSLC_CONCAT(static_assertion_, msg) OPENSSL_UNUSED;
#define AWSLC_STATIC_ASSERT_ADD_LINE0(cond, suffix) AWSLC_STATIC_ASSERT_DEFINE(cond, AWSLC_CONCAT(at_line_, suffix))
#define AWSLC_STATIC_ASSERT_ADD_LINE1(cond, line, suffix) AWSLC_STATIC_ASSERT_ADD_LINE0(cond, AWSLC_CONCAT(line, suffix))
#define AWSLC_STATIC_ASSERT_ADD_LINE2(cond, suffix) AWSLC_STATIC_ASSERT_ADD_LINE1(cond, __LINE__, suffix)
#define AWSLC_STATIC_ASSERT_ADD_ERROR(cond, suffix) AWSLC_STATIC_ASSERT_ADD_LINE2(cond, AWSLC_CONCAT(_error_is_, suffix))
#define OPENSSL_STATIC_ASSERT(cond, error) AWSLC_STATIC_ASSERT_ADD_ERROR(cond, error)

// Sanity check of "target.h": OPENSSL_64_BIT/OPENSSL_32_BIT must match actual pointer size
#if defined(OPENSSL_64_BIT)
OPENSSL_STATIC_ASSERT(sizeof(void *) == 8, pointer_size_must_be_8_bytes_for_64_bit)
#elif defined(OPENSSL_32_BIT)
OPENSSL_STATIC_ASSERT(sizeof(void *) == 4, pointer_size_must_be_4_bytes_for_32_bit)
#endif

// Sanity checks of "target.h": OPENSSL_BIG_ENDIAN should be consistent with other endianness indicators.
// If architecture-specific big-endian macros are defined, OPENSSL_BIG_ENDIAN should be too.
#if (defined(__ARMEB__) || defined(__AARCH64EB__) || defined(__MIPSEB__) || \
     defined(__BIG_ENDIAN__) || (defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && \
     __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)) && !defined(OPENSSL_BIG_ENDIAN)
#error "Big-endian architecture detected but OPENSSL_BIG_ENDIAN is not defined"
#endif
// If architecture-specific little-endian macros are defined, OPENSSL_BIG_ENDIAN should not be.
#if (defined(__ARMEL__) || defined(__AARCH64EL__) || defined(__MIPSEL__) || \
     defined(__LITTLE_ENDIAN__) || (defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && \
     __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)) && defined(OPENSSL_BIG_ENDIAN)
#error "Little-endian architecture detected but OPENSSL_BIG_ENDIAN is defined"
#endif

// C and C++ handle inline functions differently. In C++, an inline function is
// defined in just the header file, potentially emitted in multiple compilation
// units (in cases the compiler did not inline), but each copy must be identical
// to satsify ODR. In C, a non-static inline must be manually emitted in exactly
// one compilation unit with a separate extern inline declaration.
//
// In both languages, exported inline functions referencing file-local symbols
// are problematic. C forbids this altogether (though GCC and Clang seem not to
// enforce it). It works in C++, but ODR requires the definitions be identical,
// including all names in the definitions resolving to the "same entity". In
// practice, this is unlikely to be a problem, but an inline function that
// returns a pointer to a file-local symbol
// could compile oddly.
//
// Historically, we used static inline in headers. However, to satisfy ODR, use
// plain inline in C++, to allow inline consumer functions to call our header
// functions. Plain inline would also work better with C99 inline, but that is
// not used much in practice, extern inline is tedious, and there are conflicts
// with the old gnu89 model:
// https://stackoverflow.com/questions/216510/extern-inline
#if defined(__cplusplus)
#define OPENSSL_INLINE inline
#else
// Add OPENSSL_UNUSED so that, should an inline function be emitted via macro
// (e.g. a |STACK_OF(T)| implementation) in a source file without tripping
// clang's -Wunused-function.
#define OPENSSL_INLINE static inline OPENSSL_UNUSED
#endif

#if defined(OPENSSL_WINDOWS)
#define OPENSSL_NOINLINE __declspec(noinline)
#else
#define OPENSSL_NOINLINE __attribute__((noinline))
#endif

// Import native AWS-LC type definitions. The canonical types use the AWSLC_
// prefix and are defined in awslc/types.h. The typedef aliases below provide
// OpenSSL-compatible type names.
//
// Note: We use typedef aliases (not #define macros) because the STACK_OF()
// and DEFINE_STACK_OF() macros use token-pasting (##), which prevents macro
// expansion. Typedef aliases are C-level constructs that don't interfere
// with the preprocessor, ensuring consistent type names in all contexts.
#include <awslc/types.h>

// OpenSSL compatibility type aliases — primitive types
typedef AWSLC_ossl_ssize_t ossl_ssize_t;
typedef AWSLC_CBS_ASN1_TAG CBS_ASN1_TAG;
typedef AWSLC_CRYPTO_THREADID CRYPTO_THREADID;
typedef AWSLC_ASN1_BOOLEAN ASN1_BOOLEAN;
typedef AWSLC_OPENSSL_BLOCK OPENSSL_BLOCK;

// OpenSSL compatibility type aliases — ASN.1 types
typedef AWSLC_ASN1_NULL ASN1_NULL;
typedef AWSLC_ASN1_ITEM ASN1_ITEM;
typedef AWSLC_ASN1_OBJECT ASN1_OBJECT;
typedef AWSLC_ASN1_PCTX ASN1_PCTX;
typedef AWSLC_ASN1_BIT_STRING ASN1_BIT_STRING;
typedef AWSLC_ASN1_BMPSTRING ASN1_BMPSTRING;
typedef AWSLC_ASN1_ENUMERATED ASN1_ENUMERATED;
typedef AWSLC_ASN1_GENERALIZEDTIME ASN1_GENERALIZEDTIME;
typedef AWSLC_ASN1_GENERALSTRING ASN1_GENERALSTRING;
typedef AWSLC_ASN1_IA5STRING ASN1_IA5STRING;
typedef AWSLC_ASN1_INTEGER ASN1_INTEGER;
typedef AWSLC_ASN1_OCTET_STRING ASN1_OCTET_STRING;
typedef AWSLC_ASN1_PRINTABLESTRING ASN1_PRINTABLESTRING;
typedef AWSLC_ASN1_STRING ASN1_STRING;
typedef AWSLC_ASN1_T61STRING ASN1_T61STRING;
typedef AWSLC_ASN1_TIME ASN1_TIME;
typedef AWSLC_ASN1_UNIVERSALSTRING ASN1_UNIVERSALSTRING;
typedef AWSLC_ASN1_UTCTIME ASN1_UTCTIME;
typedef AWSLC_ASN1_UTF8STRING ASN1_UTF8STRING;
typedef AWSLC_ASN1_VISIBLESTRING ASN1_VISIBLESTRING;
typedef AWSLC_ASN1_TYPE ASN1_TYPE;

// OpenSSL compatibility type aliases — X.509 types
typedef AWSLC_AUTHORITY_KEYID AUTHORITY_KEYID;
typedef AWSLC_BASIC_CONSTRAINTS BASIC_CONSTRAINTS;
typedef AWSLC_DIST_POINT DIST_POINT;
typedef AWSLC_GENERAL_NAME GENERAL_NAME;
typedef AWSLC_ISSUING_DIST_POINT ISSUING_DIST_POINT;
typedef AWSLC_NAME_CONSTRAINTS NAME_CONSTRAINTS;
typedef AWSLC_NETSCAPE_SPKAC NETSCAPE_SPKAC;
typedef AWSLC_NETSCAPE_SPKI NETSCAPE_SPKI;
typedef AWSLC_X509_VERIFY_PARAM X509_VERIFY_PARAM;
typedef AWSLC_X509_ALGOR X509_ALGOR;
typedef AWSLC_X509_CRL X509_CRL;
typedef AWSLC_X509_EXTENSION X509_EXTENSION;
typedef AWSLC_X509_INFO X509_INFO;
typedef AWSLC_X509_NAME_ENTRY X509_NAME_ENTRY;
typedef AWSLC_X509_NAME X509_NAME;
typedef AWSLC_X509_PUBKEY X509_PUBKEY;
typedef AWSLC_X509_REQ X509_REQ;
typedef AWSLC_X509_SIG_INFO X509_SIG_INFO;
typedef AWSLC_X509_SIG X509_SIG;
typedef AWSLC_X509V3_CTX X509V3_CTX;
typedef AWSLC_X509V3_EXT_METHOD X509V3_EXT_METHOD;
typedef AWSLC_X509_ATTRIBUTE X509_ATTRIBUTE;
typedef AWSLC_X509_LOOKUP X509_LOOKUP;
typedef AWSLC_X509_LOOKUP_METHOD X509_LOOKUP_METHOD;
typedef AWSLC_X509_OBJECT X509_OBJECT;
typedef AWSLC_X509_REVOKED X509_REVOKED;
typedef AWSLC_X509 X509;
typedef AWSLC_X509_STORE_CTX X509_STORE_CTX;
typedef AWSLC_X509_STORE X509_STORE;
typedef AWSLC_X509_TRUST X509_TRUST;
typedef AWSLC_X509_PKEY X509_PKEY;

// OpenSSL compatibility type aliases — BigNum types
typedef AWSLC_BN_CTX BN_CTX;
typedef AWSLC_BIGNUM BIGNUM;
typedef AWSLC_BN_GENCB BN_GENCB;
typedef AWSLC_BN_MONT_CTX BN_MONT_CTX;

// OpenSSL compatibility type aliases — BIO types
typedef AWSLC_BIO_METHOD BIO_METHOD;
typedef AWSLC_BIO BIO;

// OpenSSL compatibility type aliases — Buffer types
typedef AWSLC_BUF_MEM BUF_MEM;
typedef AWSLC_CBB CBB;
typedef AWSLC_CBS CBS;

// OpenSSL compatibility type aliases — Symmetric cipher types
typedef AWSLC_CAST_KEY CAST_KEY;
typedef AWSLC_RC4_KEY RC4_KEY;

// OpenSSL compatibility type aliases — Hash / digest types
typedef AWSLC_BLAKE2B_CTX BLAKE2B_CTX;
typedef AWSLC_RIPEMD160_CTX RIPEMD160_CTX;
typedef AWSLC_SHA256_CTX SHA256_CTX;
typedef AWSLC_SHA512_CTX SHA512_CTX;
typedef AWSLC_SHA_CTX SHA_CTX;
typedef AWSLC_MD4_CTX MD4_CTX;
typedef AWSLC_MD5_CTX MD5_CTX;

// OpenSSL compatibility type aliases — CMAC / HMAC types
typedef AWSLC_CMAC_CTX CMAC_CTX;
typedef AWSLC_HMAC_CTX HMAC_CTX;

// OpenSSL compatibility type aliases — Configuration types
typedef AWSLC_CONF CONF;
typedef AWSLC_CONF_VALUE CONF_VALUE;

// OpenSSL compatibility type aliases — CRYPTO buffer types
typedef AWSLC_CRYPTO_BUFFER_POOL CRYPTO_BUFFER_POOL;
typedef AWSLC_CRYPTO_BUFFER CRYPTO_BUFFER;

// OpenSSL compatibility type aliases — DRBG types
typedef AWSLC_CTR_DRBG_STATE CTR_DRBG_STATE;

// OpenSSL compatibility type aliases — DH / DSA types
typedef AWSLC_DH DH;
typedef AWSLC_DSA DSA;
typedef AWSLC_DSA_SIG DSA_SIG;

// OpenSSL compatibility type aliases — EC types
typedef AWSLC_EC_GROUP EC_GROUP;
typedef AWSLC_EC_KEY EC_KEY;
typedef AWSLC_EC_POINT EC_POINT;
typedef AWSLC_EC_KEY_METHOD EC_KEY_METHOD;
typedef AWSLC_ECDSA_SIG ECDSA_SIG;

// OpenSSL compatibility type aliases — Engine type
typedef AWSLC_ENGINE ENGINE;

// OpenSSL compatibility type aliases — EVP digest types
typedef AWSLC_EVP_MD_CTX EVP_MD_CTX;
typedef AWSLC_EVP_MD EVP_MD;

// OpenSSL compatibility type aliases — EVP AEAD types
typedef AWSLC_EVP_AEAD EVP_AEAD;
typedef AWSLC_EVP_AEAD_CTX EVP_AEAD_CTX;

// OpenSSL compatibility type aliases — EVP cipher types
typedef AWSLC_EVP_CIPHER_CTX EVP_CIPHER_CTX;
typedef AWSLC_EVP_CIPHER EVP_CIPHER;

// OpenSSL compatibility type aliases — EVP encode types
typedef AWSLC_EVP_ENCODE_CTX EVP_ENCODE_CTX;

// OpenSSL compatibility type aliases — EVP HPKE types
typedef AWSLC_EVP_HPKE_AEAD EVP_HPKE_AEAD;
typedef AWSLC_EVP_HPKE_CTX EVP_HPKE_CTX;
typedef AWSLC_EVP_HPKE_KDF EVP_HPKE_KDF;
typedef AWSLC_EVP_HPKE_KEM EVP_HPKE_KEM;
typedef AWSLC_EVP_HPKE_KEY EVP_HPKE_KEY;

// OpenSSL compatibility type aliases — EVP KEM types
typedef AWSLC_EVP_KEM EVP_KEM;
typedef AWSLC_KEM_KEY KEM_KEY;

// OpenSSL compatibility type aliases — EVP PKEY types
typedef AWSLC_EVP_PKEY_CTX EVP_PKEY_CTX;
typedef AWSLC_EVP_PKEY_ASN1_METHOD EVP_PKEY_ASN1_METHOD;
typedef AWSLC_EVP_PKEY EVP_PKEY;
typedef AWSLC_EVP_PKEY_CTX_SIGNATURE_CONTEXT_PARAMS EVP_PKEY_CTX_SIGNATURE_CONTEXT_PARAMS;

// OpenSSL compatibility type aliases — Post-quantum types
typedef AWSLC_PQDSA_KEY PQDSA_KEY;

// OpenSSL compatibility type aliases — OCSP types
typedef AWSLC_OCSP_REQ_CTX OCSP_REQ_CTX;

// OpenSSL compatibility type aliases — OpenSSL init settings
typedef AWSLC_OPENSSL_INIT_SETTINGS OPENSSL_INIT_SETTINGS;

// OpenSSL compatibility type aliases — PKCS7 types
typedef AWSLC_PKCS7_DIGEST PKCS7_DIGEST;
typedef AWSLC_PKCS7_ENC_CONTENT PKCS7_ENC_CONTENT;
typedef AWSLC_PKCS7_ENCRYPT PKCS7_ENCRYPT;
typedef AWSLC_PKCS7_ENVELOPE PKCS7_ENVELOPE;
typedef AWSLC_PKCS7_ISSUER_AND_SERIAL PKCS7_ISSUER_AND_SERIAL;
typedef AWSLC_PKCS7_RECIP_INFO PKCS7_RECIP_INFO;
typedef AWSLC_PKCS7_SIGN_ENVELOPE PKCS7_SIGN_ENVELOPE;
typedef AWSLC_PKCS7_SIGNED PKCS7_SIGNED;
typedef AWSLC_PKCS7_SIGNER_INFO PKCS7_SIGNER_INFO;
typedef AWSLC_PKCS7 PKCS7;

// OpenSSL compatibility type aliases — PKCS8 / PKCS12 types
typedef AWSLC_PKCS12 PKCS12;
typedef AWSLC_PKCS8_PRIV_KEY_INFO PKCS8_PRIV_KEY_INFO;

// OpenSSL compatibility type aliases — RAND types
typedef AWSLC_RAND_METHOD RAND_METHOD;

// OpenSSL compatibility type aliases — RSA types
typedef AWSLC_RSA_METHOD RSA_METHOD;
typedef AWSLC_RSASSA_PSS_PARAMS RSASSA_PSS_PARAMS;
typedef AWSLC_RSA_PSS_PARAMS RSA_PSS_PARAMS;
typedef AWSLC_RSA RSA;

// OpenSSL compatibility type aliases — SPAKE2 types
typedef AWSLC_SPAKE2_CTX SPAKE2_CTX;

// OpenSSL compatibility type aliases — SRTP types
typedef AWSLC_SRTP_PROTECTION_PROFILE SRTP_PROTECTION_PROFILE;

// OpenSSL compatibility type aliases — SSL types
typedef AWSLC_SSL_CIPHER SSL_CIPHER;
typedef AWSLC_SSL_CTX SSL_CTX;
typedef AWSLC_SSL_CLIENT_HELLO SSL_CLIENT_HELLO;
typedef AWSLC_SSL_ECH_KEYS SSL_ECH_KEYS;
typedef AWSLC_SSL_METHOD SSL_METHOD;
typedef AWSLC_SSL_PRIVATE_KEY_METHOD SSL_PRIVATE_KEY_METHOD;
typedef AWSLC_SSL_QUIC_METHOD SSL_QUIC_METHOD;
typedef AWSLC_SSL_SESSION SSL_SESSION;
typedef AWSLC_SSL SSL;
typedef AWSLC_SSL_TICKET_AEAD_METHOD SSL_TICKET_AEAD_METHOD;

// OpenSSL compatibility type aliases — Error types
typedef AWSLC_ERR_FNS ERR_FNS;

// OpenSSL compatibility type aliases — Trust Token types
typedef AWSLC_TRUST_TOKEN TRUST_TOKEN;
typedef AWSLC_TRUST_TOKEN_CLIENT TRUST_TOKEN_CLIENT;
typedef AWSLC_TRUST_TOKEN_ISSUER TRUST_TOKEN_ISSUER;
typedef AWSLC_TRUST_TOKEN_METHOD TRUST_TOKEN_METHOD;

// BSSL_CHECK aborts if |condition| is not true.
#define BSSL_CHECK(condition) \
  do {                        \
    if (!(condition)) {       \
      abort();                \
    }                         \
  } while (0);

#if defined(__cplusplus)
}  // extern C
#elif !defined(BORINGSSL_NO_CXX)
#define BORINGSSL_NO_CXX
#endif

#if defined(BORINGSSL_PREFIX)
#define BSSL_NAMESPACE_BEGIN \
  namespace bssl {           \
  inline namespace BORINGSSL_PREFIX {
#define BSSL_NAMESPACE_END \
  }                        \
  }
#else
#define BSSL_NAMESPACE_BEGIN namespace bssl {
#define BSSL_NAMESPACE_END }
#endif

// MSVC doesn't set __cplusplus to 201103 to indicate C++11 support (see
// https://connect.microsoft.com/VisualStudio/feedback/details/763051/a-value-of-predefined-macro-cplusplus-is-still-199711l)
// so MSVC is just assumed to support C++11.
#if !defined(BORINGSSL_NO_CXX) && __cplusplus < 201103L && !defined(_MSC_VER)
#define BORINGSSL_NO_CXX
#endif

#if !defined(BORINGSSL_NO_CXX)

extern "C++" {

#include <memory>

// STLPort, used by some Android consumers, not have std::unique_ptr.
#if defined(_STLPORT_VERSION)
#define BORINGSSL_NO_CXX
#endif

}  // extern C++
#endif  // !BORINGSSL_NO_CXX

#if defined(BORINGSSL_NO_CXX)

#define BORINGSSL_MAKE_DELETER(type, deleter)
#define BORINGSSL_MAKE_UP_REF(type, up_ref_func)

#else

extern "C++" {

BSSL_NAMESPACE_BEGIN

namespace internal {

// The Enable parameter is ignored and only exists so specializations can use
// SFINAE.
template <typename T, typename Enable = void>
struct DeleterImpl {};

struct Deleter {
  template <typename T>
  void operator()(T *ptr) {
    // Rather than specialize Deleter for each type, we specialize
    // DeleterImpl. This allows bssl::UniquePtr<T> to be used while only
    // including base.h as long as the destructor is not emitted. This matches
    // std::unique_ptr's behavior on forward-declared types.
    //
    // DeleterImpl itself is specialized in the corresponding module's header
    // and must be included to release an object. If not included, the compiler
    // will error that DeleterImpl<T> does not have a method Free.
    DeleterImpl<T>::Free(ptr);
  }
};

template <typename T, typename CleanupRet, void (*init)(T *),
          CleanupRet (*cleanup)(T *)>
class StackAllocated {
 public:
  StackAllocated() { init(&ctx_); }
  ~StackAllocated() { cleanup(&ctx_); }

  StackAllocated(const StackAllocated &) = delete;
  StackAllocated& operator=(const StackAllocated &) = delete;

  T *get() { return &ctx_; }
  const T *get() const { return &ctx_; }

  T *operator->() { return &ctx_; }
  const T *operator->() const { return &ctx_; }

  void Reset() {
    cleanup(&ctx_);
    init(&ctx_);
  }

 private:
  T ctx_;
};

template <typename T, typename CleanupRet, void (*init)(T *),
          CleanupRet (*cleanup)(T *), void (*move)(T *, T *)>
class StackAllocatedMovable {
 public:
  StackAllocatedMovable() { init(&ctx_); }
  ~StackAllocatedMovable() { cleanup(&ctx_); }

  StackAllocatedMovable(StackAllocatedMovable &&other) {
    init(&ctx_);
    move(&ctx_, &other.ctx_);
  }
  StackAllocatedMovable &operator=(StackAllocatedMovable &&other) {
    move(&ctx_, &other.ctx_);
    return *this;
  }

  T *get() { return &ctx_; }
  const T *get() const { return &ctx_; }

  T *operator->() { return &ctx_; }
  const T *operator->() const { return &ctx_; }

  void Reset() {
    cleanup(&ctx_);
    init(&ctx_);
  }

 private:
  T ctx_;
};

}  // namespace internal

#define BORINGSSL_MAKE_DELETER(type, deleter)     \
  namespace internal {                            \
  template <>                                     \
  struct DeleterImpl<type> {                      \
    static void Free(type *ptr) { deleter(ptr); } \
  };                                              \
  }

// Holds ownership of heap-allocated BoringSSL structures. Sample usage:
//   bssl::UniquePtr<RSA> rsa(RSA_new());
//   bssl::UniquePtr<BIO> bio(BIO_new(BIO_s_mem()));
template <typename T>
using UniquePtr = std::unique_ptr<T, internal::Deleter>;

#define BORINGSSL_MAKE_UP_REF(type, up_ref_func)             \
  inline UniquePtr<type> UpRef(type *v) {                    \
    if (v != nullptr) {                                      \
      up_ref_func(v);                                        \
    }                                                        \
    return UniquePtr<type>(v);                               \
  }                                                          \
                                                             \
  inline UniquePtr<type> UpRef(const UniquePtr<type> &ptr) { \
    return UpRef(ptr.get());                                 \
  }

BSSL_NAMESPACE_END

}  // extern C++

#endif  // !BORINGSSL_NO_CXX

#endif  // OPENSSL_HEADER_BASE_H

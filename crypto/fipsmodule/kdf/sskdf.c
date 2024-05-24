// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0 OR ISC

#include <assert.h>
#include <openssl/base.h>
#include <openssl/digest.h>
#include <openssl/hmac.h>
#include <openssl/kdf.h>
#include "../delocate.h"
#include "internal.h"

static int SSKDF_VARIANT_DIGEST_CTX_init(SSKDF_VARIANT_CTX *ctx,
                                         const EVP_MD *digest) {
  SSKDF_VARIANT_DIGEST_CTX *variant_ctx = NULL;
  EVP_MD_CTX *md_ctx = NULL;

  int ret = 0;

  if (!ctx || !digest) {
    goto err;
  }

  variant_ctx = OPENSSL_malloc(sizeof(SSKDF_VARIANT_DIGEST_CTX));
  if (!variant_ctx) {
    goto err;
  }

  md_ctx = EVP_MD_CTX_new();
  if (!md_ctx) {
    goto err;
  }

  ret = 1;
  variant_ctx->digest = digest;
  variant_ctx->ctx = md_ctx;
  ctx->data = variant_ctx;

  return ret;

err:
  EVP_MD_CTX_free(md_ctx);
  OPENSSL_free(variant_ctx);

  return ret;
}



static void SSKDF_VARIANT_DIGEST_CTX_cleanup(SSKDF_VARIANT_CTX *ctx) {
  if (!ctx) {
    return;
  }
  SSKDF_VARIANT_DIGEST_CTX *variant_ctx = (SSKDF_VARIANT_DIGEST_CTX *)ctx->data;
  if (!ctx->data) {
    return;
  }
  EVP_MD_CTX_free(variant_ctx->ctx);
  OPENSSL_free(variant_ctx);
  ctx->data = NULL;
}

static int SSKDF_VARIANT_HMAC_CTX_init(SSKDF_VARIANT_CTX *ctx,
                                       const EVP_MD *digest,
                                       const uint8_t *salt, size_t salt_len) {
  SSKDF_VARIANT_HMAC_CTX *variant_ctx = NULL;
  HMAC_CTX *hmac_ctx = NULL;

  int ret = 0;

  if (!ctx || !digest) {
    goto err;
  }

  variant_ctx = OPENSSL_malloc(sizeof(SSKDF_VARIANT_HMAC_CTX));
  if (!variant_ctx) {
    goto err;
  }

  hmac_ctx = HMAC_CTX_new();
  if (!hmac_ctx) {
    goto err;
  }

  if (!HMAC_Init_ex(hmac_ctx, salt, salt_len, digest, NULL)) {
    goto err;
  }

  ret = 1;
  variant_ctx->ctx = hmac_ctx;
  ctx->data = variant_ctx;

  return ret;

err:
  HMAC_CTX_free(hmac_ctx);
  OPENSSL_free(variant_ctx);

  return ret;
}



static void SSKDF_VARIANT_HMAC_CTX_cleanup(SSKDF_VARIANT_CTX *ctx) {
  if (!ctx) {
    return;
  }
  SSKDF_VARIANT_HMAC_CTX *hmac_ctx = (SSKDF_VARIANT_HMAC_CTX *)ctx->data;
  if (!hmac_ctx) {
    return;
  }
  HMAC_CTX_free(hmac_ctx->ctx);
  OPENSSL_free(hmac_ctx);
  ctx->data = NULL;
}

static size_t sskdf_variant_digest_size(SSKDF_VARIANT_CTX *ctx) {
  if (!ctx || !ctx->data) {
    return 0;
  }
  SSKDF_VARIANT_DIGEST_CTX *variant_ctx = (SSKDF_VARIANT_DIGEST_CTX *)ctx->data;
  return EVP_MD_size(variant_ctx->digest);
}

static int sskdf_variant_digest_compute(SSKDF_VARIANT_CTX *ctx, uint8_t *out,
                                        size_t out_len,
                                        const uint8_t counter[4],
                                        const uint8_t *secret,
                                        size_t secret_len, const uint8_t *info,
                                        size_t info_len) {
  if (!ctx || !ctx->data || !out || !counter || !secret) {
    return 0;
  }

  SSKDF_VARIANT_DIGEST_CTX *variant_ctx = (SSKDF_VARIANT_DIGEST_CTX *)ctx->data;

  if (!variant_ctx->ctx || !variant_ctx->digest) {
    return 0;
  }

  if (!EVP_MD_CTX_reset(variant_ctx->ctx) ||
      !EVP_DigestInit_ex(variant_ctx->ctx, variant_ctx->digest, NULL) ||
      !EVP_DigestUpdate(variant_ctx->ctx, &counter[0], 4) ||
      !EVP_DigestUpdate(variant_ctx->ctx, secret, secret_len) ||
      !EVP_DigestUpdate(variant_ctx->ctx, info, info_len) ||
      !EVP_DigestFinal(variant_ctx->ctx, out, NULL)) {
    return 0;
  }

  return 1;
}

static size_t sskdf_variant_hmac_size(SSKDF_VARIANT_CTX *ctx) {
  if (!ctx || !ctx->data) {
    return 0;
  }
  SSKDF_VARIANT_HMAC_CTX *variant_ctx = (SSKDF_VARIANT_HMAC_CTX *)ctx->data;
  if (!variant_ctx) {
    return 0;
  }
  return HMAC_size(variant_ctx->ctx);
}

static int sskdf_variant_hmac_compute(SSKDF_VARIANT_CTX *ctx, uint8_t *out,
                                      size_t out_len, const uint8_t counter[4],
                                      const uint8_t *secret, size_t secret_len,
                                      const uint8_t *info, size_t info_len) {
  if (!ctx || !ctx->data || !out || !counter || !secret) {
    return 0;
  }

  SSKDF_VARIANT_HMAC_CTX *variant_ctx = (SSKDF_VARIANT_HMAC_CTX *)ctx->data;

  if (!variant_ctx->ctx) {
    return 0;
  }

  if (!HMAC_Init_ex(variant_ctx->ctx, NULL, 0, NULL, NULL) ||
      !HMAC_Update(variant_ctx->ctx, &counter[0], 4) ||
      !HMAC_Update(variant_ctx->ctx, secret, secret_len) ||
      !HMAC_Update(variant_ctx->ctx, info, info_len) ||
      !HMAC_Final(variant_ctx->ctx, out, NULL)) {
    return 0;
  }

  return 1;
}


DEFINE_METHOD_FUNCTION(SSKDF_VARIANT, SSKDF_VARIANT_DIGEST) {
  memset(out, 0, sizeof(SSKDF_VARIANT));
  out->size = sskdf_variant_digest_size;
  out->compute = sskdf_variant_digest_compute;
}


DEFINE_METHOD_FUNCTION(SSKDF_VARIANT, SSKDF_VARIANT_HMAC) {
  memset(out, 0, sizeof(SSKDF_VARIANT));
  out->size = sskdf_variant_hmac_size;
  out->compute = sskdf_variant_hmac_compute;
};

static int SSKDF(const SSKDF_VARIANT *variant, SSKDF_VARIANT_CTX *ctx,
                 uint8_t *out_key, size_t out_len, const uint8_t *secret,
                 size_t secret_len, const uint8_t *info, size_t info_len) {
  int ret = 0;

  if (!ctx) {
    abort();
  }

  if (!out_key || out_len == 0 || !secret || secret_len == 0) {
    goto err;
  }

  size_t h_len = variant->size(ctx);
  if (!h_len) {
    goto err;
  }
  assert(h_len <= EVP_MAX_MD_SIZE);

  uint64_t n = (out_len + h_len - 1) / h_len;

  if (out_len + h_len < out_len || n > UINT32_MAX) {
    goto err;
  }

  // Validate that the sum of the fixed info lengths won't overflow a size_t
  if (4 > SIZE_MAX - secret_len || secret_len > SIZE_MAX - 4 ||
      info_len > SIZE_MAX - 4 - secret_len) {
    goto err;
  }

  // Now cap the approved algorithms to not exceed the max_H_inputBits for
  // hash(x) or HMAC-hash(salt, x). UINT32_MAX is a sufficient max value. See
  // Section 4.2 Table 1 and 2
  // https://nvlpubs.nist.gov/nistpubs/SpecialPublications/NIST.SP.800-56Cr2.pdf
  if (4 + secret_len + info_len > UINT32_MAX) {
    goto err;
  }

  // TODO: Abstract buffer size, if we ever need to support KMAC this could be
  // variable. Currently sufficient for HMAC and digest variants
  uint8_t previous[EVP_MAX_MD_SIZE];

  size_t done = 0;

  for (uint32_t i = 0; i < n; i++) {
    uint8_t counter[4];  // counter == 4 bytes
    size_t todo;

    CRYPTO_store_u32_be(&counter[0], i + 1);

    if (!variant->compute(ctx, &previous[0], h_len, counter, secret, secret_len,
                          info, info_len)) {
      goto err;
    }

    todo = h_len;
    if (todo > out_len - done) {
      todo = out_len - done;
    }
    OPENSSL_memcpy(out_key + done, previous, todo);
    done += todo;
  }

  ret = 1;

err:
  return ret;
}

int SSKDF_DIGEST(uint8_t *out_key, size_t out_len, const EVP_MD *digest,
                 const uint8_t *secret, size_t secret_len, const uint8_t *info,
                 size_t info_len) {
  SSKDF_VARIANT_CTX ctx;
  int ret = 0;

  if (!SSKDF_VARIANT_DIGEST_CTX_init(&ctx, digest)) {
    return 0;
  }

  if (!SSKDF(SSKDF_VARIANT_DIGEST(), &ctx, out_key, out_len, secret, secret_len,
             info, info_len)) {
    goto end;
  }

  ret = 1;

end:
  SSKDF_VARIANT_DIGEST_CTX_cleanup(&ctx);
  return ret;
}

int SSKDF_HMAC(uint8_t *out_key, size_t out_len, const EVP_MD *digest,
               const uint8_t *secret, size_t secret_len, const uint8_t *info,
               size_t info_len, const uint8_t *salt, size_t salt_len) {
  SSKDF_VARIANT_CTX ctx;
  int ret = 0;

  if (!SSKDF_VARIANT_HMAC_CTX_init(&ctx, digest, salt, salt_len)) {
    return 0;
  }

  if (!SSKDF(SSKDF_VARIANT_HMAC(), &ctx, out_key, out_len, secret, secret_len,
             info, info_len)) {
    goto end;
  }

  ret = 1;

end:
  SSKDF_VARIANT_HMAC_CTX_cleanup(&ctx);
  return ret;
}

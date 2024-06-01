// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0 OR ISC

#ifndef OPENSSL_HEADER_KDF_INTERNAL_H
#define OPENSSL_HEADER_KDF_INTERNAL_H

#include <openssl/digest.h>
#include <openssl/hmac.h>

typedef struct {
  void *data;
} SSKDF_VARIANT_CTX;

typedef struct {
  const EVP_MD *digest;
  EVP_MD_CTX *ctx;
} SSKDF_VARIANT_DIGEST_CTX;

typedef struct {
  HMAC_CTX *ctx;
} SSKDF_VARIANT_HMAC_CTX;

typedef struct {
  size_t (*size)(SSKDF_VARIANT_CTX *ctx);
  int (*compute)(SSKDF_VARIANT_CTX *ctx, uint8_t *out, size_t out_len,
                 const uint8_t counter[4], const uint8_t *secret,
                 size_t secret_len, const uint8_t *info, size_t info_len);
} SSKDF_VARIANT;

const SSKDF_VARIANT *SSKDF_VARIANT_DIGEST(void);

const SSKDF_VARIANT *SSKDF_VARIANT_HMAC(void);

#endif // OPENSSL_HEADER_KDF_INTERNAL_H

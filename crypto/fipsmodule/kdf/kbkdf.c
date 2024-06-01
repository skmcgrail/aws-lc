// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0 OR ISC

#include <openssl/kdf.h>
#include "internal.h"

int KBKDF_CTR(uint8_t *out_key, size_t out_len, const EVP_MD *digest,
              const uint8_t *secret, size_t secret_len, const uint8_t *info,
              size_t info_len) {
  int ret = 0;

  HMAC_CTX *hmac_ctx = NULL;

  if (!out_key || out_len == 0 || !secret || secret_len == 0) {
    goto err;
  }

  hmac_ctx = HMAC_CTX_new();
  if (!hmac_ctx) {
    goto err;
  }

  if (!HMAC_Init_ex(hmac_ctx, secret, secret_len, digest, NULL)) {
    goto err;
  }

  size_t h_len = HMAC_size(hmac_ctx);
  if (!h_len) {
    goto err;
  }
  assert(h_len > 0);

  uint64_t n = (out_len + h_len - 1) / h_len;

  if (out_len + h_len < out_len || n > UINT32_MAX) {
    goto err;
  }

  uint8_t previous[EVP_MAX_MD_SIZE];

  size_t done = 0;

  for (uint32_t i = 0; i < n; i++) {
    uint8_t counter[4];  // counter == 4 bytes
    size_t todo;

    CRYPTO_store_u32_be(&counter[0], i + 1);

    if (!HMAC_Init_ex(hmac_ctx, NULL, 0, NULL, NULL) ||
        !HMAC_Update(hmac_ctx, &counter[0], 4) ||
        !HMAC_Update(hmac_ctx, info, info_len) ||
        !HMAC_Final(hmac_ctx, previous, NULL)) {
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
  HMAC_CTX_free(hmac_ctx);
  return ret;
}

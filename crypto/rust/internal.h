// Copyright Amazon.com Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0 OR ISC

#ifndef OPENSSL_HEADER_CRYPTO_RUST_INTERNAL_H
#define OPENSSL_HEADER_CRYPTO_RUST_INTERNAL_H

#include <openssl/base.h>

#if defined(__cplusplus)
extern "C" {
#endif

// AWSLC_RUST is defined when AWS-LC is configured with -DENABLE_RUST=ON, which
// links the Rust code under crypto/rust into libcrypto. The Rust symbols
// themselves are not exported from libcrypto; C code wraps them.
//
// The declarations below are guarded so that the symbol registry tooling,
// which scans headers without AWSLC_RUST, does not pick them up.
#if defined(AWSLC_RUST)

// CRYPTO_rust_byte_sum writes the sum of the |in_len| bytes at |in| to |*out|
// using the Rust implementation. It returns one on success and zero on error.
// It is a placeholder that exists to test the C-to-Rust call path.
OPENSSL_EXPORT int CRYPTO_rust_byte_sum(const uint8_t *in, size_t in_len,
                                        uint64_t *out);

#endif  // AWSLC_RUST

#if defined(__cplusplus)
}  // extern C
#endif

#endif  // OPENSSL_HEADER_CRYPTO_RUST_INTERNAL_H

/* Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0 OR ISC */

#ifndef AWSLC_HEADER_DIGEST_H
#define AWSLC_HEADER_DIGEST_H

/**
 * @file
 * @brief Native AWS-LC digest function declarations.
 *
 * This file declares the canonical AWS-LC digest functions with the AWSLC_
 * prefix. OpenSSL-compatible function names are provided as preprocessor
 * macros in openssl/digest.h for backward compatibility.
 *
 * Unlike type aliases (which must use typedef to avoid token-pasting issues
 * with STACK_OF), function aliases use #define macros safely because function
 * names are not used in ## token-pasting contexts.
 */

#include <stdbool.h>

#include <openssl/base.h>

#if defined(__cplusplus)
extern "C" {
#endif


// Hash algorithms.
//
// The following functions return |AWSLC_EVP_MD| objects that implement the
// named hash function.

OPENSSL_EXPORT const AWSLC_EVP_MD *AWSLC_EVP_md4(void);
OPENSSL_EXPORT const AWSLC_EVP_MD *AWSLC_EVP_md5(void);
OPENSSL_EXPORT const AWSLC_EVP_MD *AWSLC_EVP_ripemd160(void);
OPENSSL_EXPORT const AWSLC_EVP_MD *AWSLC_EVP_sha1(void);
OPENSSL_EXPORT const AWSLC_EVP_MD *AWSLC_EVP_sha224(void);
OPENSSL_EXPORT const AWSLC_EVP_MD *AWSLC_EVP_sha256(void);
OPENSSL_EXPORT const AWSLC_EVP_MD *AWSLC_EVP_sha384(void);
OPENSSL_EXPORT const AWSLC_EVP_MD *AWSLC_EVP_sha512(void);
OPENSSL_EXPORT const AWSLC_EVP_MD *AWSLC_EVP_sha512_224(void);
OPENSSL_EXPORT const AWSLC_EVP_MD *AWSLC_EVP_sha512_256(void);
OPENSSL_EXPORT const AWSLC_EVP_MD *AWSLC_EVP_sha3_224(void);
OPENSSL_EXPORT const AWSLC_EVP_MD *AWSLC_EVP_sha3_256(void);
OPENSSL_EXPORT const AWSLC_EVP_MD *AWSLC_EVP_sha3_384(void);
OPENSSL_EXPORT const AWSLC_EVP_MD *AWSLC_EVP_sha3_512(void);
OPENSSL_EXPORT const AWSLC_EVP_MD *AWSLC_EVP_shake128(void);
OPENSSL_EXPORT const AWSLC_EVP_MD *AWSLC_EVP_shake256(void);
OPENSSL_EXPORT const AWSLC_EVP_MD *AWSLC_EVP_blake2b256(void);

// AWSLC_EVP_md5_sha1 is a TLS-specific |AWSLC_EVP_MD| which computes the
// concatenation of MD5 and SHA-1, as used in TLS 1.1 and below.
OPENSSL_EXPORT const AWSLC_EVP_MD *AWSLC_EVP_md5_sha1(void);

// AWSLC_EVP_get_digestbynid returns an |AWSLC_EVP_MD| for the given NID, or
// NULL if no such digest is known.
OPENSSL_EXPORT const AWSLC_EVP_MD *AWSLC_EVP_get_digestbynid(int nid);

// AWSLC_EVP_get_digestbyobj returns an |AWSLC_EVP_MD| for the given
// |AWSLC_ASN1_OBJECT|, or NULL if no such digest is known.
OPENSSL_EXPORT const AWSLC_EVP_MD *AWSLC_EVP_get_digestbyobj(
    const AWSLC_ASN1_OBJECT *obj);


// Digest contexts.

// AWSLC_EVP_MD_CTX_init initialises an, already allocated,
// |AWSLC_EVP_MD_CTX|. This is the same as setting the structure to zero.
OPENSSL_EXPORT void AWSLC_EVP_MD_CTX_init(AWSLC_EVP_MD_CTX *ctx);

// AWSLC_EVP_MD_CTX_new allocates and initialises a fresh |AWSLC_EVP_MD_CTX|
// and returns it, or NULL on allocation failure.
OPENSSL_EXPORT AWSLC_EVP_MD_CTX *AWSLC_EVP_MD_CTX_new(void);

// AWSLC_EVP_MD_CTX_cleanup frees any resources owned by |ctx| and resets it to
// a freshly initialised state. It does not free |ctx| itself. It returns one.
OPENSSL_EXPORT int AWSLC_EVP_MD_CTX_cleanup(AWSLC_EVP_MD_CTX *ctx);

// AWSLC_EVP_MD_CTX_cleanse zeros the digest state in |ctx| and then performs
// the actions of |AWSLC_EVP_MD_CTX_cleanup|.
OPENSSL_EXPORT void AWSLC_EVP_MD_CTX_cleanse(AWSLC_EVP_MD_CTX *ctx);

// AWSLC_EVP_MD_CTX_free calls |AWSLC_EVP_MD_CTX_cleanup| and then frees |ctx|.
OPENSSL_EXPORT void AWSLC_EVP_MD_CTX_free(AWSLC_EVP_MD_CTX *ctx);

// AWSLC_EVP_MD_CTX_copy_ex sets |out|, which must already be initialised, to
// be a copy of |in|. It returns one on success and zero on allocation failure.
OPENSSL_EXPORT int AWSLC_EVP_MD_CTX_copy_ex(AWSLC_EVP_MD_CTX *out,
                                             const AWSLC_EVP_MD_CTX *in);

// AWSLC_EVP_MD_CTX_move sets |out|, which must already be initialised, to the
// hash state in |in|. |in| is mutated and left in an empty state.
OPENSSL_EXPORT void AWSLC_EVP_MD_CTX_move(AWSLC_EVP_MD_CTX *out,
                                           AWSLC_EVP_MD_CTX *in);

// AWSLC_EVP_MD_CTX_reset calls |AWSLC_EVP_MD_CTX_cleanup| followed by
// |AWSLC_EVP_MD_CTX_init|. It returns one.
OPENSSL_EXPORT int AWSLC_EVP_MD_CTX_reset(AWSLC_EVP_MD_CTX *ctx);


// Digest operations.

// AWSLC_EVP_DigestInit_ex configures |ctx| for a fresh hashing operation using
// |type|. It returns one on success and zero on allocation failure.
OPENSSL_EXPORT int AWSLC_EVP_DigestInit_ex(AWSLC_EVP_MD_CTX *ctx,
                                           const AWSLC_EVP_MD *type,
                                           AWSLC_ENGINE *engine);

// AWSLC_EVP_DigestInit acts like |AWSLC_EVP_DigestInit_ex| except that |ctx|
// is initialised before use.
OPENSSL_EXPORT int AWSLC_EVP_DigestInit(AWSLC_EVP_MD_CTX *ctx,
                                        const AWSLC_EVP_MD *type);

// AWSLC_EVP_DigestUpdate hashes |len| bytes from |data| into the hashing
// operation in |ctx|. It returns one.
OPENSSL_EXPORT int AWSLC_EVP_DigestUpdate(AWSLC_EVP_MD_CTX *ctx,
                                          const void *data, size_t len);

// AWSLC_EVP_DigestFinal_ex finishes the digest in |ctx| and writes the output
// to |md_out|. It returns one.
OPENSSL_EXPORT int AWSLC_EVP_DigestFinal_ex(AWSLC_EVP_MD_CTX *ctx,
                                            uint8_t *md_out,
                                            unsigned int *out_size);

// AWSLC_EVP_DigestFinal acts like |AWSLC_EVP_DigestFinal_ex| except that
// |AWSLC_EVP_MD_CTX_cleanup| is called on |ctx| before returning.
OPENSSL_EXPORT int AWSLC_EVP_DigestFinal(AWSLC_EVP_MD_CTX *ctx,
                                         uint8_t *md_out,
                                         unsigned int *out_size);

// AWSLC_EVP_Digest performs a complete hashing operation in one call.
OPENSSL_EXPORT int AWSLC_EVP_Digest(const void *data, size_t len,
                                    uint8_t *md_out, unsigned int *out_size,
                                    const AWSLC_EVP_MD *type,
                                    AWSLC_ENGINE *impl);


// Digest function accessors.

// AWSLC_EVP_MD_type returns a NID identifying |md|.
OPENSSL_EXPORT int AWSLC_EVP_MD_type(const AWSLC_EVP_MD *md);

// AWSLC_EVP_MD_flags returns the flags for |md|.
OPENSSL_EXPORT uint32_t AWSLC_EVP_MD_flags(const AWSLC_EVP_MD *md);

// AWSLC_EVP_MD_size returns the digest size of |md|, in bytes.
OPENSSL_EXPORT size_t AWSLC_EVP_MD_size(const AWSLC_EVP_MD *md);

// AWSLC_EVP_MD_block_size returns the native block-size of |md|, in bytes.
OPENSSL_EXPORT size_t AWSLC_EVP_MD_block_size(const AWSLC_EVP_MD *md);


// Digest operation accessors.

// AWSLC_EVP_MD_CTX_md returns the underlying digest function, or NULL if one
// has not been set.
OPENSSL_EXPORT const AWSLC_EVP_MD *AWSLC_EVP_MD_CTX_md(
    const AWSLC_EVP_MD_CTX *ctx);

// AWSLC_EVP_MD_CTX_size returns the digest size of |ctx|, in bytes.
OPENSSL_EXPORT size_t AWSLC_EVP_MD_CTX_size(const AWSLC_EVP_MD_CTX *ctx);

// AWSLC_EVP_MD_CTX_block_size returns the block size of the digest function
// used by |ctx|, in bytes.
OPENSSL_EXPORT size_t AWSLC_EVP_MD_CTX_block_size(
    const AWSLC_EVP_MD_CTX *ctx);

// AWSLC_EVP_MD_CTX_type returns a NID describing the digest function used by
// |ctx|.
OPENSSL_EXPORT int AWSLC_EVP_MD_CTX_type(const AWSLC_EVP_MD_CTX *ctx);


// ASN.1 functions.

// AWSLC_EVP_parse_digest_algorithm parses an AlgorithmIdentifier structure
// containing a hash function OID and advances |cbs|.
OPENSSL_EXPORT const AWSLC_EVP_MD *AWSLC_EVP_parse_digest_algorithm(
    AWSLC_CBS *cbs);

// AWSLC_EVP_marshal_digest_algorithm marshals |md| as an AlgorithmIdentifier
// structure and appends the result to |cbb|.
OPENSSL_EXPORT int AWSLC_EVP_marshal_digest_algorithm(
    AWSLC_CBB *cbb, const AWSLC_EVP_MD *md);


// Deprecated functions.

OPENSSL_EXPORT int AWSLC_EVP_MD_CTX_copy(AWSLC_EVP_MD_CTX *out,
                                          const AWSLC_EVP_MD_CTX *in);

OPENSSL_EXPORT const AWSLC_EVP_MD *AWSLC_EVP_get_digestbyname(const char *);

OPENSSL_EXPORT AWSLC_EVP_MD_CTX *AWSLC_EVP_MD_CTX_create(void);

OPENSSL_EXPORT void AWSLC_EVP_MD_CTX_destroy(AWSLC_EVP_MD_CTX *ctx);

OPENSSL_EXPORT int AWSLC_EVP_DigestFinalXOF(AWSLC_EVP_MD_CTX *ctx,
                                            uint8_t *out, size_t len);

OPENSSL_EXPORT int AWSLC_EVP_DigestSqueeze(AWSLC_EVP_MD_CTX *ctx,
                                           uint8_t *out, size_t len);

OPENSSL_EXPORT uint32_t AWSLC_EVP_MD_meth_get_flags(const AWSLC_EVP_MD *md);

OPENSSL_EXPORT int AWSLC_EVP_MD_nid(const AWSLC_EVP_MD *md);

OPENSSL_EXPORT void AWSLC_EVP_MD_CTX_set_pkey_ctx(AWSLC_EVP_MD_CTX *ctx,
                                                   AWSLC_EVP_PKEY_CTX *pctx);

OPENSSL_EXPORT AWSLC_EVP_PKEY_CTX *AWSLC_EVP_MD_CTX_get_pkey_ctx(
    const AWSLC_EVP_MD_CTX *ctx);

OPENSSL_EXPORT AWSLC_EVP_PKEY_CTX *AWSLC_EVP_MD_CTX_pkey_ctx(
    const AWSLC_EVP_MD_CTX *ctx);


// General No-op Functions [Deprecated].

OPENSSL_EXPORT OPENSSL_DEPRECATED void AWSLC_EVP_MD_unstable_sha3_enable(
    bool enable);

OPENSSL_EXPORT OPENSSL_DEPRECATED bool AWSLC_EVP_MD_unstable_sha3_is_enabled(
    void);

OPENSSL_EXPORT OPENSSL_DEPRECATED void AWSLC_EVP_MD_CTX_set_flags(
    AWSLC_EVP_MD_CTX *ctx, int flags);

OPENSSL_EXPORT OPENSSL_DEPRECATED int AWSLC_EVP_add_digest(
    const AWSLC_EVP_MD *digest);

OPENSSL_EXPORT OPENSSL_DEPRECATED const AWSLC_EVP_MD *AWSLC_EVP_md_null(void);


#if defined(__cplusplus)
}  // extern C
#endif

#endif  // AWSLC_HEADER_DIGEST_H
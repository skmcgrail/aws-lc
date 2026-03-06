/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 *
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 *
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 *
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.] */

#ifndef OPENSSL_HEADER_DIGEST_H
#define OPENSSL_HEADER_DIGEST_H

#include <stdbool.h>

#include <openssl/base.h>

// Import native AWS-LC digest function declarations. The canonical functions
// use the AWSLC_ prefix and are defined in awslc/digest.h. The #define macros
// below provide OpenSSL-compatible function names for backward compatibility.
//
// Note: Unlike type aliases (which must use typedef due to STACK_OF token-
// pasting), function aliases safely use #define macros because function names
// are not used in ## token-pasting contexts.
#include <awslc/digest.h>


// OpenSSL compatibility function name mappings — Hash algorithms.
#define EVP_md4 AWSLC_EVP_md4
#define EVP_md5 AWSLC_EVP_md5
#define EVP_ripemd160 AWSLC_EVP_ripemd160
#define EVP_sha1 AWSLC_EVP_sha1
#define EVP_sha224 AWSLC_EVP_sha224
#define EVP_sha256 AWSLC_EVP_sha256
#define EVP_sha384 AWSLC_EVP_sha384
#define EVP_sha512 AWSLC_EVP_sha512
#define EVP_sha512_224 AWSLC_EVP_sha512_224
#define EVP_sha512_256 AWSLC_EVP_sha512_256
#define EVP_sha3_224 AWSLC_EVP_sha3_224
#define EVP_sha3_256 AWSLC_EVP_sha3_256
#define EVP_sha3_384 AWSLC_EVP_sha3_384
#define EVP_sha3_512 AWSLC_EVP_sha3_512
#define EVP_shake128 AWSLC_EVP_shake128
#define EVP_shake256 AWSLC_EVP_shake256
#define EVP_blake2b256 AWSLC_EVP_blake2b256
#define EVP_md5_sha1 AWSLC_EVP_md5_sha1

// OpenSSL compatibility function name mappings — Digest lookup.
#define EVP_get_digestbynid AWSLC_EVP_get_digestbynid
#define EVP_get_digestbyobj AWSLC_EVP_get_digestbyobj

// OpenSSL compatibility function name mappings — Digest contexts.
#define EVP_MD_CTX_init AWSLC_EVP_MD_CTX_init
#define EVP_MD_CTX_new AWSLC_EVP_MD_CTX_new
#define EVP_MD_CTX_cleanup AWSLC_EVP_MD_CTX_cleanup
#define EVP_MD_CTX_cleanse AWSLC_EVP_MD_CTX_cleanse
#define EVP_MD_CTX_free AWSLC_EVP_MD_CTX_free
#define EVP_MD_CTX_copy_ex AWSLC_EVP_MD_CTX_copy_ex
#define EVP_MD_CTX_move AWSLC_EVP_MD_CTX_move
#define EVP_MD_CTX_reset AWSLC_EVP_MD_CTX_reset

// OpenSSL compatibility function name mappings — Digest operations.
#define EVP_DigestInit_ex AWSLC_EVP_DigestInit_ex
#define EVP_DigestInit AWSLC_EVP_DigestInit
#define EVP_DigestUpdate AWSLC_EVP_DigestUpdate
#define EVP_DigestFinal_ex AWSLC_EVP_DigestFinal_ex
#define EVP_DigestFinal AWSLC_EVP_DigestFinal
#define EVP_Digest AWSLC_EVP_Digest

// OpenSSL compatibility function name mappings — Digest function accessors.
#define EVP_MD_type AWSLC_EVP_MD_type
#define EVP_MD_flags AWSLC_EVP_MD_flags
#define EVP_MD_size AWSLC_EVP_MD_size
#define EVP_MD_block_size AWSLC_EVP_MD_block_size

// OpenSSL compatibility function name mappings — Digest operation accessors.
#define EVP_MD_CTX_md AWSLC_EVP_MD_CTX_md
#define EVP_MD_CTX_size AWSLC_EVP_MD_CTX_size
#define EVP_MD_CTX_block_size AWSLC_EVP_MD_CTX_block_size
#define EVP_MD_CTX_type AWSLC_EVP_MD_CTX_type

// OpenSSL compatibility function name mappings — ASN.1 functions.
#define EVP_parse_digest_algorithm AWSLC_EVP_parse_digest_algorithm
#define EVP_marshal_digest_algorithm AWSLC_EVP_marshal_digest_algorithm

// OpenSSL compatibility function name mappings — Deprecated functions.
#define EVP_MD_CTX_copy AWSLC_EVP_MD_CTX_copy
#define EVP_get_digestbyname AWSLC_EVP_get_digestbyname
#define EVP_MD_CTX_create AWSLC_EVP_MD_CTX_create
#define EVP_MD_CTX_destroy AWSLC_EVP_MD_CTX_destroy
#define EVP_DigestFinalXOF AWSLC_EVP_DigestFinalXOF
#define EVP_DigestSqueeze AWSLC_EVP_DigestSqueeze
#define EVP_MD_meth_get_flags AWSLC_EVP_MD_meth_get_flags
#define EVP_MD_nid AWSLC_EVP_MD_nid
#define EVP_MD_CTX_set_pkey_ctx AWSLC_EVP_MD_CTX_set_pkey_ctx
#define EVP_MD_CTX_get_pkey_ctx AWSLC_EVP_MD_CTX_get_pkey_ctx
#define EVP_MD_CTX_pkey_ctx AWSLC_EVP_MD_CTX_pkey_ctx

// OpenSSL compatibility function name mappings — No-op / deprecated functions.
#define EVP_MD_unstable_sha3_enable AWSLC_EVP_MD_unstable_sha3_enable
#define EVP_MD_unstable_sha3_is_enabled AWSLC_EVP_MD_unstable_sha3_is_enabled
#define EVP_MD_CTX_set_flags AWSLC_EVP_MD_CTX_set_flags
#define EVP_add_digest AWSLC_EVP_add_digest
#define EVP_md_null AWSLC_EVP_md_null


// EVP_MAX_MD_SIZE is the largest digest size supported, in bytes.
// Functions that output a digest generally require the buffer have
// at least this much space.
#define EVP_MAX_MD_SIZE 64  // SHA-512 is the longest so far.

// EVP_MAX_MD_CHAINING_LENGTH is the largest chaining length supported, in
// bytes. This constant is only for Merkle-Damgard-based hashed functions
// like SHA-1, SHA-2, and MD5. The chaining length is defined as the output
// length of the hash in bytes, before any truncation (e.g., 32 for SHA-224 and
// SHA-256, 64 for SHA-384 and SHA-512).
// This constant is only used internally by HMAC.
#define EVP_MAX_MD_CHAINING_LENGTH 64  // SHA-512 has the longest chaining length so far

// EVP_MAX_MD_BLOCK_SIZE is the largest digest block size supported, in
// bytes.
#define EVP_MAX_MD_BLOCK_SIZE 144      // SHA3-224 has the largest block size so far

// EVP_MD_FLAG_DIGALGID_ABSENT indicates that the parameter type in an X.509
// DigestAlgorithmIdentifier representing this digest function should be
// undefined rather than NULL.
#define EVP_MD_FLAG_DIGALGID_ABSENT 2

// EVP_MD_FLAG_XOF indicates that the digest is an extensible-output function
// (XOF).
#define EVP_MD_FLAG_XOF 4

// EVP_MD_CTX_FLAG_NON_FIPS_ALLOW is meaningless. In OpenSSL it permits non-FIPS
// algorithms in FIPS mode. But BoringSSL FIPS mode doesn't prohibit algorithms
// (it's up the the caller to use the FIPS module in a fashion compliant with
// their needs). Thus this exists only to allow code to compile.
#define EVP_MD_CTX_FLAG_NON_FIPS_ALLOW 0


#if defined(__cplusplus)
extern "C" {
#endif

struct evp_md_pctx_ops;

// env_md_ctx_st is typoed ("evp" -> "env"), but the typo comes from OpenSSL
// and some consumers forward-declare these structures so we're leaving it
// alone.
struct env_md_ctx_st {
  // digest is the underlying digest function, or NULL if not set.
  const EVP_MD *digest;
  // md_data points to a block of memory that contains the hash-specific
  // context.
  void *md_data;

  // update is usually copied from |digest->update|. This is only different
  // when consumed through |EVP_PKEY_HMAC|.
  // TODO: Look into untangling this, so that |EVP_PKEY_HMAC| can directly call
  // |digest->update|. |digest->update| operates against |md_data| above, but
  // |HMAC_CTX| maintains its own data state in |HMAC_CTX->md_ctx|.
  // |HMAC_Update| also has an additional state transition to handle.
  int (*update)(EVP_MD_CTX *ctx, const void *data, size_t count);

  // pctx is an opaque (at this layer) pointer to additional context that
  // EVP_PKEY functions may store in this object.
  EVP_PKEY_CTX *pctx;

  // pctx_ops, if not NULL, points to a vtable that contains functions to
  // manipulate |pctx|.
  const struct evp_md_pctx_ops *pctx_ops;

  // flags is only used for two cases.
  // 1. Set flag |EVP_MD_CTX_FLAG_KEEP_PKEY_CTX|, so as to let |*pctx| refrain
  //    from being freed when |*pctx| was set externally with
  //    |EVP_MD_CTX_set_pkey_ctx|.
  // 2. Set flag |EVP_MD_CTX_HMAC| for |EVP_PKEY_HMAC|.
  unsigned long flags;
} /* EVP_MD_CTX */;

#if defined(__cplusplus)
}  // extern C

#if !defined(BORINGSSL_NO_CXX)
extern "C++" {

BSSL_NAMESPACE_BEGIN

BORINGSSL_MAKE_DELETER(EVP_MD_CTX, EVP_MD_CTX_free)

using ScopedEVP_MD_CTX =
    internal::StackAllocatedMovable<EVP_MD_CTX, int, EVP_MD_CTX_init,
                                    EVP_MD_CTX_cleanup, EVP_MD_CTX_move>;

BSSL_NAMESPACE_END

}  // extern C++
#endif

#endif

#define DIGEST_R_INPUT_NOT_INITIALIZED 100
#define DIGEST_R_DECODE_ERROR 101
#define DIGEST_R_UNKNOWN_HASH 102

#endif  // OPENSSL_HEADER_DIGEST_H
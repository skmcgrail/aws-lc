/* Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0 OR ISC */

#ifndef AWSLC_HEADER_TYPES_H
#define AWSLC_HEADER_TYPES_H

/**
 * @file
 * @brief Native AWS-LC type definitions.
 *
 * This file defines the canonical AWS-LC types with the AWSLC_ prefix.
 * OpenSSL-compatible type names are provided as preprocessor macros in
 * openssl/base.h for backward compatibility.
 */

#include <stddef.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

// Primitive type aliases
typedef ptrdiff_t AWSLC_ossl_ssize_t;
typedef uint32_t AWSLC_CBS_ASN1_TAG;
typedef int AWSLC_CRYPTO_THREADID;
typedef int AWSLC_ASN1_BOOLEAN;
typedef void *AWSLC_OPENSSL_BLOCK;

// ASN.1 types
typedef struct asn1_null_st AWSLC_ASN1_NULL;
typedef struct ASN1_ITEM_st AWSLC_ASN1_ITEM;
typedef struct asn1_object_st AWSLC_ASN1_OBJECT;
typedef struct asn1_pctx_st AWSLC_ASN1_PCTX;
typedef struct asn1_string_st AWSLC_ASN1_BIT_STRING;
typedef struct asn1_string_st AWSLC_ASN1_BMPSTRING;
typedef struct asn1_string_st AWSLC_ASN1_ENUMERATED;
typedef struct asn1_string_st AWSLC_ASN1_GENERALIZEDTIME;
typedef struct asn1_string_st AWSLC_ASN1_GENERALSTRING;
typedef struct asn1_string_st AWSLC_ASN1_IA5STRING;
typedef struct asn1_string_st AWSLC_ASN1_INTEGER;
typedef struct asn1_string_st AWSLC_ASN1_OCTET_STRING;
typedef struct asn1_string_st AWSLC_ASN1_PRINTABLESTRING;
typedef struct asn1_string_st AWSLC_ASN1_STRING;
typedef struct asn1_string_st AWSLC_ASN1_T61STRING;
typedef struct asn1_string_st AWSLC_ASN1_TIME;
typedef struct asn1_string_st AWSLC_ASN1_UNIVERSALSTRING;
typedef struct asn1_string_st AWSLC_ASN1_UTCTIME;
typedef struct asn1_string_st AWSLC_ASN1_UTF8STRING;
typedef struct asn1_string_st AWSLC_ASN1_VISIBLESTRING;
typedef struct asn1_type_st AWSLC_ASN1_TYPE;

// X.509 types
typedef struct AUTHORITY_KEYID_st AWSLC_AUTHORITY_KEYID;
typedef struct BASIC_CONSTRAINTS_st AWSLC_BASIC_CONSTRAINTS;
typedef struct DIST_POINT_st AWSLC_DIST_POINT;
typedef struct GENERAL_NAME_st AWSLC_GENERAL_NAME;
typedef struct ISSUING_DIST_POINT_st AWSLC_ISSUING_DIST_POINT;
typedef struct NAME_CONSTRAINTS_st AWSLC_NAME_CONSTRAINTS;
typedef struct Netscape_spkac_st AWSLC_NETSCAPE_SPKAC;
typedef struct Netscape_spki_st AWSLC_NETSCAPE_SPKI;
typedef struct X509_VERIFY_PARAM_st AWSLC_X509_VERIFY_PARAM;
typedef struct X509_algor_st AWSLC_X509_ALGOR;
typedef struct X509_crl_st AWSLC_X509_CRL;
typedef struct X509_extension_st AWSLC_X509_EXTENSION;
typedef struct X509_info_st AWSLC_X509_INFO;
typedef struct X509_name_entry_st AWSLC_X509_NAME_ENTRY;
typedef struct X509_name_st AWSLC_X509_NAME;
typedef struct X509_pubkey_st AWSLC_X509_PUBKEY;
typedef struct X509_req_st AWSLC_X509_REQ;
typedef struct x509_sig_info_st AWSLC_X509_SIG_INFO;
typedef struct X509_sig_st AWSLC_X509_SIG;
typedef struct v3_ext_ctx AWSLC_X509V3_CTX;
typedef struct v3_ext_method AWSLC_X509V3_EXT_METHOD;
typedef struct x509_attributes_st AWSLC_X509_ATTRIBUTE;
typedef struct x509_lookup_st AWSLC_X509_LOOKUP;
typedef struct x509_lookup_method_st AWSLC_X509_LOOKUP_METHOD;
typedef struct x509_object_st AWSLC_X509_OBJECT;
typedef struct x509_revoked_st AWSLC_X509_REVOKED;
typedef struct x509_st AWSLC_X509;
typedef struct x509_store_ctx_st AWSLC_X509_STORE_CTX;
typedef struct x509_store_st AWSLC_X509_STORE;
typedef struct x509_trust_st AWSLC_X509_TRUST;
typedef struct private_key_st AWSLC_X509_PKEY;

// BigNum types
typedef struct bignum_ctx AWSLC_BN_CTX;
typedef struct bignum_st AWSLC_BIGNUM;
typedef struct bn_gencb_st AWSLC_BN_GENCB;
typedef struct bn_mont_ctx_st AWSLC_BN_MONT_CTX;

// BIO types
typedef struct bio_method_st AWSLC_BIO_METHOD;
typedef struct bio_st AWSLC_BIO;

// Buffer types
typedef struct buf_mem_st AWSLC_BUF_MEM;
typedef struct cbb_st AWSLC_CBB;
typedef struct cbs_st AWSLC_CBS;

// Symmetric cipher types
typedef struct cast_key_st AWSLC_CAST_KEY;
typedef struct rc4_key_st AWSLC_RC4_KEY;

// Hash / digest types
typedef struct blake2b_state_st AWSLC_BLAKE2B_CTX;
typedef struct RIPEMD160state_st AWSLC_RIPEMD160_CTX;
typedef struct sha256_state_st AWSLC_SHA256_CTX;
typedef struct sha512_state_st AWSLC_SHA512_CTX;
typedef struct sha_state_st AWSLC_SHA_CTX;
typedef struct md4_state_st AWSLC_MD4_CTX;
typedef struct md5_state_st AWSLC_MD5_CTX;

// CMAC / HMAC types
typedef struct cmac_ctx_st AWSLC_CMAC_CTX;
typedef struct hmac_ctx_st AWSLC_HMAC_CTX;

// Configuration types
typedef struct conf_st AWSLC_CONF;
typedef struct conf_value_st AWSLC_CONF_VALUE;

// CRYPTO buffer types
typedef struct crypto_buffer_pool_st AWSLC_CRYPTO_BUFFER_POOL;
typedef struct crypto_buffer_st AWSLC_CRYPTO_BUFFER;

// DRBG types
typedef struct ctr_drbg_state_st AWSLC_CTR_DRBG_STATE;

// DH / DSA types
typedef struct dh_st AWSLC_DH;
typedef struct dsa_st AWSLC_DSA;
typedef struct DSA_SIG_st AWSLC_DSA_SIG;

// EC types
typedef struct ec_group_st AWSLC_EC_GROUP;
typedef struct ec_key_st AWSLC_EC_KEY;
typedef struct ec_point_st AWSLC_EC_POINT;
typedef struct ec_key_method_st AWSLC_EC_KEY_METHOD;
typedef struct ecdsa_sig_st AWSLC_ECDSA_SIG;

// Engine type
typedef struct engine_st AWSLC_ENGINE;

// EVP digest types
typedef struct env_md_ctx_st AWSLC_EVP_MD_CTX;
typedef struct env_md_st AWSLC_EVP_MD;

// EVP AEAD types
typedef struct evp_aead_st AWSLC_EVP_AEAD;
typedef struct evp_aead_ctx_st AWSLC_EVP_AEAD_CTX;

// EVP cipher types
typedef struct evp_cipher_ctx_st AWSLC_EVP_CIPHER_CTX;
typedef struct evp_cipher_st AWSLC_EVP_CIPHER;

// EVP encode types
/**
 * @typedef AWSLC_EVP_ENCODE_CTX
 * @copydoc evp_encode_ctx_st
 * @see evp_encode_ctx_st
 */
typedef struct evp_encode_ctx_st AWSLC_EVP_ENCODE_CTX;

// EVP HPKE types
typedef struct evp_hpke_aead_st AWSLC_EVP_HPKE_AEAD;
typedef struct evp_hpke_ctx_st AWSLC_EVP_HPKE_CTX;
typedef struct evp_hpke_kdf_st AWSLC_EVP_HPKE_KDF;
typedef struct evp_hpke_kem_st AWSLC_EVP_HPKE_KEM;
typedef struct evp_hpke_key_st AWSLC_EVP_HPKE_KEY;

// EVP KEM types
typedef struct evp_kem_st AWSLC_EVP_KEM;
typedef struct kem_key_st AWSLC_KEM_KEY;

// EVP PKEY types
typedef struct evp_pkey_ctx_st AWSLC_EVP_PKEY_CTX;
typedef struct evp_pkey_asn1_method_st AWSLC_EVP_PKEY_ASN1_METHOD;
typedef struct evp_pkey_st AWSLC_EVP_PKEY;
typedef struct evp_pkey_ctx_signature_context_params_st AWSLC_EVP_PKEY_CTX_SIGNATURE_CONTEXT_PARAMS;

// Post-quantum types
typedef struct pqdsa_key_st AWSLC_PQDSA_KEY;

// OCSP types
typedef struct ocsp_req_ctx_st AWSLC_OCSP_REQ_CTX;

// OpenSSL init settings
typedef struct ossl_init_settings_st AWSLC_OPENSSL_INIT_SETTINGS;

// PKCS7 types
typedef struct pkcs7_digest_st AWSLC_PKCS7_DIGEST;
typedef struct pkcs7_enc_content_st AWSLC_PKCS7_ENC_CONTENT;
typedef struct pkcs7_encrypt_st AWSLC_PKCS7_ENCRYPT;
typedef struct pkcs7_envelope_st AWSLC_PKCS7_ENVELOPE;
typedef struct pkcs7_issuer_and_serial_st AWSLC_PKCS7_ISSUER_AND_SERIAL;
typedef struct pkcs7_recip_info_st AWSLC_PKCS7_RECIP_INFO;
typedef struct pkcs7_sign_envelope_st AWSLC_PKCS7_SIGN_ENVELOPE;
typedef struct pkcs7_signed_st AWSLC_PKCS7_SIGNED;
typedef struct pkcs7_signer_info_st AWSLC_PKCS7_SIGNER_INFO;
typedef struct pkcs7_st AWSLC_PKCS7;

// PKCS8 / PKCS12 types
typedef struct pkcs12_st AWSLC_PKCS12;
typedef struct pkcs8_priv_key_info_st AWSLC_PKCS8_PRIV_KEY_INFO;

// RAND types
typedef struct rand_meth_st AWSLC_RAND_METHOD;

// RSA types
typedef struct rsa_meth_st AWSLC_RSA_METHOD;
typedef struct rsassa_pss_params_st AWSLC_RSASSA_PSS_PARAMS;
typedef struct rsa_pss_params_st AWSLC_RSA_PSS_PARAMS;
typedef struct rsa_st AWSLC_RSA;

// SPAKE2 types
typedef struct spake2_ctx_st AWSLC_SPAKE2_CTX;

// SRTP types
typedef struct srtp_protection_profile_st AWSLC_SRTP_PROTECTION_PROFILE;

// SSL types
typedef struct ssl_cipher_st AWSLC_SSL_CIPHER;
typedef struct ssl_ctx_st AWSLC_SSL_CTX;
typedef struct ssl_early_callback_ctx AWSLC_SSL_CLIENT_HELLO;
typedef struct ssl_ech_keys_st AWSLC_SSL_ECH_KEYS;
typedef struct ssl_method_st AWSLC_SSL_METHOD;
typedef struct ssl_private_key_method_st AWSLC_SSL_PRIVATE_KEY_METHOD;
typedef struct ssl_quic_method_st AWSLC_SSL_QUIC_METHOD;
typedef struct ssl_session_st AWSLC_SSL_SESSION;
typedef struct ssl_st AWSLC_SSL;
typedef struct ssl_ticket_aead_method_st AWSLC_SSL_TICKET_AEAD_METHOD;

// Error types
typedef struct st_ERR_FNS AWSLC_ERR_FNS;

// Trust Token types
typedef struct trust_token_st AWSLC_TRUST_TOKEN;
typedef struct trust_token_client_st AWSLC_TRUST_TOKEN_CLIENT;
typedef struct trust_token_issuer_st AWSLC_TRUST_TOKEN_ISSUER;
typedef struct trust_token_method_st AWSLC_TRUST_TOKEN_METHOD;

#if defined(__cplusplus)
}  // extern "C"
#endif

#endif  // AWSLC_HEADER_TYPES_H
# Product Context

## What is AWS-LC?

AWS-LC (AWS libcrypto) is a general-purpose cryptographic library maintained by the AWS Cryptography team. It provides `libcrypto` (C) and `libssl` (C++) implementations that are largely API-compatible with OpenSSL 1.1.1. It is based on code from Google's BoringSSL project and the OpenSSL project.

## Why Does It Exist?

- **Security**: Maintain a secure libcrypto for AWS and its customers
- **Compatibility**: Provide OpenSSL API compatibility so existing applications can easily adopt it
- **Performance**: Include optimized assembly implementations for x86 and ARM architectures (including AWS Graviton processors)
- **Open Source Contributions**: Serve as the home for the AWS Cryptography team to publish enhancements submitted to other libcrypto projects
- **FIPS Compliance**: Support FIPS-validated cryptographic operations

## Problems It Solves

1. Provides a drop-in replacement for OpenSSL with better security and performance for AWS workloads
2. Includes formally verified implementations of critical algorithms (SHA-2, HMAC, AES-KW, AES-GCM)
3. Supports post-quantum cryptography algorithms
4. Offers optimized assembly for AWS Graviton (ARM) and Intel (x86) processors

## Current Project Goal: Type Separation

The active project goal is to **separate AWS-LC types from OpenSSL types** to create a clearer boundary between:
- The AWS-LC native types and functions
- The OpenSSL "compatibility layer" functions and types

This follows the pattern used by WolfSSL, where:
- Native functions are prefixed (e.g., `wolfSSL_EVP_sha256`)
- OpenSSL compatibility headers provide `#define` mappings (e.g., `#define EVP_sha256 wolfSSL_EVP_sha256`)

This separation will eventually enable implementing AWS-LC as an **OpenSSL 3.x provider** that can be plugged into OpenSSL without symbol conflicts.

## User Experience Goals

- Existing OpenSSL-based applications should continue to work with AWS-LC through the compatibility layer
- New applications can directly use AWS-LC native APIs for a cleaner interface
- The library should eventually integrate cleanly into OpenSSL 3.x's provider architecture
# System Patterns

## Architecture Overview

AWS-LC is a C/C++ cryptographic library structured into two main components:
- **libcrypto** (C): Core cryptographic primitives and algorithms
- **libssl** (C++): TLS/SSL protocol implementation

The codebase is derived from Google's BoringSSL with AWS-specific enhancements.

## Directory Structure

```
aws-lc/
├── include/openssl/       # Public API headers (OpenSSL-compatible)
├── crypto/                # Core crypto implementations
│   ├── fipsmodule/        # FIPS-validated module
│   ├── evp_extra/         # EVP (envelope) API implementations
│   ├── cipher_extra/      # Cipher implementations
│   ├── digest_extra/      # Digest/hash implementations
│   ├── ec_extra/          # Elliptic curve extras
│   ├── rsa_extra/         # RSA extras
│   ├── x509/              # X.509 certificate handling
│   ├── pem/               # PEM encoding
│   ├── bio/               # BIO (I/O abstraction)
│   ├── err/               # Error handling
│   ├── perlasm/           # Perl-based assembly generation
│   └── ...
├── ssl/                   # TLS/SSL implementation
├── generated-src/         # Pre-generated platform-specific assembly
├── tool/                  # bssl command-line tool
├── tool-openssl/          # OpenSSL-compatible command-line tool
├── fuzz/                  # Fuzz testing
├── tests/                 # CI test infrastructure
├── third_party/           # Third-party dependencies
├── cmake/                 # CMake build helpers
└── util/                  # Build and utility scripts
```

## Key Design Patterns

### 1. OpenSSL API Compatibility Layer (Current State)

Currently, AWS-LC directly uses OpenSSL type names and function signatures in `include/openssl/`. The types are defined in `include/openssl/base.h` using typedefs that map internal struct names to OpenSSL-compatible type names:

```c
typedef struct evp_cipher_ctx_st EVP_CIPHER_CTX;
typedef struct evp_cipher_st EVP_CIPHER;
typedef struct env_md_ctx_st EVP_MD_CTX;
typedef struct env_md_st EVP_MD;
typedef struct rsa_st RSA;
typedef struct ec_key_st EC_KEY;
// ... etc
```

There is no separation between "AWS-LC native" and "OpenSSL compatibility" — they are currently one and the same.

### 2. Type and Function Separation Pattern (WolfSSL-style, Implemented)

AWS-LC now uses an `AWSLC_` prefix pattern to create distinct native types/functions separate from OpenSSL compatibility mappings:

**Header structure**:
```
include/
├── awslc/
│   ├── types.h          # Native AWSLC_ typedefs (the "real" types)
│   └── digest.h         # Native AWSLC_ digest function declarations
└── openssl/
    ├── base.h            # Type compatibility (typedef aliases)
    ├── digest.h          # Function compatibility (#define macros) + struct defs + C++ helpers
    └── ...
```

**Type aliases use typedef** (NOT #define):
```c
// awslc/types.h — native type
typedef struct env_md_st AWSLC_EVP_MD;
// openssl/base.h — compatibility alias
typedef AWSLC_EVP_MD EVP_MD;
```
Reason: `STACK_OF()` and `DEFINE_STACK_OF()` use `##` token-pasting. `#define` macros expand before `##`, breaking the generated struct names. Typedef aliases are C-level constructs that don't interfere with the preprocessor.

**Function aliases use #define macros**:
```c
// awslc/digest.h — native declaration
OPENSSL_EXPORT const AWSLC_EVP_MD *AWSLC_EVP_sha256(void);
// openssl/digest.h — compatibility macro
#define EVP_sha256 AWSLC_EVP_sha256
```
Reason: Function names are never used in `##` token-pasting contexts, so `#define` macros work correctly. They also rename implementation-side symbols transparently (including `DEFINE_METHOD_FUNCTION` internal helpers).

**Note**: This is distinct from the existing `BORINGSSL_PREFIX` symbol prefixing mechanism, which serves a different purpose (avoiding symbol collisions when multiple BoringSSL-derived libraries are linked together).

### 3. FIPS Module Boundary

The `crypto/fipsmodule/` directory contains the FIPS-validated cryptographic module with its own boundary and self-tests. This boundary must be preserved during any refactoring.

### 4. C++ Smart Pointer Pattern

AWS-LC provides `bssl::UniquePtr<T>` for C++ consumers, with `BORINGSSL_MAKE_DELETER` macros to define type-specific deleters. Any type renaming must maintain this pattern.

### 5. Platform-Specific Assembly

Optimized assembly implementations exist for multiple platforms:
- `generated-src/linux-x86_64/`, `generated-src/linux-aarch64/`, etc.
- Generated via Perl scripts in `crypto/perlasm/`

## Component Relationships

```
[Application Code]
        |
        v
[include/openssl/*.h]  <-- Public API (OpenSSL-compatible)
        |
        v
[crypto/*.c]           <-- C implementations
[ssl/*.cc]             <-- C++ TLS implementation
        |
        v
[crypto/fipsmodule/]   <-- FIPS boundary
        |
        v
[Platform Assembly]    <-- Optimized per-platform code
```

## Build System

- **CMake** is the primary build system
- **Ninja** is the recommended build tool
- **Perl** generates assembly from `perlasm/` scripts
- **Go** is used for test runners and utilities
- Pre-generated files in `generated-src/` allow building without Go/Perl
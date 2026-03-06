# Progress

## Project Status: Type Separation Complete + Digest Function Separation Complete

All ~100 types and all ~50 digest functions have been separated into native `AWSLC_` prefixed symbols with OpenSSL-compatible aliases.

## What Works

### AWS-LC Baseline
- AWS-LC builds and passes all tests as a fully functional cryptographic library
- OpenSSL 1.1.1 API compatibility is in place
- FIPS module is validated and operational
- Optimized assembly for x86 and ARM platforms
- Version 1.69.0 with API version 35

### Type Separation (Complete)
- All ~100 types separated: native `AWSLC_` types in `include/awslc/types.h`
- OpenSSL compatibility via typedef aliases in `include/openssl/base.h`
- Header structure: `awslc/types.h` (native) → `openssl/base.h` (compatibility)

### Function Separation — Digest (Complete)
- All ~50 digest functions separated: native `AWSLC_` functions in `include/awslc/digest.h`
- OpenSSL compatibility via `#define` macros in `include/openssl/digest.h`
- Header structure: `awslc/digest.h` (native declarations) → `openssl/digest.h` (compat macros + struct def + C++ helpers)
- Full build succeeds: 346 targets (libcrypto, libssl, tools, all tests)
- All tests pass: crypto (2594), SSL (2550), 0 failures
- Symbol verification: `nm` confirmed `AWSLC_EVP_sha256`, `AWSLC_EVP_DigestInit`, `AWSLC_EVP_MD_CTX_new` in FIPS and non-FIPS objects
- `DEFINE_METHOD_FUNCTION` creates consistent internal helpers (e.g., `AWSLC_EVP_sha256_storage`)

## What's Left to Build

- [x] Finalize naming convention for AWS-LC native prefix → `AWSLC_` for both types and functions
- [x] Select initial subset of types/functions for proof-of-concept → EVP_MD, EVP_MD_CTX
- [x] Implement proof-of-concept type separation on small API surface
- [x] Validate backward compatibility with existing consumers
- [x] Extend type separation to ALL types in `base.h` (~100 types)
- [x] Restructure headers: native types in `awslc/types.h`, compat in `openssl/base.h`
- [x] Update CMakeLists.txt with install rule for `awslc/` headers
- [x] Implement function separation for digest functions (AWSLC_ prefix + #define compat macros)
- [x] Validate DEFINE_METHOD_FUNCTION compatibility with #define macros
- [x] Verify symbol names in built library (nm confirmed AWSLC_ prefix)
- [ ] Extend function separation to cipher functions (`cipher.h`)
- [ ] Extend function separation to EVP functions (`evp.h`)
- [ ] Extend function separation to RSA functions (`rsa.h`)
- [ ] Extend function separation to EC functions (`ec.h`)
- [ ] Extend function separation to remaining headers
- [ ] Design header structure for functions (native `awslc/*.h` vs. compatibility `openssl/*.h`)
- [ ] Address assembly symbol implications (when function renaming happens)
- [ ] Address C++ `bssl::UniquePtr<T>` / `BORINGSSL_MAKE_DELETER` patterns for function names
- [ ] Document the pattern for contributors
- [ ] Prepare for eventual OpenSSL 3.x provider implementation

## Known Considerations

- **`base.h.in` is the source of truth for types** — `base.h` is generated from it via CMake `configure_file`; never edit `base.h` directly
- **`awslc/digest.h` is the source of truth for digest functions** — `openssl/digest.h` provides only compat macros + struct def + C++ helpers
- **Typedef aliases (NOT #define macros) for type compatibility** — `STACK_OF()` uses `##` token-pasting which breaks `#define` macros; typedef aliases are preprocessor-transparent
- **`#define` macros ARE correct for function compatibility** — function names aren't used in `##` token-pasting contexts
- **`#define` macros expand BEFORE DEFINE_METHOD_FUNCTION's `##`** — creating consistent internal helper names (e.g., `AWSLC_EVP_sha256_storage`, `AWSLC_EVP_sha256_init`)
- The `BORINGSSL_PREFIX` mechanism is orthogonal and should not be conflated with type/function separation
- FIPS module boundary must be preserved — no issues observed with either typedef or #define approach
- C++ `bssl::UniquePtr<T>` smart pointer patterns work transparently via typedef aliases
- C++ `BORINGSSL_MAKE_DELETER` and `ScopedEVP_MD_CTX` work via the `#define` macros transparently
- Assembly code references symbols that may be affected when functions are renamed
- Pre-generated assembly in `generated-src/` will need regeneration if symbols change
- Struct definition (`env_md_ctx_st`) stays in `openssl/digest.h` since it uses compat type names and is consumed by internal code

## Evolution of Decisions

1. **2026-03-05**: Project brief created. WolfSSL prefix pattern identified as the model. Goal is OpenSSL 3.x provider integration.
2. **2026-03-05**: Memory bank initialized. Clarified that `BORINGSSL_PREFIX` is unrelated to this work.
3. **2026-03-05**: Naming convention decided: `AWSLC_` prefix for both types and functions.
4. **2026-03-05**: Proof-of-concept implemented — typedef rename + `#define` compat macros in `base.h` for `EVP_MD` and `EVP_MD_CTX`. Full build and all tests pass.
5. **2026-03-05**: Extended to all types. Discovered `#define` macros break `STACK_OF()` due to `##` token-pasting. Switched to typedef aliases. Created `include/awslc/types.h` for native types. Full build (557 targets) and all tests pass (crypto: 2594, SSL: 2550, 0 failures).
6. **2026-03-06**: Implemented digest function separation. Created `include/awslc/digest.h` with ~50 `AWSLC_` prefixed function declarations. Modified `openssl/digest.h` to use `#include <awslc/digest.h>` + `#define` compat macros. Verified `#define` macros work correctly with `DEFINE_METHOD_FUNCTION` — `##` token-pasting creates consistent internal helpers. Full build (346 targets) and all tests pass. Symbol verification via `nm` confirmed `AWSLC_EVP_sha256`, `AWSLC_EVP_DigestInit`, `AWSLC_EVP_MD_CTX_new` properly exported.
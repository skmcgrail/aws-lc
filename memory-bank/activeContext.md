# Active Context

## Current Work Focus

Function separation for digest functions is complete and validated. Following the type separation pattern, digest functions now have native `AWSLC_` prefixed symbols in `include/awslc/digest.h` with `#define` compatibility macros in `openssl/digest.h`.

## Core Objective

Separate AWS-LC's own types and functions from OpenSSL-compatible types and functions, following a WolfSSL-inspired prefix pattern with `AWSLC_` for both types and functions.

## What Was Done

### Phase 1: Full Type Separation (Previously Complete)

**Architecture**:
```
include/
├── awslc/
│   └── types.h          # Native AWSLC_ typedefs (the "real" types)
└── openssl/
    ├── base.h            # Generated from base.h.in; includes awslc/types.h + typedef aliases
    ├── base.h.in         # Template with typedef aliases
    └── ...               # All other headers unchanged
```

### Phase 2: Digest Function Separation (Just Completed)

**Architecture**:
```
include/
├── awslc/
│   ├── types.h          # Native AWSLC_ typedefs
│   └── digest.h         # Native AWSLC_ digest function declarations (NEW)
└── openssl/
    ├── base.h            # Type compatibility aliases (typedef)
    ├── digest.h          # Includes awslc/digest.h + #define function compat macros (MODIFIED)
    └── ...
```

**Files changed**:
1. **`include/awslc/digest.h`** (NEW) — All ~50 digest function declarations with `AWSLC_` prefix
2. **`include/openssl/digest.h`** (MODIFIED) — Replaced function declarations with `#include <awslc/digest.h>` + `#define` compatibility macros

**Approach (function aliases use #define macros)**:
- In `awslc/digest.h`: `OPENSSL_EXPORT const AWSLC_EVP_MD *AWSLC_EVP_sha256(void);`
- In `openssl/digest.h`: `#define EVP_sha256 AWSLC_EVP_sha256`

**Why #define macros work for functions (but not types)**:
- Function names are never used in `##` token-pasting contexts
- `#define` expands before `DEFINE_METHOD_FUNCTION` macro's `##`, creating consistent internal helpers (e.g., `AWSLC_EVP_sha256_storage`, `AWSLC_EVP_sha256_init`)
- Implementation files don't need changes — the `#define` macros handle renaming transparently

### Validation Results
- **Build**: Full project compiled cleanly — 346 targets (libcrypto, libssl, all tools, all tests)
- **Crypto tests**: 2594/2595 passed (1 skipped, 0 failed)
- **SSL tests**: 2550/2577 passed (27 skipped, 0 failed)
- **Symbol verification**: `nm` confirmed `AWSLC_EVP_sha256`, `AWSLC_EVP_DigestInit`, `AWSLC_EVP_MD_CTX_new` are properly exported in the FIPS module and non-FIPS objects

## Key Decisions Made

1. **Prefix convention**: `AWSLC_` for both types and functions
2. **Type aliases**: typedef aliases in `openssl/base.h` (NOT #define, due to STACK_OF `##` token-pasting)
3. **Function aliases**: `#define` macros in `openssl/digest.h` (safe — function names aren't token-pasted)
4. **Header structure**: Native declarations in `awslc/`, compatibility in `openssl/`
5. **Implementation files unchanged**: Macros handle renaming transparently
6. **DEFINE_METHOD_FUNCTION compatibility**: `#define` expands BEFORE `##` token-pasting, creating consistent helper names

## Next Steps

1. **Extend function separation to other headers**: Apply the same pattern to `cipher.h`, `evp.h`, `rsa.h`, `ec.h`, etc.
   - Create `awslc/cipher.h`, `awslc/evp.h`, etc.
   - Add `#define` macros to corresponding `openssl/` headers
2. **Assembly symbols**: Verify that assembly-level symbols are also renamed (they should be, since assembly calls the C symbols)
3. **Document the pattern**: Create a contributor guide for adding new prefixed functions
4. **FIPS implications**: Confirmed — no issues with the `#define` approach for functions

## Important Patterns and Preferences

- **Types use typedef aliases** (NOT #define) — `STACK_OF()` uses `##` token-pasting
- **Functions use #define macros** — function names aren't token-pasted
- **`base.h.in` is the template for types**: Always edit `base.h.in`, not `base.h` directly
- **`awslc/digest.h` is the source of truth for digest functions**: `openssl/digest.h` provides only compat macros + struct definition + C++ helpers
- Keep the `BORINGSSL_PREFIX` mechanism separate; it is orthogonal to this work
- Maintain backward compatibility for existing OpenSSL API consumers
- The struct definition (`env_md_ctx_st`) stays in `openssl/digest.h` since it uses compat type names
- C++ `BORINGSSL_MAKE_DELETER` and `ScopedEVP_MD_CTX` work via the `#define` macros transparently
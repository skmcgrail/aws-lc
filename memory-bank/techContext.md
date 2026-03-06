# Technical Context

## Technologies Used

### Languages
- **C**: Core `libcrypto` implementation
- **C++** (C++11): `libssl` implementation and some test code
- **Go**: Test runners, utility scripts (`util/all_tests.go`, `util/read_symbols.go`)
- **Perl**: Assembly generation scripts (`crypto/perlasm/`)
- **Python**: Some utility/test scripts
- **NASM**: Windows assembly

### Build System
- **CMake** (3.0+): Primary build system
- **Ninja**: Recommended build tool (faster than Make)
- **Make**: Alternative build tool (non-Windows)

### Key Build Dependencies
- C/C++ compiler with C++11 support (GCC 4.1.3+, Clang, or MSVC 14+)
- CMake 3.0+
- Perl (for assembly generation; optional with pre-generated files)
- Go 1.17.13+ (for test infrastructure; optional with pre-generated files)
- NASM (Windows only, for assembly)
- libunwind (optional, for x86_64 Linux assembly testing)

### Build Commands
```bash
# Debug build with Ninja
cmake -GNinja -B build
ninja -C build

# Release build
cmake -GNinja -B build -DCMAKE_BUILD_TYPE=Release
ninja -C build

# Run tests
ninja -C build run_tests

# Shared library build
cmake -GNinja -B build -DBUILD_SHARED_LIBS=1
```

### Key CMake Options
- `-DCMAKE_BUILD_TYPE=Release|RelWithDebInfo|Debug`
- `-DBUILD_SHARED_LIBS=1` — Build as shared library
- `-DOPENSSL_SMALL=1` — Optimize for binary size over performance
- `-DBORINGSSL_PREFIX=PREFIX` — Add prefix to all symbols (unrelated to type separation work)
- `-DENABLE_DATA_INDEPENDENT_TIMING=ON` — Enable AArch64 DIT support

## Testing Infrastructure

- **Unit tests**: C/C++ tests built by CMake/Ninja
- **Blackbox tests**: Go-based TLS tests in `ssl/test/runner/`
- **Fuzz tests**: In `fuzz/` directory
- **Sanitizers**: ASan, MSan, CFI, TSan, UBSan
- **Valgrind**: Memory checking
- **Formal verification**: Via [AWS-LC Formal Verification](https://github.com/awslabs/aws-lc-verification)
- **CI**: Automated testing on every change

## Platform Support

### Primary (CI-tested)
| OS | CPU |
|---------|---------|
| Linux | x86, x86-64, aarch64, ppc, ppc64, ppc64le |
| Windows | x86-64 |
| macOS | x86-64, aarch64 |
| Android | aarch64 |

### Secondary (community interest)
- iOS (aarch64), Android (arm32), Emscripten (wasm32)
- Linux: arm32, loongarch64, risc-v64, s390x
- Windows: aarch64
- OpenBSD/FreeBSD: x86-64

## Repository Information

- **Origin**: `git@github.com:skmcgrail/aws-lc`
- **Upstream**: `git@github.com:aws/aws-lc`
- **Fork of**: Google BoringSSL + OpenSSL heritage
- **Version**: 1.69.0 (as of `AWSLC_VERSION_NUMBER_STRING` in `base.h`)
- **API Version**: `AWSLC_API_VERSION 35`

## Development Environment

- **OS**: Linux 6.1
- **IDE**: VS Code
- **Shell**: zsh
- **Nix**: `flake.nix` and `nix/devshell.nix` available for reproducible development environment
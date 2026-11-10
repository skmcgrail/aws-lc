# Rust in libcrypto: working notes

Context for agents and contributors working on the Rust integration. The
user-facing overview is in `README.md`; this file records decisions, the
reasons for them, gotchas, and open research. Last updated 2026-11-10.

## Status

- Experimental and opt-in: `-DENABLE_RUST=ON` (default OFF). Crates are not
  published (`publish = false`).
- `caracal`: native Rust implementations (safe Rust, no C types).
- `caracal-ffi`: C ABI over `caracal`, built as a `staticlib`.
- Only a placeholder API exists: C `CRYPTO_rust_byte_sum` (`rust.c`) calls
  Rust `caracal_byte_sum` (`caracal-ffi`), which calls `caracal::byte_sum`.

## Where things are wired

| What | Where |
| --- | --- |
| `ENABLE_RUST` option, unsupported-combination guards, `include(cmake/cargo.cmake)` | top-level `CMakeLists.txt` |
| `cargo_add_staticlib(caracal_ffi ...)` call, pkg-config `Libs.private` | top-level `CMakeLists.txt`, just before `add_subdirectory(crypto)` |
| cargo discovery, `RUST_TARGET`, `cargo_add_staticlib()` | `cmake/cargo.cmake` |
| `rust/rust.c` + `AWSLC_RUST` on `crypto_objects`; linking in `build_libcrypto()`; `rust_test.cc` in `crypto_test`; static install rule | `crypto/CMakeLists.txt` |
| Header generation | `util/generate_caracal_ffi_header.sh`, `caracal-ffi/cbindgen.toml` |
| CI header drift check | `.github/workflows/rust.yml` |

## Decisions and why

- **Location `crypto/rust/`**: Rust belongs to libcrypto (the C library), and
  is kept out of `crypto/fipsmodule/`. `AWSLC_RUST` is defined only for
  `crypto_objects` and `crypto_test`, so fipsmodule code cannot call Rust.
- **`no_std` + `panic = "abort"`**: no Rust std, allocator, runtime, or system
  libs in libcrypto; unwinding across the C ABI is UB. `caracal` uses
  `#![cfg_attr(not(test), no_std)]` so `cargo test` still works with std.
- **Runtime support in `caracal-ffi` (`mod rt`, `cfg(not(test))`)**: a
  `#[panic_handler]` that calls libc `abort`, and a `rust_eh_personality`
  stub that aborts. The stub is needed because the prebuilt `core` is
  compiled for unwinding; static Debug links failed with
  `undefined reference to rust_eh_personality` without it. Fat LTO alone did
  not remove the reference for the staticlib.
- **Rust symbols are internal to libcrypto.** The C wrapper is the only
  exported entry point (`OPENSSL_EXPORT`, for tests in shared builds). Its
  declaration in `internal.h` is guarded by `AWSLC_RUST` so the symbol
  registry tooling (`util/read_public_symbols`, which preprocesses headers
  without `AWSLC_RUST`) does not see it and CI's baseline check passes.
- **Shared builds**: the staticlib carries ~550 GLOBAL DEFAULT symbols
  (mangled `core`/`compiler_builtins`), so `crypto` links it with
  `-Wl,--exclude-libs,libcaracal_ffi.a` (ELF only). Verified: no Rust symbols
  in `nm -D` of `libcrypto.so`.
- **Static builds**: `libcaracal_ffi.a` is installed next to `libcrypto.a`;
  the exported target uses
  `$<INSTALL_INTERFACE:\${_IMPORT_PREFIX}/<libdir>/libcaracal_ffi.a>` and
  pkg-config gets `Libs.private: -lcaracal_ffi`. Verified with out-of-tree
  `find_package(crypto)` and `pkg-config --static` consumers.
- **Imported target defined at top level**, not in `crypto/`, so
  `$<LINK_ONLY:caracal_ffi>` resolves from every directory that links
  `crypto` (imported targets are directory-scoped; `GLOBAL` needs CMake 3.11,
  minimum is 3.5).
- **Cargo invocation**: an always-run `add_custom_target` with `BYPRODUCTS`
  (Ninja restat avoids needless relinks); cargo does its own dependency
  tracking. `--locked`, `--profile dev` for Debug or no build type,
  `--profile release` otherwise; per-config genexes for multi-config
  generators (genex `BYPRODUCTS` only on CMake >= 3.20). Output in
  `<build>/cargo`.
- **Cross-compiling**: configure fails if `CMAKE_CROSSCOMPILING` and
  `RUST_TARGET` is empty (otherwise a host-arch archive would link-fail
  confusingly).
- **Unsupported combinations rejected at configure**: `FIPS` (no Rust module
  boundary defined), `BORINGSSL_PREFIX` (Rust symbols are not renamed), and
  `ENABLE_SYMBOL_VERSIONING` (`local: *` would hide the unregistered test
  hook; note this is on by default with `ENABLE_DIST_PKG`).
- **Toolchain**: edition 2024, `rust-version = "1.85"`. Lints:
  `unsafe_op_in_unsafe_fn` deny, `clippy::undocumented_unsafe_blocks` deny,
  `missing_docs` warn.
- **C header is generated, committed, and drift-checked** (chosen over
  `build.rs`, which would pull ~30 crates from crates.io into every build, and
  over a cbindgen build-time requirement). Mirrors how `crypto/libcrypto.map`
  is handled. cbindgen is pinned to 0.29.4 in the script; CI reads the pin
  from the script. `include/.clang-format` disables formatting so
  format-on-save cannot break the drift check.
- **cbindgen config notes**: `usize_is_size_t = true` (else `uintptr_t`);
  `[export] exclude = ["abort", "rust_eh_personality"]` (cbindgen also emits
  items from `extern` blocks); header parameter names come from Rust, and
  `in` is a Rust keyword, so FFI params are `input`/`input_len`; Rust doc
  comments are copied verbatim, so write them for C readers.

## Verifying changes

```sh
# Rust
cd crypto/rust
cargo fmt --check && cargo clippy --all-targets && cargo test
cargo +1.86.0 build --locked          # closest installed toolchain to the MSRV
util/generate_caracal_ffi_header.sh --check   # from the repo root

# C integration (repeat with -DBUILD_SHARED_LIBS=ON and CMAKE_BUILD_TYPE=Release)
cmake -GNinja -B build -DENABLE_RUST=ON -DCMAKE_BUILD_TYPE=Debug
ninja -C build crypto_test && ./build/crypto/crypto_test --gtest_filter='RustTest.*'
nm -D --defined-only build/crypto/libcrypto.so | grep -E ' (caracal_|_RN|rust_eh)'  # expect nothing
```

Verified on 2026-11-10 (Linux x86_64): shared Release/Debug, static Debug,
Ninja Multi-Config Debug/Release; full `crypto_test` on shared Release (3109
passed, 1 environment-dependent skip); static install consumed via CMake and
pkg-config; Rust edits trigger relink and a no-op build does not; default
OFF build has no cargo steps; all three guards fire. Rust builds on nightly
1.101, stable, and 1.86.0.

## Gotchas

- The interactive shell is zsh: unquoted `$var` does not word-split, so a
  loop over flag strings passes them as one argument. Use `bash -c` or
  `${=var}`.
- rustfmt can move a `// SAFETY:` comment away from its `unsafe` block (e.g.
  inside `assert_eq!(...)`), which then fails clippy. Bind to a local first.
- Nightly cargo writes `--emit=obj` output under
  `target/<profile>/build/<pkg>/<hash>/out/`; use `--emit=obj=<path>`.
- Cargo does not track `--emit=obj=<path>` outputs: delete the file and
  re-run, and cargo reports `Finished` without regenerating it.
- Host tools here: GNU `ld`/`objcopy` 2.41 are x86-only; `clang`,
  `llvm-objcopy`, `llvm-nm`, `llvm-ar` exist; no `ld.lld`, no aarch64 GCC.
  Nightly has `rust-src` and `llvm-tools`, so `-Zbuild-std=core --offline`
  can build cross objects without downloading a target. Installed rustup
  targets: x86_64 gnu and musl only. cbindgen 0.29.4 is in `~/.cargo/bin`.

## Research areas

### 1. One static library instead of two (investigated, not implemented)

Goal: `libcrypto.a` self-contained, so no `libcaracal_ffi.a` install,
`_IMPORT_PREFIX` workaround, pkg-config entry, or `--exclude-libs`.

- **`ar` merge**: one archive, but every Rust symbol stays global. Rejected.
- **`ld -r` + `objcopy`** (prototyped by hand): `ld -r -u caracal_byte_sum
  libcaracal_ffi.a -o caracal_ffi.o`, then
  `objcopy --wildcard --keep-global-symbol='caracal_*'`. Result: 3.2 MB
  Debug object (archive 8.3 MB), only `caracal_byte_sum` global, undefined
  only `abort bcmp memcmp memcpy memset`. Appended to `libcrypto.a`, a
  consumer linked with just `-lcrypto -lpthread`. Cost: needs target-aware
  `ld`/`objcopy` when cross-compiling (or LLVM tools); macOS `ld -r` with
  `-exported_symbols_list` untested; MSVC has no partial link.
- **rustc fat LTO object** (prototyped; recommended): `lto = "fat"`,
  `codegen-units = 1`, `cargo rustc -p caracal-ffi -- --emit=obj=<path>`.
  LTO internalizes everything except `#[no_mangle]` exports. Results, all
  without the personality stub:

  | Build | Size | Globals | Undefined |
  | --- | --- | --- | --- |
  | x86_64 release | 1.2 KB | `caracal_byte_sum` | none |
  | x86_64 dev, `opt-level=1` (debug assertions on) | 12 KB | `caracal_byte_sum` | none |
  | aarch64 release, built on x86_64 with rustc only | 2.2 KB | `caracal_byte_sum` | none |
  | x86_64 dev, `opt-level=0` | 2 MB | `caracal_byte_sum`, `DW.ref.rust_eh_personality` | `__udivti3 __umodti3 __extendhfsf2 __truncsfhf2 rust_eh_personality` + libc |

  So Debug needs `opt-level >= 1`; then the stub can be deleted (it is a
  global that could clash with std's in a Rust consumer). The object would be
  added to `crypto` as an `EXTERNAL_OBJECT`/`GENERATED` source, as the FIPS
  build does with `bcm.o`. Works for static and shared, any object format,
  any target rustc supports.

  Status of the technique: `--emit=obj` and cross-crate fat LTO are both
  documented and stable; using them together as a packaging mode is not
  documented. Risks: the object only contains dependency code because of
  fat LTO; `compiler_builtins` is never LTO'd and is dropped, so intrinsics
  come from libgcc/compiler-rt (older libgcc lacks `__extendhfsf2`); "no
  undefined symbols" is an optimizer outcome; cargo does not track the
  output. Failures are link errors, not miscompiles. Planned guardrails:
  build-time check that globals match `caracal_*` and undefined symbols are
  on an allowlist; force a cargo rebuild when the object is missing; a CI
  cross build.
- **Hiding the FFI symbols in shared builds without `--exclude-libs`**:
  declare them hidden on the C side (`#pragma GCC visibility push(hidden)`,
  via cbindgen `after_includes`). ELF keeps the most restrictive
  visibility; tested with GCC and Clang, no `caracal_*` in `nm -D`.
- No stable way to make rustc emit hidden visibility for staticlib internals
  (`-Zdefault-visibility=hidden` is nightly-only).

### 2. Cross-compiling

- Auto-derive `RUST_TARGET` from `CMAKE_C_COMPILER_TARGET` or
  `${CMAKE_C_COMPILER} -dumpmachine`, with a mapping table and a fallback to
  requiring it. Not 1:1 (vendor field, ARM `armv7` vs `arm` and float ABI,
  Android API levels, Apple deployment targets).
- Detect a Rust/C target mismatch at build time (compare object
  architecture). Today a static build with tests off would only fail at the
  consumer's link.
- Apple: pass the deployment target to rustc; universal builds
  (`CMAKE_OSX_ARCHITECTURES="arm64;x86_64"`) need one object per arch plus
  `lipo`.
- Windows (MSVC and MinGW), Android, and BSD are untested.
- Cargo never links a staticlib or `--emit=obj` output, so no target linker
  config (`CARGO_TARGET_<TRIPLE>_LINKER`) is needed; only the target's `core`
  (`rustup target add <triple>`).

### 3. Rust programs consuming static libcrypto (aws-lc-rs / aws-lc-sys)

With the current staticlib, a Rust binary can hit duplicate symbols from
`core` (same toolchain gives the same mangled names) or from the
`rust_eh_personality` stub. Research area 1's LTO object should remove this.
Not yet tested against aws-lc-rs.

### 4. Lifting the guards

- **`BORINGSSL_PREFIX`**: rename `caracal_*` (objcopy `--redefine-syms`, or a
  build-time prefix fed to `#[export_name]`); check interplay with the
  prefix symbol list generated by `util/read_symbols.go`.
- **Symbol versioning**: register the test hook as `PRIVATE` with
  `util/update_symbol_version.sh`, or test differently.
- **FIPS**: decide whether Rust may ever be inside the module boundary;
  today it is outside and the combination is rejected.

### 5. Other

- CI only checks header drift. Add jobs that build and test with
  `ENABLE_RUST` (static and shared), run `cargo fmt/clippy/test`, check the
  MSRV, and do a cross build.
- `cargo test` is not part of `run_tests`.
- macOS shared builds: `--exclude-libs` is not applied, so Rust `core`
  symbols would be exported (moot if area 1 lands).
- Ninja Multi-Config on CMake 3.17-3.19 lacks genex `BYPRODUCTS`; untested.
- Fat LTO re-optimizes `core` on every Rust change and disables incremental
  builds; watch build times as real code lands.
- `alloc` support, if needed: a `#[global_allocator]` over `OPENSSL_malloc`.
- Constant-time code in Rust: the optimizer can undo branch-free code; needs
  a value-barrier strategy matching libcrypto's C `value_barrier_w`
  approach before porting secret-dependent code.

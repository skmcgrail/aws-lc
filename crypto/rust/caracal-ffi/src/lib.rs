// Copyright Amazon.com Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0 OR ISC

//! C ABI for `caracal`, built as a static library and linked into libcrypto.
//!
//! `include/caracal_ffi.h` is generated from this file by cbindgen. Run
//! `util/generate_caracal_ffi_header.sh` after changing any exported item.
//! Doc comments on exported items are copied into the header, so write them
//! for C readers.
//!
//! Functions follow libcrypto conventions: they return 1 on success and 0 on
//! failure, and never panic across the boundary.
//!
//! These symbols are internal to libcrypto. C code in libcrypto calls them;
//! they are not part of AWS-LC's public API.

#![cfg_attr(not(test), no_std)]

use core::ffi::c_int;

/// Builds a byte slice from a C pointer and length.
///
/// Returns `None` if `ptr` is NULL with a non-zero `len`, or if `len` exceeds
/// `isize::MAX`. A zero `len` yields an empty slice regardless of `ptr`, since
/// C callers commonly pass NULL for empty buffers but `from_raw_parts` forbids
/// it.
///
/// # Safety
///
/// If `len` is non-zero and `ptr` is non-NULL, `ptr` must be valid for reads
/// of `len` bytes for the lifetime `'a`, and the memory must not be mutated
/// during that lifetime.
unsafe fn slice_from_c<'a>(ptr: *const u8, len: usize) -> Option<&'a [u8]> {
    if len == 0 {
        return Some(&[]);
    }
    if ptr.is_null() || len > isize::MAX as usize {
        return None;
    }
    // SAFETY: `ptr` is non-NULL, `len` is in range, and the caller guarantees
    // `ptr` is valid for reads of `len` bytes for `'a`.
    Some(unsafe { core::slice::from_raw_parts(ptr, len) })
}

/// Writes the sum of the `input_len` bytes at `input` to `*out`.
///
/// Returns 1 on success, or 0 if `out` is NULL or `input` is NULL with a
/// non-zero `input_len`.
///
/// # Safety
///
/// `input` must be NULL or valid for reads of `input_len` bytes. `out` must be
/// NULL or valid for writing a `uint64_t`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn caracal_byte_sum(
    input: *const u8,
    input_len: usize,
    out: *mut u64,
) -> c_int {
    if out.is_null() {
        return 0;
    }
    // SAFETY: forwarded from this function's contract.
    let Some(input) = (unsafe { slice_from_c(input, input_len) }) else {
        return 0;
    };
    // SAFETY: `out` is non-NULL and the caller guarantees it is writable.
    unsafe { out.write(caracal::byte_sum(input)) };
    1
}

/// Runtime support required because this is a `no_std` staticlib. Excluded
/// from `cargo test`, where `std` provides them.
#[cfg(not(test))]
mod rt {
    unsafe extern "C" {
        fn abort() -> !;
    }

    #[panic_handler]
    fn panic(_info: &core::panic::PanicInfo<'_>) -> ! {
        // SAFETY: `abort` takes no arguments and never returns.
        unsafe { abort() }
    }

    /// The prebuilt `core` library is compiled for unwinding, so its objects
    /// reference the personality routine even though this crate is built with
    /// `panic = "abort"`. Defining the real one requires nightly. Nothing can
    /// unwind through Rust frames here, so it is unreachable; abort if not.
    #[unsafe(no_mangle)]
    extern "C" fn rust_eh_personality() -> ! {
        // SAFETY: `abort` takes no arguments and never returns.
        unsafe { abort() }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use core::ptr;

    #[test]
    fn byte_sum_ok() {
        let data = [1u8, 2, 3];
        let mut out = 0u64;
        // SAFETY: `data` and `out` are valid for the call.
        let ret = unsafe { caracal_byte_sum(data.as_ptr(), data.len(), &mut out) };
        assert_eq!(ret, 1);
        assert_eq!(out, 6);
    }

    #[test]
    fn byte_sum_null_empty_input() {
        let mut out = 1u64;
        // SAFETY: NULL with zero length is permitted; `out` is valid.
        assert_eq!(unsafe { caracal_byte_sum(ptr::null(), 0, &mut out) }, 1);
        assert_eq!(out, 0);
    }

    #[test]
    fn byte_sum_rejects_null() {
        let mut out = 0u64;
        // SAFETY: NULL pointers are rejected before any dereference.
        unsafe {
            assert_eq!(caracal_byte_sum(ptr::null(), 1, &mut out), 0);
            assert_eq!(caracal_byte_sum([0u8].as_ptr(), 1, ptr::null_mut()), 0);
        }
    }
}

// Copyright Amazon.com Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0 OR ISC

//! Native Rust implementations used by AWS-LC's libcrypto.
//!
//! This crate is safe, idiomatic Rust with no knowledge of C. The C ABI lives
//! in `caracal-ffi`, which translates raw pointers into the types used here.
//!
//! It is `no_std` so that libcrypto does not pick up the Rust standard
//! library, its allocator, or its runtime. Unit tests run with `std`.

#![cfg_attr(not(test), no_std)]
#![deny(unsafe_code)]

/// Returns the sum of all bytes in `input`, wrapping on overflow.
///
/// Placeholder used to exercise the C-to-Rust call path.
pub fn byte_sum(input: &[u8]) -> u64 {
    input
        .iter()
        .fold(0u64, |acc, &b| acc.wrapping_add(u64::from(b)))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn byte_sum_empty() {
        assert_eq!(byte_sum(&[]), 0);
    }

    #[test]
    fn byte_sum_values() {
        assert_eq!(byte_sum(&[1, 2, 3]), 6);
        assert_eq!(byte_sum(&[0xff; 4]), 4 * 0xff);
    }
}

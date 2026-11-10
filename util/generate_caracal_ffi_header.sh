#!/usr/bin/env bash
# Copyright Amazon.com Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0 OR ISC
#
# Regenerates crypto/rust/caracal-ffi/include/caracal_ffi.h from the
# caracal-ffi crate using cbindgen. Commit the result.
#
# Usage: util/generate_caracal_ffi_header.sh [--check]
#
#   --check  Do not write anything; exit non-zero if the committed header
#            differs from what cbindgen would generate. Used by CI.
#
# Output depends on the cbindgen version, so it is pinned below. Install it
# with:
#
#   cargo install --locked cbindgen --version <CBINDGEN_VERSION>

set -euo pipefail

CBINDGEN_VERSION="0.29.4"

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
crate_dir="${repo_root}/crypto/rust/caracal-ffi"
header="${crate_dir}/include/caracal_ffi.h"
header_rel="${header#"${repo_root}"/}"

check=0
case "${1:-}" in
  "") ;;
  --check) check=1 ;;
  *)
    echo "Usage: $0 [--check]" >&2
    exit 2
    ;;
esac

install_hint="cargo install --locked cbindgen --version ${CBINDGEN_VERSION}"
if ! command -v cbindgen >/dev/null 2>&1; then
  echo "cbindgen not found. Install it with: ${install_hint}" >&2
  exit 1
fi
found_version="$(cbindgen --version | awk '{print $2}')"
if [[ "${found_version}" != "${CBINDGEN_VERSION}" ]]; then
  echo "cbindgen ${CBINDGEN_VERSION} is required, found ${found_version}." >&2
  echo "Install it with: ${install_hint} --force" >&2
  exit 1
fi

out="${header}"
if (( check )); then
  out="$(mktemp)"
  trap 'rm -f "${out}"' EXIT
fi

cbindgen --config "${crate_dir}/cbindgen.toml" --output "${out}" "${crate_dir}"

if (( check )); then
  if ! diff -u "${header}" "${out}"; then
    echo >&2
    echo "${header_rel} is out of date. Run util/generate_caracal_ffi_header.sh and commit the result." >&2
    exit 1
  fi
  echo "${header_rel} is up to date."
else
  echo "Wrote ${header_rel}."
fi

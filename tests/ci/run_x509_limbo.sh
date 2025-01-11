#!/usr/bin/env bash
# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0 OR ISC

set -euxo pipefail

source tests/ci/common_posix_setup.sh

# For now we will just verify that the patch applies and our reporting to builds and tests successfully
# Subsequent follow-up PRs will wire this up into a new CodeBuild project and handle producing and tracking
# the reports.

SCRATCH_DIR="${SYS_ROOT}/scratch"
X509_CI_DIR="${SRC_ROOT}/tests/ci/x509"
X509_LIMBO_SRC="${SCRATCH_DIR}/x509-limbo"
BASE_COMMIT_SRC="${SYS_ROOT}/base-src"
BASE_REF="${BASE_REF:-${CODEBUILD_WEBHOOK_BASE_REF:?}}"

function build_reporting_tool() {
    pushd "${X509_CI_DIR}/limbo-report"
    make
    mv ./limbo-report "${SCRATCH_DIR}/"
    popd # "${X509_CI_DIR}/limbo-report"
}

function clone_and_patch_x509_limbo() {
    git clone https://github.com/C2SP/x509-limbo.git "${X509_LIMBO_SRC}"
    pushd "${X509_LIMBO_SRC}"
    patch -p1 -i "${X509_CI_DIR}/x509-limbo.patch"
    popd # "${X509_LIMBO_SRC}"
}

function run_aws_lc_harness() {
    pushd "${X509_LIMBO_SRC}"
    AWS_LC_SRC_DIR="${1}" make test-aws-lc
    popd # "${X509_LIMBO_SRC}"
}

git worktree add "${BASE_COMMIT_SRC}" "${BASE_REF:?}"

mkdir -p "${SCRATCH_DIR}"
rm -rf "${SCRATCH_DIR:?}"/*
pushd "${SCRATCH_DIR}"

build_reporting_tool
clone_and_patch_x509_limbo

REPORTS_DIR="${SRC_ROOT}/x509-limbo-reports"

# Build run x509-limbo on current src of event
run_aws_lc_harness "${SRC_ROOT}"
"${SCRATCH_DIR}/limbo-report" annotate "${X509_LIMBO_SRC}/limbo.json" "${X509_LIMBO_SRC}/results/aws-lc.json" | tee "${REPORTS_DIR}/base.json"
"${SCRATCH_DIR}/limbo-report" annotate -csv "${X509_LIMBO_SRC}/limbo.json" "${X509_LIMBO_SRC}/results/aws-lc.json" | tee "${REPORTS_DIR}/base.csv"

# Build run x509-limbo on the base src for event
run_aws_lc_harness "${BASE_COMMIT_SRC}"
"${SCRATCH_DIR}/limbo-report" annotate "${X509_LIMBO_SRC}/limbo.json" "${X509_LIMBO_SRC}/results/aws-lc.json" | tee "${REPORTS_DIR}/changes.json"
"${SCRATCH_DIR}/limbo-report" annotate -csv "${X509_LIMBO_SRC}/limbo.json" "${X509_LIMBO_SRC}/results/aws-lc.json" | tee "${REPORTS_DIR}/changes.csv"

# Produce diff report
set +e
"${SCRATCH_DIR}/limbo-report" diff "${REPORTS_DIR}/base.json" "${REPORTS_DIR}/changes.json" | tee "${REPORTS_DIR}/summary.txt"
DIFF_RET_STATUS=${PIPESTATUS[0]}

set -e
popd # "${SCRATCH_DIR}"
rm -rf "${SCRATCH_DIR:?}"

if [ $DIFF_RET_STATUS -eq 0 ]; then
    exit 0
else
    exit 1
fi

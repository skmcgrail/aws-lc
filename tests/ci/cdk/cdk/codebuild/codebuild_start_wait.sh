#!/usr/bin/env bash

set -exo pipefail

# Set your project name
PROJECT_NAME="${PROJECT_NAME:?}"
AWS_REGION="${AWS_REGION:?}"

# Start the build and capture the build ID
BUILD_ID=$(aws --region "${AWS_REGION}" codebuild start-build-batch --project-name "${PROJECT_NAME}" --query 'build.id' --output text)

if [ -z "$BUILD_ID" ]; then
    echo "Failed to start build"
    exit 1
fi

echo "Build started with ID: $BUILD_ID"

# Function to get build status
get_build_status() {
    aws codebuild batch-get-builds --ids "${BUILD_ID}" --query 'builds[0].buildStatus' --output text
}

# Wait for the build to complete
while true; do
    STATUS=$(get_build_status)
    echo "Current status: $STATUS"
    
    if [ "$STATUS" = "SUCCEEDED" ]; then
        echo "Build completed successfully"
        exit 0
        elif [ "$STATUS" = "FAILED" ] || [ "$STATUS" = "STOPPED" ]; then
        echo "Build failed or was stopped"
        exit 1
        elif [ "$STATUS" = "FAULT" ] || [ "$STATUS" = "TIMED_OUT" ]; then
        echo "Build encountered a fault or timed out"
        exit 1
    fi
    
    # Wait for 30 seconds before checking again
    sleep 30
done

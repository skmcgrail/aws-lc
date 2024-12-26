#!/usr/bin/env python3

# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0 OR ISC

from aws_cdk import Environment, App
from aws_cdk.pipelines import CodePipeline
from cdk.pipeline import AwsLcCiPipeline
from util.metadata import AWS_ACCOUNT, AWS_REGION

# Initialize app.
app = App()

env = Environment(account=AWS_ACCOUNT, region=AWS_REGION)
AwsLcCiPipeline(app, "AwsLcCiPipeline", env=env)

app.synth()

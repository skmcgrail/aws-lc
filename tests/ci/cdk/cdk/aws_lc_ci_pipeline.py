from typing import Any, Dict, Mapping, Sequence
import aws_cdk as cdk
from aws_cdk import Environment, IPolicyValidationPluginBeta1, IStackSynthesizer, PermissionsBoundary
from cdk.aws_lc_analytics_stack import AwsLcGitHubAnalyticsStack
from cdk.aws_lc_android_ci_stack import AwsLcAndroidCIStack
from cdk.aws_lc_ec2_test_framework_ci_stack import AwsLcEC2TestingCIStack
from cdk.aws_lc_github_ci_stack import AwsLcGitHubCIStack
from cdk.aws_lc_github_fuzz_ci_stack import AwsLcGitHubFuzzCIStack
from cdk.ecr_stack import EcrStack
from cdk.linux_docker_image_batch_build_stack import LinuxDockerImageBatchBuildStack
from cdk.windows_docker_image_build_stack import WindowsDockerImageBuildStack
from constructs import Construct
from aws_cdk.pipelines import CodePipeline, CodePipelineSource, ShellStep
from util.metadata import GITHUB_REPO_OWNER, GITHUB_REPO_NAME, LINUX_AARCH_ECR_REPO, LINUX_X86_ECR_REPO, WINDOWS_X86_ECR_REPO

class AwsLcCiPipelineStack(cdk.Stack):
    def __init__(self, scope: Construct, id: str, **kwargs):
        super().__init__(scope, id, **kwargs)

        pipeline = CodePipeline(self, "Pipeline", synth=ShellStep(
            "synthesize",
            input=CodePipelineSource.git_hub("{}/{}".format(GITHUB_REPO_OWNER, GITHUB_REPO_NAME)),
            commands=[
                "npm install -g aws-cdk",
                "cd tests/ci",
                "python -m pip install -r requirements.txt"
            ],
        ))

        pipeline.add_stage(AwsLcCiPipelineStage(self, "AwsLcCiStage"))

class AwsLcCiPipelineStage(cdk.Stage):
    def __init__(self, scope: Construct, id: str, **kwargs):
        super().__init__(scope, id, **kwargs)

        # Define AWS ECR stacks.
        # ECR holds the docker images, which are pre-built to accelerate the code builds/tests of git pull requests.
        EcrStack(self, "aws-lc-ecr-linux-x86", LINUX_X86_ECR_REPO)
        EcrStack(self, "aws-lc-ecr-linux-aarch", LINUX_AARCH_ECR_REPO)
        EcrStack(self, "aws-lc-ecr-windows-x86", WINDOWS_X86_ECR_REPO)

        # Define CodeBuild Batch job for building Docker images.
        LinuxDockerImageBatchBuildStack(self, "aws-lc-docker-image-build-linux")

        # AWS CodeBuild cannot build Windows Docker images because DIND (Docker In Docker) is not supported on Windows.
        # Windows Docker images are created by running commands in Windows EC2 instance.
        WindowsDockerImageBuildStack(self, "aws-lc-docker-image-build-windows")

        # Define CodeBuild Batch job for testing code.
        x86_build_spec_file = "cdk/codebuild/github_ci_linux_x86_omnibus.yaml"
        AwsLcGitHubCIStack(self, "aws-lc-ci-linux-x86", x86_build_spec_file)
        arm_build_spec_file = "cdk/codebuild/github_ci_linux_arm_omnibus.yaml"
        AwsLcGitHubCIStack(self, "aws-lc-ci-linux-arm", arm_build_spec_file)
        integration_build_spec_file = "cdk/codebuild/github_ci_integration_omnibus.yaml"
        AwsLcGitHubCIStack(self, "aws-lc-ci-integration", integration_build_spec_file)
        win_x86_build_spec_file = "cdk/codebuild/github_ci_windows_x86_omnibus.yaml"
        AwsLcGitHubCIStack(self, "aws-lc-ci-windows-x86", win_x86_build_spec_file)
        fuzz_build_spec_file = "cdk/codebuild/github_ci_fuzzing_omnibus.yaml"
        AwsLcGitHubFuzzCIStack(self, "aws-lc-ci-fuzzing", fuzz_build_spec_file)
        analytics_build_spec_file = "cdk/codebuild/github_ci_analytics_omnibus.yaml"
        AwsLcGitHubAnalyticsStack(self, "aws-lc-ci-analytics", analytics_build_spec_file)
        # bm_framework_build_spec_file = "cdk/codebuild/bm_framework_omnibus.yaml"
        # BmFrameworkStack(app, "aws-lc-ci-bm-framework", bm_framework_build_spec_file, env=env)
        ec2_test_framework_build_spec_file = "cdk/codebuild/ec2_test_framework_omnibus.yaml"
        AwsLcEC2TestingCIStack(self, "aws-lc-ci-ec2-test-framework", ec2_test_framework_build_spec_file)
        android_build_spec_file = "cdk/codebuild/github_ci_android_omnibus.yaml"
        AwsLcAndroidCIStack(self, "aws-lc-ci-devicefarm-android", android_build_spec_file)

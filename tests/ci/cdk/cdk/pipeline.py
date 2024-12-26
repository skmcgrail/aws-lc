from aws_cdk import Stack, Stage, Environment
from aws_cdk import (
    pipelines,
    aws_codestarconnections as codestarconnections,
    aws_iam as iam,
)

from cdk.aws_lc_analytics_stack import AwsLcGitHubAnalyticsStack
from cdk.aws_lc_android_ci_stack import AwsLcAndroidCIStack
from cdk.aws_lc_ec2_test_framework_ci_stack import AwsLcEC2TestingCIStack
from cdk.aws_lc_github_ci_stack import AwsLcGitHubCIStack
from cdk.aws_lc_github_fuzz_ci_stack import AwsLcGitHubFuzzCIStack
from cdk.ecr_stack import EcrStack
from cdk.linux_docker_image_batch_build_stack import LinuxDockerImageBatchBuildStack
from cdk.windows_docker_image_build_stack import WindowsDockerImageBuildStack
from constructs import Construct
from util.metadata import (
    AWS_ACCOUNT,
    AWS_REGION,
    GITHUB_REPO_NAME,
    GITHUB_REPO_OWNER,
    GITHUB_SOURCE_VERSION,
    LINUX_AARCH_ECR_REPO,
    LINUX_X86_ECR_REPO,
    WINDOWS_X86_ECR_REPO,
)


class AwsLcCiPipeline(Stack):
    def __init__(
        self,
        scope: Construct,
        id: str,
        **kwargs,
    ) -> None:
        super().__init__(
            scope,
            id,
            **kwargs,
        )

        gh_connection = codestarconnections.CfnConnection(
            self,
            "GitHubConnection",
            connection_name="AwsLcCiPipelineGitHubConnection",
            provider_type="GitHub",
        )

        pipeline = pipelines.CodePipeline(
            self,
            "AwsLcCiPipeline",
            synth=pipelines.ShellStep(
                "Synth",
                input=pipelines.CodePipelineSource.connection(
                    f"{GITHUB_REPO_OWNER}/{GITHUB_REPO_NAME}",
                    "main",
                    connection_arn=gh_connection.attr_connection_arn,
                    trigger_on_push=True,
                ),
                commands=[
                    "npm install -g aws-cdk",
                    "cd tests/ci",
                    "python -m pip install -r requirements.txt",
                    "cd cdk",
                    "cdk synth",
                ],
                env={
                    "CDK_DEPLOY_ACCOUNT": AWS_ACCOUNT,
                    "CDK_DEPLOY_REGION": AWS_REGION,
                    "GITHUB_REPO_OWNER": GITHUB_REPO_OWNER,
                    "GITHUB_REPO_NAME": GITHUB_REPO_NAME,
                    "GITHUB_SOURCE_VERSION": GITHUB_SOURCE_VERSION,
                },
                primary_output_directory="tests/ci/cdk/cdk.out",
            ),
            self_mutation=True,
            code_build_defaults=pipelines.CodeBuildOptions(
                role_policy=[
                    iam.PolicyStatement(
                        effect=iam.Effect.ALLOW,
                        resources=["*"],
                        actions=["sts:AssumeRole"],
                        conditions={
                            "StringEquals": {
                                "iam:ResourceTag/aws-cdk:bootstrap-role": "lookup",
                            }
                        },
                    )
                ]
            ),
        )

        pipeline.add_stage(
            AwsLcCiAppStage(
                self,
                "DevStage",
                env=Environment(account=AWS_ACCOUNT, region=AWS_REGION),
            )
        )


class AwsLcCiAppStage(Stage):
    def __init__(
        self,
        scope,
        id,
        **kwargs,
    ):
        super().__init__(
            scope,
            id,
            **kwargs,
        )

        # Define AWS ECR stacks.
        # ECR holds the docker images, which are pre-built to accelerate the code builds/tests of git pull requests.
        EcrStack(
            self,
            "aws-lc-ecr-linux-x86",
            LINUX_X86_ECR_REPO,
            env=kwargs["env"],
            stack_name="aws-lc-ecr-linux-x86",
        )
        EcrStack(
            self,
            "aws-lc-ecr-linux-aarch",
            LINUX_AARCH_ECR_REPO,
            env=kwargs["env"],
            stack_name="aws-lc-ecr-linux-aarch",
        )
        EcrStack(
            self,
            "aws-lc-ecr-windows-x86",
            WINDOWS_X86_ECR_REPO,
            env=kwargs["env"],
            stack_name="aws-lc-ecr-windows-x86",
        )

        # Define CodeBuild Batch job for building Docker images.
        LinuxDockerImageBatchBuildStack(
            self,
            "aws-lc-docker-image-build-linux",
            env=kwargs["env"],
            stack_name="aws-lc-docker-image-build-linux",
        )

        # AWS CodeBuild cannot build Windows Docker images because DIND (Docker In Docker) is not supported on Windows.
        # Windows Docker images are created by running commands in Windows EC2 instance.
        WindowsDockerImageBuildStack(
            self,
            "aws-lc-docker-image-build-windows",
            env=kwargs["env"],
            stack_name="aws-lc-docker-image-build-windows",
        )

        # Define CodeBuild Batch job for testing code.
        x86_build_spec_file = "cdk/codebuild/github_ci_linux_x86_omnibus.yaml"
        AwsLcGitHubCIStack(
            self,
            "aws-lc-ci-linux-x86",
            x86_build_spec_file,
            env=kwargs["env"],
            stack_name="aws-lc-ci-linux-x86",
        )
        arm_build_spec_file = "cdk/codebuild/github_ci_linux_arm_omnibus.yaml"
        AwsLcGitHubCIStack(
            self,
            "aws-lc-ci-linux-arm",
            arm_build_spec_file,
            env=kwargs["env"],
            stack_name="aws-lc-ci-linux-arm",
        )
        integration_build_spec_file = "cdk/codebuild/github_ci_integration_omnibus.yaml"
        AwsLcGitHubCIStack(
            self,
            "aws-lc-ci-integration",
            integration_build_spec_file,
            env=kwargs["env"],
            stack_name="aws-lc-ci-integration",
        )
        win_x86_build_spec_file = "cdk/codebuild/github_ci_windows_x86_omnibus.yaml"
        AwsLcGitHubCIStack(
            self,
            "aws-lc-ci-windows-x86",
            win_x86_build_spec_file,
            env=kwargs["env"],
            stack_name="aws-lc-ci-windows-x86",
        )
        fuzz_build_spec_file = "cdk/codebuild/github_ci_fuzzing_omnibus.yaml"
        AwsLcGitHubFuzzCIStack(
            self,
            "aws-lc-ci-fuzzing",
            fuzz_build_spec_file,
            env=kwargs["env"],
            stack_name="aws-lc-ci-fuzzing",
        )
        analytics_build_spec_file = "cdk/codebuild/github_ci_analytics_omnibus.yaml"
        AwsLcGitHubAnalyticsStack(
            self,
            "aws-lc-ci-analytics",
            analytics_build_spec_file,
            env=kwargs["env"],
            stack_name="aws-lc-ci-analytics",
        )
        # bm_framework_build_spec_file = "cdk/codebuild/bm_framework_omnibus.yaml"
        # BmFrameworkStack(app, "aws-lc-ci-bm-framework", bm_framework_build_spec_file, env=env)
        # ec2_test_framework_build_spec_file = (
        #     "cdk/codebuild/ec2_test_framework_omnibus.yaml"
        # )
        # AwsLcEC2TestingCIStack(
        #     self,
        #     "aws-lc-ci-ec2-test-framework",
        #     ec2_test_framework_build_spec_file,
        #     env=kwargs["env"],
        #     stack_name="aws-lc-ci-ec2-test-framework",
        # )
        android_build_spec_file = "cdk/codebuild/github_ci_android_omnibus.yaml"
        AwsLcAndroidCIStack(
            self,
            "aws-lc-ci-devicefarm-android",
            android_build_spec_file,
            env=kwargs["env"],
            stack_name="aws-lc-ci-devicefarm-android",
        )

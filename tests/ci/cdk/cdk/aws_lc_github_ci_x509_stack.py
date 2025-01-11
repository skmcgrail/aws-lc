from aws_cdk import Stack, aws_codebuild as codebuild, aws_s3 as s3
from constructs import Construct
from util.build_spec_loader import BuildSpecLoader
from util.metadata import (
    GITHUB_PUSH_CI_BRANCH_TARGETS,
    GITHUB_REPO_NAME,
    GITHUB_REPO_OWNER,
)


class AwsLcGitHubX509CIStack(Stack):
    def __init__(
        self,
        scope: Construct,
        id: str,
        **kwargs,
    ) -> None:
        super().__init__(scope, id, **kwargs)

        git_hub_source = codebuild.Source.git_hub(
            owner=GITHUB_REPO_OWNER,
            repo=GITHUB_REPO_NAME,
            webhook=True,
            webhook_filters=[
                codebuild.FilterGroup.in_event_of(
                    codebuild.EventAction.PULL_REQUEST_CREATED,
                    codebuild.EventAction.PULL_REQUEST_UPDATED,
                    codebuild.EventAction.PULL_REQUEST_REOPENED,
                ),
                codebuild.FilterGroup.in_event_of(
                    codebuild.EventAction.PUSH
                ).and_branch_is(GITHUB_PUSH_CI_BRANCH_TARGETS),
            ],
            webhook_triggers_batch_build=True,
        )

        self.reports_bucket = s3.Bucket(
            self,
            "aws-lc-x509-reports",
            block_public_access=s3.BlockPublicAccess.BLOCK_ALL,
        )

        self.codebuild_project = codebuild.Project(
            self,
            id,
            project_name=id,
            source=git_hub_source,
            build_spec=BuildSpecLoader.load(
                "cdk/codebuild/github_ci_x509_omnibus.yaml"
            ),
            artifacts=codebuild.Artifacts.s3(
                bucket=self.reports_bucket,
                package_zip=False,
                include_build_id=False,
            ),
        )

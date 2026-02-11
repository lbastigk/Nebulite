#!/bin/bash

# Creates a list of all git commit messages since the last release tag 'vX.Y.Z' and saves it to a file named 'CHANGELOG.md'.

# Make sure we are in the root directory of the git repository
cd "$(git rev-parse --show-toplevel)" || { echo "Error: Not a git repository"; exit 1; }

# Make sure we are on the main branch
# If not, exit with an error message
CURRENT_BRANCH=$(git branch --show-current)
if [ "$CURRENT_BRANCH" != "main" ]; then
    echo "Error: You must be on the main branch to run this script. Current branch: $CURRENT_BRANCH"
    exit 1
fi

# Get the latest release tag
LATEST_TAG=$(git describe --tags --abbrev=0)
if [ -z "$LATEST_TAG" ]; then
    echo "Error: No tags found in the repository. Please create a tag for the latest release before running this script."
    exit 1
fi

# Get the commit messages since the latest tag
COMMIT_MESSAGES=$(git log "$LATEST_TAG"..HEAD --pretty=format:"- %s")

# Save the commit messages to CHANGELOG.md
echo "## Changelog since $LATEST_TAG" > CHANGELOG.md
echo "" >> CHANGELOG.md
echo "$COMMIT_MESSAGES" >> CHANGELOG.md
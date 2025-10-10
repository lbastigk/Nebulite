#!/bin/bash

# This script is used to test the deployment of the application:
# - clones the repository
# - switches to the branch currently in use
# - runs the installation script

# The installation script:
# - initializes the repository
# - builds the application
# - runs tests to verify the binaries

# Check if we are in the root directory of the repository
if [ ! -f "Scripts/Validation/deployment_test.sh" ]; then
  echo "Please run this script from the root directory of the repository."
  exit 1
fi

# Get the current branch name
CURRENT_BRANCH=$(git rev-parse --abbrev-ref HEAD)
echo "Current branch: $CURRENT_BRANCH"

# Create a temporary directory for cloning the repository
TEMP_DIR=$(mktemp -d)
echo "Cloning repository into temporary directory: $TEMP_DIR"

# Clone the repository and switch to the current branch
cd $TEMP_DIR || exit 1
git clone https://github.com/lbastigk/Nebulite.git
cd Nebulite || exit 1
git checkout $CURRENT_BRANCH

# Install the application and run tests
echo "Running installation script..."
chmod +x ./install.sh
./install.sh

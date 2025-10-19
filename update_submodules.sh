#!/bin/bash

# Script to update all git submodules in the Flux UI Framework
# This script will pull the latest changes from the remote repositories

set -e  # Exit on any error

echo "🔄 Updating git submodules for Flux UI Framework..."
echo

# Check if we're in a git repository
if ! git rev-parse --git-dir > /dev/null 2>&1; then
    echo "❌ Error: Not in a git repository"
    exit 1
fi

# Check if .gitmodules exists
if [ ! -f .gitmodules ]; then
    echo "❌ Error: .gitmodules file not found"
    exit 1
fi

echo "📋 Current submodules:"
git submodule status
echo

# Initialize and update all submodules (non-recursive to avoid nested submodule issues)
echo "🔧 Initializing and updating all submodules..."
git submodule update --init

echo
echo "📥 Pulling latest changes for all submodules..."

# Update each submodule to the latest commit on their default branch
git submodule foreach 'git pull origin $(git branch --show-current)'

echo
echo "✅ Submodule update complete!"
echo
echo "📋 Updated submodules:"
git submodule status

echo
echo "💡 To commit these changes, run:"
echo "   git add .gitmodules third_party/"
echo "   git commit -m 'Update submodules to latest versions'"

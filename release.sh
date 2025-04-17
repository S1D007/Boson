#!/bin/bash

read -rp "Enter the tag you want to recreate: " tag

if [[ -z "$tag" ]]; then
  echo "Error: Tag name cannot be empty."
  exit 1
fi

if git rev-parse "$tag" >/dev/null 2>&1; then
  echo "Deleting local tag '$tag'..."
  if git tag -d "$tag"; then
    echo "Local tag '$tag' deleted."
  else
    echo "Failed to delete local tag '$tag'."
    exit 1
  fi
else
  echo "Tag '$tag' does not exist locally. Skipping local deletion."
fi

if git ls-remote --tags origin | grep -q "refs/tags/$tag$"; then
  echo "Deleting remote tag '$tag'..."
  if git push origin ":refs/tags/$tag"; then
    echo "Remote tag '$tag' deleted."
  else
    echo "Failed to delete remote tag '$tag'."
    exit 1
  fi
else
  echo "Tag '$tag' does not exist on remote. Skipping remote deletion."
fi

echo "Creating new tag '$tag'..."
if git tag "$tag"; then
  echo "Tag '$tag' created successfully."
else
  echo "Failed to create tag '$tag'."
  exit 1
fi

echo "Pushing tag '$tag' to remote..."
if git push origin "$tag"; then
  echo "Tag '$tag' pushed to remote successfully."
else
  echo "Failed to push tag '$tag' to remote."
  exit 1
fi

echo "Done."

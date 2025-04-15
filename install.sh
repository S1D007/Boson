#!/bin/bash
set -e

REPO="S1D007/boson"
CLI_NAME="boson"
INSTALL_DIR="$HOME/.local/bin"

OS="$(uname -s | tr '[:upper:]' '[:lower:]')"
if [[ "$OS" == "darwin" ]]; then
    ASSET="boson-macos.tar.gz"
elif [[ "$OS" == "linux" ]]; then
    ASSET="boson-linux.tar.gz"
else
    echo "Unsupported OS: $OS"
    exit 1
fi

if ! command -v jq >/dev/null 2>&1; then
  echo "jq is required but not installed. Installing jq..."
  if [[ "$OS" == "darwin" ]]; then
    brew install jq
  elif [[ "$OS" == "linux" ]]; then
    sudo apt-get update && sudo apt-get install -y jq
  fi
fi

TAG=$(curl -s "https://api.github.com/repos/$REPO/releases/latest" | jq -r .tag_name)

mkdir -p "$INSTALL_DIR"
TMPDIR=$(mktemp -d)
curl -L "https://github.com/$REPO/releases/download/$TAG/$ASSET" -o "$TMPDIR/$ASSET"
tar -xzf "$TMPDIR/$ASSET" -C "$TMPDIR"
mv "$TMPDIR/boson" "$INSTALL_DIR/$CLI_NAME"
chmod +x "$INSTALL_DIR/$CLI_NAME"
rm -rf "$TMPDIR"

if [[ ":$PATH:" != *":$INSTALL_DIR:"* ]]; then
    echo "export PATH=\"\$PATH:$INSTALL_DIR\"" >> "$HOME/.bashrc"
    echo "export PATH=\"\$PATH:$INSTALL_DIR\"" >> "$HOME/.zshrc"
    export PATH="$PATH:$INSTALL_DIR"
fi

echo "Boson CLI installed successfully!"
echo "Run 'boson --help' to get started."

UPDATE_SNIPPET="\n# Boson CLI auto-update\nif command -v boson >/dev/null 2>&1; then\n  boson update >/dev/null 2>&1\nfi\n"
if ! grep -q 'Boson CLI auto-update' "$HOME/.bashrc"; then
    echo "$UPDATE_SNIPPET" >> "$HOME/.bashrc"
fi
if ! grep -q 'Boson CLI auto-update' "$HOME/.zshrc"; then
    echo "$UPDATE_SNIPPET" >> "$HOME/.zshrc"
fi
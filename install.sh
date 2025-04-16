#!/bin/bash
set -e

REPO="S1D007/boson"
CLI_NAME="boson"
INSTALL_DIR="$HOME/.local/bin"

OS="$(uname -s | tr '[:upper:]' '[:lower:]')"
case "$OS" in
  darwin) ASSET="boson-macos.tar.gz" ;;
  linux) ASSET="boson-linux.tar.gz" ;;
  *) echo "Unsupported OS: $OS" && exit 1 ;;
esac

if ! command -v jq >/dev/null 2>&1; then
  if [[ "$OS" == "darwin" ]]; then
    brew install jq
  else
    sudo apt-get update && sudo apt-get install -y jq
  fi
fi

TAG=$(curl -s "https://api.github.com/repos/$REPO/releases/latest" | jq -r .tag_name)
mkdir -p "$INSTALL_DIR"

if [ ! -w "$INSTALL_DIR" ]; then
  sudo mkdir -p "$INSTALL_DIR"
fi

TMPDIR=$(mktemp -d)
curl -L "https://github.com/$REPO/releases/download/$TAG/$ASSET" -o "$TMPDIR/$ASSET"
tar -xzf "$TMPDIR/$ASSET" -C "$TMPDIR"

if [ -w "$INSTALL_DIR" ]; then
  mv "$TMPDIR/$CLI_NAME" "$INSTALL_DIR/$CLI_NAME"
  chmod +x "$INSTALL_DIR/$CLI_NAME"
  if [ -d "$TMPDIR/templates" ]; then
    cp -r "$TMPDIR/templates" "$INSTALL_DIR/templates"
  fi
else
  sudo mv "$TMPDIR/$CLI_NAME" "$INSTALL_DIR/$CLI_NAME"
  sudo chmod +x "$INSTALL_DIR/$CLI_NAME"
  if [ -d "$TMPDIR/templates" ]; then
    sudo cp -r "$TMPDIR/templates" "$INSTALL_DIR/templates"
  fi
fi

rm -rf "$TMPDIR"

echo "Boson CLI installed successfully!"
echo "Run 'boson --help' to get started."

if [[ ":$PATH:" != *":$INSTALL_DIR:"* ]]; then
  echo ""
  echo "⚠️  The CLI install directory ($INSTALL_DIR) is not in your PATH."
  echo "   To fix this, add the following line to your shell config (e.g., ~/.bashrc or ~/.zshrc):"
  echo "     export PATH=\"\$PATH:$INSTALL_DIR\""
  echo ""
fi

echo ""
echo "📦 Optional: To enable auto-update, add the following snippet to your shell config:"
echo ""
echo "  # Boson CLI auto-update"
echo "  if command -v boson >/dev/null 2>&1; then"
echo "    boson update >/dev/null 2>&1"
echo "  fi"
echo ""

#!/bin/bash
set -e

REPO="S1D007/boson"
CLI_NAME="boson"
INSTALL_DIR="$HOME/.local/bin"

OS="$(uname -s | tr '[:upper:]' '[:lower:]')"
ARCH="$(uname -m)"

# Normalize architecture names
case "$ARCH" in
  x86_64) ARCH_NORMALIZED="amd64" ;;
  i*86) ARCH_NORMALIZED="386" ;;
  aarch64|arm64) ARCH_NORMALIZED="arm64" ;;
  armv7*|armv8*) ARCH_NORMALIZED="arm" ;;
  *) ARCH_NORMALIZED="$ARCH" ;;
esac

case "$OS" in
  darwin) 
    if [ "$ARCH_NORMALIZED" = "arm64" ]; then
      ASSET="boson-macos-arm64.tar.gz"
    else
      ASSET="boson-macos-amd64.tar.gz"
    fi
    ;;
  linux) 
    ASSET="boson-linux-${ARCH_NORMALIZED}.tar.gz"
    ;;
  *) echo "Unsupported OS: $OS" && exit 1 ;;
esac

echo "Detected OS: $OS, Architecture: $ARCH (normalized to $ARCH_NORMALIZED)"
echo "Using asset: $ASSET"

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
echo "Downloading from: https://github.com/$REPO/releases/download/$TAG/$ASSET"
curl -L "https://github.com/$REPO/releases/download/$TAG/$ASSET" -o "$TMPDIR/$ASSET" || {
  echo "Failed to download $ASSET"
  echo "Trying fallback to boson-$OS.tar.gz..."
  FALLBACK_ASSET="boson-$OS.tar.gz"
  curl -L "https://github.com/$REPO/releases/download/$TAG/$FALLBACK_ASSET" -o "$TMPDIR/$FALLBACK_ASSET" || {
    echo "Error: Failed to download Boson CLI for your platform"
    echo "Please check if your architecture ($ARCH / $ARCH_NORMALIZED) is supported"
    echo "You can build from source instead: https://bosonframework.vercel.app/docs/getting-started/installation"
    exit 1
  }
  ASSET=$FALLBACK_ASSET
}

tar -xzf "$TMPDIR/$ASSET" -C "$TMPDIR" || {
  echo "Error extracting archive. The downloaded file may be corrupted or in an unexpected format."
  exit 1
}

if [ -w "$INSTALL_DIR" ]; then
  mv "$TMPDIR/$CLI_NAME" "$INSTALL_DIR/$CLI_NAME"
  chmod +x "$INSTALL_DIR/$CLI_NAME"
  if [ -d "$TMPDIR/templates" ]; then
    rm -rf "$INSTALL_DIR/templates"
    cp -a "$TMPDIR/templates" "$INSTALL_DIR/"
    echo "Templates installed successfully to $INSTALL_DIR/templates"
  else
    echo "Warning: Templates directory not found in the package"
  fi
else
  sudo mv "$TMPDIR/$CLI_NAME" "$INSTALL_DIR/$CLI_NAME"
  sudo chmod +x "$INSTALL_DIR/$CLI_NAME"
  if [ -d "$TMPDIR/templates" ]; then
    sudo rm -rf "$INSTALL_DIR/templates"
    sudo cp -a "$TMPDIR/templates" "$INSTALL_DIR/"
    echo "Templates installed successfully to $INSTALL_DIR/templates"
  else
    echo "Warning: Templates directory not found in the package"
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
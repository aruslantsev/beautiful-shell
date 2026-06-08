#!/usr/bin/env bash
set -e

REPO="aruslantsev/beautiful_shell"
INSTALL_DIR="${HOME}/.beautiful_shell"
GITHUB_API="https://api.github.com/repos/${REPO}/releases/latest"

echo "=== Installing Beautiful Shell ==="

# OS and arch detection
OS="$(uname -s | tr '[:upper:]' '[:lower:]')"
ARCH="$(uname -m)"

case "${ARCH}" in
    x86_64|amd64) ARCH_SUFFIX="amd64" ;;
    arm64|aarch64) ARCH_SUFFIX="arm64" ;;
    *) echo "Unsupported architecture: ${ARCH}"; exit 1 ;;
esac

case "${OS}" in
    linux) OS_NAME="linux" ;;
    darwin) OS_NAME="macos" ;;
    freebsd) OS_NAME="freebsd" ;;
    *) echo "Unsupported OS: ${OS}"; exit 1 ;;
esac

BINARY_NAME="beautiful_shell-${OS_NAME}-${ARCH_SUFFIX}"

# 2. Directories
mkdir -p "${INSTALL_DIR}/bin"
mkdir -p "${INSTALL_DIR}/scripts"

# 3. Download binary
echo "Fetching latest release assets from GitHub..."
DOWNLOAD_URL=$(curl -s "${GITHUB_API}" | grep "browser_download_url" | grep "${BINARY_NAME}" | cut -d '"' -f 4)

if [ -z "$DOWNLOAD_URL" ]; then
    echo "Error: Could not find precompiled binary for ${OS_NAME} (${ARCH_SUFFIX}) in the latest release."
    exit 1
fi

echo "Downloading binary from ${DOWNLOAD_URL}..."
curl -L "${DOWNLOAD_URL}" -o "${INSTALL_DIR}/bin/beautiful_shell"
chmod +x "${INSTALL_DIR}/bin/beautiful_shell"

# 4. КCopy scripts
echo "Copying scripts and configs..."
# Check if exists local copy

if [ -d "./src/scripts" ] && [ -f "./src/shellrc" ]; then
    cp -r ./src/scripts/* "${INSTALL_DIR}/scripts/"
    cp ./src/shellrc "${INSTALL_DIR}/shellrc"
else
    # Pull from master using `curl | sh`
    echo "Downloading scripts from repository..."
    SCRIPTS_URL="https://raw.githubusercontent.com/${REPO}/master/src"
    cp -r src/scripts/* "${INSTALL_DIR}/scripts/" 2>/dev/null || {
        # Download scripts
        for script in messages colors 20_history 02_aliases 30_devtools initialize 00_path 03_environment 10_ssh-agent; do
            curl -fsL "${SCRIPTS_URL}/scripts/${script}" -o "${INSTALL_DIR}/scripts/${script}"
        done
        curl -fsL "${SCRIPTS_URL}/shellrc" -o "${INSTALL_DIR}/shellrc"
    }
fi

echo "=== Installation Successful! ==="
echo ""
echo "To activate beautiful_shell, add the following line to your ~/.bashrc or ~/.zshrc:"
echo "--------------------------------------------------------"
echo "  . \${HOME}/.beautiful_shell/shellrc"
echo "--------------------------------------------------------"
echo "Then restart your terminal."
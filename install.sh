#!/bin/sh
# MarkIt Installer — https://github.com/GhostVaibhav/MarkIt
# Usage: curl -fsSL https://raw.githubusercontent.com/GhostVaibhav/MarkIt/master/install.sh | sh
set -e

REPO="GhostVaibhav/MarkIt"
BINARY_NAME="markit"
INSTALL_DIR="/usr/local/bin"

# --- Colors ---
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m'

info()  { printf "${CYAN}▸ ${NC} %s\n" "$1"; }
ok()    { printf "${GREEN}✔ ${NC} %s\n" "$1"; }
warn()  { printf "${YELLOW}⚠ ${NC} %s\n" "$1"; }
fail()  { printf "${RED}✖  %s${NC}\n" "$1" >&2; exit 1; }

# --- Detect architecture ---
detect_arch() {
    arch=$(uname -m)
    case "$arch" in
        x86_64|amd64)       echo "x86_64"   ;;
        aarch64|arm64)      echo "aarch64"   ;;
        armv7*|armhf)       echo "armv7"     ;;
        i686|i386|i586)     echo "i686"      ;;
        riscv64)            echo "riscv64"   ;;
        ppc64le|ppc64el)    echo "ppc64le"   ;;
        s390x)              echo "s390x"     ;;
        *)                  fail "Unsupported architecture: $arch" ;;
    esac
}

# --- Download file ---
download() {
    url="$1"
    output="$2"
    if command -v curl >/dev/null 2>&1; then
        curl -fL -# -o "$output" "$url"
    elif command -v wget >/dev/null 2>&1; then
        wget --show-progress -qO "$output" "$url"
    else
        fail "Neither curl nor wget found. Please install one and retry."
    fi
}

# --- Main ---
main() {
    printf "\n${BOLD}  MarkIt Installer${NC}\n"
    printf "  ─────────────────\n\n"

    # Detect arch
    ARCH=$(detect_arch)
    info "Detected architecture: ${ARCH}"

    # Build download URL
    ASSET="MarkIt-linux-${ARCH}"
    URL="https://github.com/${REPO}/releases/latest/download/${ASSET}.tar.gz"
    info "Downloading from: ${URL}"

    # Download archive to temp
    TMPDIR=$(mktemp -d)
    TARFILE="${TMPDIR}/${ASSET}.tar.gz"
    trap 'rm -rf "$TMPDIR"' EXIT
    download "$URL" "$TARFILE" || fail "Download failed. Check your internet connection or if the release exists for ${ARCH}."
    ok "Downloaded successfully"

    # Extract binary from archive
    info "Extracting..."
    tar xzf "$TARFILE" -C "$TMPDIR"

    # Find the binary (it may be at the top level or nested)
    EXTRACTED="${TMPDIR}/${ASSET}"
    [ -f "$EXTRACTED" ] || EXTRACTED=$(find "$TMPDIR" -name "${ASSET}" -type f | head -1)
    [ -f "$EXTRACTED" ] || EXTRACTED=$(find "$TMPDIR" -name "MarkIt" -type f | head -1)
    [ -f "$EXTRACTED" ] || fail "Could not find MarkIt binary inside the downloaded archive."
    ok "Extracted binary"

    # Make executable
    chmod +x "$EXTRACTED"

    # Install — try /usr/local/bin first, fall back to ~/.local/bin
    if [ -w "$INSTALL_DIR" ]; then
        mv "$EXTRACTED" "${INSTALL_DIR}/${BINARY_NAME}"
        ok "Installed to ${INSTALL_DIR}/${BINARY_NAME}"
    elif command -v sudo >/dev/null 2>&1; then
        info "Requesting sudo to install to ${INSTALL_DIR}..."
        sudo mv "$EXTRACTED" "${INSTALL_DIR}/${BINARY_NAME}"
        sudo chmod +x "${INSTALL_DIR}/${BINARY_NAME}"
        ok "Installed to ${INSTALL_DIR}/${BINARY_NAME}"
    else
        # Fallback to ~/.local/bin
        INSTALL_DIR="${HOME}/.local/bin"
        mkdir -p "$INSTALL_DIR"
        mv "$EXTRACTED" "${INSTALL_DIR}/${BINARY_NAME}"
        ok "Installed to ${INSTALL_DIR}/${BINARY_NAME}"

        # Check if ~/.local/bin is in PATH
        case ":$PATH:" in
            *":${INSTALL_DIR}:"*) ;;
            *)
                warn "${INSTALL_DIR} is not in your PATH."
                warn "Add it by running: export PATH=\"${INSTALL_DIR}:\$PATH\""
                warn "Or add that line to your ~/.bashrc or ~/.profile"
                ;;
        esac
    fi

    # Verify
    printf "\n"
    if command -v "$BINARY_NAME" >/dev/null 2>&1; then
        ok "MarkIt is ready! Run '${BINARY_NAME}' to get started."
    else
        ok "Installed! You may need to restart your shell or add the install directory to your PATH."
    fi
    printf "\n"
}

main

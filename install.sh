#!/bin/sh
# Download a beautiful_shell release, unpack it into ~/.beautiful_shell,
# and append the shellrc hook to bash/zsh RC files.
set -eu

REPO="aruslantsev/beautiful-shell"
INSTALL_DIR="${HOME}/.beautiful_shell"
BIN_PATH="${INSTALL_DIR}/bin/beautiful_shell"
VERSION="${VERSION-}"
NO_HOOK=0

HOOK_BLOCK='
if [ -f ~/.beautiful_shell/shellrc ]; then
    . ~/.beautiful_shell/shellrc
fi
'

info() {
	printf '[INFO] %s\n' "$1"
}

err() {
	printf '[ERROR] %s\n' "$1" >&2
}

die() {
	err "$1"
	exit 1
}

usage() {
	cat <<'EOF'
Usage: install.sh [options]

Download the matching GitHub release tarball, verify its checksum, unpack it
to ~/.beautiful_shell, and append the shellrc hook to ~/.bashrc and/or ~/.zshrc.

Options:
  --version TAG   Install a specific release (e.g. v1.0.0)
  --no-hook       Do not modify ~/.bashrc or ~/.zshrc
  --help          Show this help

Environment:
  VERSION         Same as --version
EOF
}

while [ $# -gt 0 ]; do
	case "$1" in
		--version)
			[ $# -ge 2 ] || die "--version requires a value"
			VERSION=$2
			shift 2
			;;
		--version=*)
			VERSION=${1#--version=}
			shift
			;;
		--no-hook)
			NO_HOOK=1
			shift
			;;
		--help|-h)
			usage
			exit 0
			;;
		*)
			die "Unknown option: $1"
			;;
	esac
done

detect_os() {
	os=$(uname -s | tr '[:upper:]' '[:lower:]')
	case "${os}" in
		linux|darwin|freebsd) ;;
		*) die "Unsupported OS: ${os}" ;;
	esac
}

detect_arch() {
	arch=$(uname -m)
	case "${arch}" in
		x86_64|amd64) arch=amd64 ;;
		aarch64|arm64) arch=aarch64 ;;
		*) die "Unsupported architecture: ${arch}" ;;
	esac
}

download() {
	url=$1
	dest=$2
	if command -v curl >/dev/null 2>&1; then
		curl -fsSL -o "${dest}" "${url}" || die "Failed to download ${url}"
	elif command -v wget >/dev/null 2>&1; then
		wget -q -O "${dest}" "${url}" || die "Failed to download ${url}"
	else
		die "Need curl or wget to download release artifacts"
	fi
}

file_sha256() {
	file=$1
	if command -v sha256sum >/dev/null 2>&1; then
		sha256sum "${file}" | awk '{print $1}'
	elif command -v shasum >/dev/null 2>&1; then
		shasum -a 256 "${file}" | awk '{print $1}'
	elif command -v sha256 >/dev/null 2>&1; then
		sha256 -q "${file}"
	else
		die "Need sha256sum, shasum, or sha256 to verify the download"
	fi
}

expected_sha256() {
	sums=$1
	asset=$2
	awk -v f="${asset}" '
		$2 == f || $2 == ("*" f) { print $1; exit }
	' "${sums}"
}

has_hook() {
	[ -f "$1" ] && grep -q 'beautiful_shell/shellrc' "$1"
}

append_hook() {
	rcfile=$1
	if has_hook "${rcfile}"; then
		info "Hook already present in ${rcfile}"
		return 0
	fi
	printf '%s\n' "${HOOK_BLOCK}" >> "${rcfile}"
	info "Appended shellrc hook to ${rcfile}"
}

detect_os
detect_arch
asset="beautiful_shell-${os}-${arch}.tar.gz"

if [ -n "${VERSION}" ]; then
	base_url="https://github.com/${REPO}/releases/download/${VERSION}"
	info "Installing ${VERSION} for ${os}-${arch}"
else
	base_url="https://github.com/${REPO}/releases/latest/download"
	info "Installing latest release for ${os}-${arch}"
fi

workdir=$(mktemp -d 2>/dev/null || mktemp -d -t bs-install)
trap 'rm -rf "${workdir}"' EXIT INT HUP

info "Downloading ${asset}"
download "${base_url}/${asset}" "${workdir}/${asset}"

info "Downloading SHA256SUMS"
download "${base_url}/SHA256SUMS" "${workdir}/SHA256SUMS"

expected=$(expected_sha256 "${workdir}/SHA256SUMS" "${asset}")
[ -n "${expected}" ] || die "No SHA256SUMS entry for ${asset}"
actual=$(file_sha256 "${workdir}/${asset}")
if [ "${actual}" != "${expected}" ]; then
	die "Checksum mismatch for ${asset} (expected ${expected}, got ${actual})"
fi
info "Checksum verified"

info "Unpacking into ${HOME}"
tar -xzf "${workdir}/${asset}" -C "${HOME}"

if [ ! -f "${BIN_PATH}" ]; then
	die "Expected binary missing at ${BIN_PATH}"
fi
chmod +x "${BIN_PATH}"
info "Installed ${BIN_PATH}"

if [ "${NO_HOOK}" -eq 0 ]; then
	current_shell=$(basename "${SHELL-}")
	if [ -f "${HOME}/.bashrc" ]; then
		append_hook "${HOME}/.bashrc"
	elif [ "${current_shell}" = "bash" ]; then
		append_hook "${HOME}/.bashrc"
	fi
	if [ -f "${HOME}/.zshrc" ]; then
		append_hook "${HOME}/.zshrc"
	elif [ "${current_shell}" = "zsh" ]; then
		append_hook "${HOME}/.zshrc"
	fi
else
	info "Skipped shell RC hook (--no-hook)"
fi

info "Installed to ${INSTALL_DIR}"
info "Open a new terminal, or run: . ${INSTALL_DIR}/shellrc"

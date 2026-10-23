#!/bin/sh
set -e

out="${1:?usage: pack-ecosystem.sh <tarball.tar.gz>}"
case "${out}" in
	/*) dest="${out}" ;;
	*) dest="${PWD}/${out}" ;;
esac

rm -rf dist
mkdir -p dist/.beautiful_shell/bin dist/.beautiful_shell/scripts
cp build/beautiful_shell dist/.beautiful_shell/bin/
cp src/shellrc dist/.beautiful_shell/
cp src/scripts/* dist/.beautiful_shell/scripts/
(cd dist && tar -czf "${dest}" .beautiful_shell)

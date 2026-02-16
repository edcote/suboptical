#!/bin/bash -e
# shellcheck source=/dev/null

. "${GIT_ROOT:-$(git rev-parse --show-toplevel)}/env_utils.sh"

activate_conda

TMPD=$(mktemp -d)
trap 'rm -rf ${TMPD}' EXIT

set -x
mkdir -p "${TMPD}"
cd "${TMPD}"
git clone https://github.com/andrewwutw/build-djgpp.git
export DJGPP_PREFIX="${HOME}/.local"
cd build-djgpp
./build-djgpp.sh 12.2.0

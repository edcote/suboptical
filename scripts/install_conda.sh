#!/bin/bash -e
# shellcheck source=/dev/null

. "${GIT_ROOT:-$(git rev-parse --show-toplevel)}/env_utils.sh"

TMPD=$(mktemp -d)
trap 'rm -rf ${TMPD}' EXIT

[[ -f ${GIT_ROOT}/miniconda/bin/activate ]] && exit

set -x
mkdir -p "${TMPD}"
cd "${TMPD}"
wget https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh -O miniconda.sh
bash miniconda.sh -b

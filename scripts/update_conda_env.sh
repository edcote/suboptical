#!/bin/bash -e
# shellcheck source=/dev/null

. "${GIT_ROOT:-$(git rev-parse --show-toplevel)}/env_utils.sh"

executable_in_path conda || die "conda not in \$PATH?"

cd "${GIT_ROOT}"
conda env update -q -f environment.yml --prune

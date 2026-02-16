#!/bin/bash -e
# shellcheck source=/dev/null

. "${GIT_ROOT:-$(git rev-parse --show-toplevel)}/env_utils.sh"

activate_conda

cd "${GIT_ROOT}"
pip-sync -q requirements.txt

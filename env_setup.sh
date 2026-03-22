#!/bin/bash -e
# shellcheck source=/dev/null

. "${GIT_ROOT:-$(git rev-parse --show-toplevel)}/env_utils.sh"

if [[ -f ${GIT_ROOT}/.done_env_setup ]]; then
  die "Envsetup already done. If needed, remove file .done_env_setup and retry setup?"
fi

"${GIT_ROOT}"/scripts/update_conda_env.sh || die
"${GIT_ROOT}"/scripts/update_python_venv.sh || die

if [[ ! -f ${GIT_ROOT}/.secrets.baseline ]]; then
  detect-secrets scan > "${GIT_ROOT}/.secrets.baseline"
fi
pre-commit install || true

touch "${GIT_ROOT}/.done_env_setup"

#!/bin/bash -e
# shellcheck source=/dev/null

. "${GIT_ROOT:-$(git rev-parse --show-toplevel)}/env_utils.sh"

prerequisites_met || die "Prerequisites check failed!"

if [[ ${ENVSHELL_LOADED} -eq 1 ]]; then
  info "Environment already loaded. Try exit to reload?"
  exit 0
fi

[[ -f ${GIT_ROOT}/.done_env_setup ]] || "${GIT_ROOT}"/env_setup.sh

activate_conda
activate_venv

export ENVSHELL_LOADED=1

cd "${GIT_ROOT}"
bash

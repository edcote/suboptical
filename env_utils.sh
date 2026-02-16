#!/bin/false
# shellcheck shell=bash
# shellcheck source=/dev/null

# Prints info message to stderr
info() {
  echo "$*" >&2
}

# Prints fatal message to stderr and exits
die() {
  local message="Something terrible happened!"
  if [[ $# -gt 0 ]]; then
    message="$*"
  fi
  echo "ERROR: ${message}" >&2
  exit 1
}

# Prompts for confirmation and returns true if yes
confirm() {
  local prompt="${1:-Are you sure? (y/N): }"
  read -r -p "$prompt" response
  if [[ $response =~ ^[yY]$ || $response =~ ^[yY][eE][sS]$ ]]; then
    return 0
  else
    return 1
  fi
}


# Prepends directory to $PATH
add_path() {
  if [[ -d "$1" ]] && [[ ":$PATH:" != *":$1:"* ]]; then
    PATH="$1${PATH:+":$PATH"}"
  fi
}

# Returns true if binary executable found in system path
executable_in_path() {
   builtin type -P "$1" &> /dev/null
}

# Activates Conda environment
activate_conda() {
  local miniconda_root
  pushd . >& /dev/null
  cd "${GIT_ROOT}" || die "\$GIT_ROOT not set?"
  if ! conda env list |& grep -q suboptical; then
    conda env create -f environment.yml
  fi
  miniconda_root=$(realpath "$(dirname "$(which conda)")/..")
  . "${miniconda_root}"/bin/activate suboptical
  popd >& /dev/null || return
}

# Activates Python virtual environment
activate_venv() {
  pushd . >& /dev/null
  cd "${GIT_ROOT}" || die "\$GIT_ROOT not set?"
  export VIRTUAL_ENV_DISABLE_PROMPT=1
  if [[ ! -f .venv/bin/black ]] || ((RANDOM%100==0)); then
    python -m venv .venv
    . .venv/bin/activate
    pip install -q -U pip
    pip install -q -r requirements.txt
  else
    . .venv/bin/activate
  fi
  popd >& /dev/null || return
}

# Gets path to Git root directory and export function to environment
export_git_root() {
  GIT_ROOT=${GIT_ROOT:-$(git rev-parse --show-toplevel)}
  export GIT_ROOT
}
export_git_root

# Checks if prerequisites are met
prerequisites_met() {
  executable_in_path conda || { info "conda not found in \$PATH"; return 1; }
  return 0
}

# Intercepts `git clean -fdx` and prompts for confirmation
git() {
  if [[ "$1" == "clean" ]] && [[ "$*" == *"-fdx"* ]]; then
    echo -e "\033[0;31m⚠️  WARNING: You are about to permanently delete all untracked and ignored files.\033[0m"
    if confirm "Are you absolutely sure you want to proceed? (y/N): "; then
      command git "$@"
    else
      return 1
    fi
  else
    command git "$@"
  fi
}
export -f git

# Runs `gemini-cli` without needing to install the package
gemini() {
  npx @google/gemini-cli
}
export -f gemini

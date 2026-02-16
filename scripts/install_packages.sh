#!/bin/bash -e
# shellcheck source=/dev/null

. "${GIT_ROOT:-$(git rev-parse --show-toplevel)}/env_utils.sh"

if [[ "$EUID" -ne 0 ]]; then
  echo "This script must be run as root. Please use sudo." >&2
  exit 1
fi

if ! confirm "Are you sure you want to install the packages? (y/N): "; then
  echo "Installation aborted."
  exit 0
fi

PACKAGES=(
	dosemu2
)

add-apt-repository -y ppa:dosemu2/ppa
apt-get update -y
apt-get install -y "${PACKAGES[@]}"

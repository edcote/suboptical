# suboptical.org

## Preliminary Setup

### Conda

Install conda using `scripts/install_conda.sh`. Then run script `env_setup.sh` to set up the development environment. This step is only needed once.

Launch development environment shell using `env_shell.sh`.

Update conda packages by editing `environment.yml` then running `update_conda_env.sh`.

### DJGPP

Run `scripts/install_djgpp.sh` to install the C/C++ compiler.

### Packages

Run `scripts/install_packages.sh` to install required packages such as dosemu2.

### Pre-commit hooks

Run pre-commit on all files: `pre-commit run --all-files`.

Run single pre-commit hook on one or more files: `pre-commit run shellcheck --files env_*.sh`.

## Usage Instructions

Launch development environment shell using `env_shell.sh`.

Build using `make all`. Optionally clean using `make clean`.


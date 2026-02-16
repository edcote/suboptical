# suboptical.org

## Project Overview

This repository contains a development environment for 32-bit DOS demoscene demos targeting the i386 architecture, built using the DJGPP compiler suite.

## Tech Stack

- **Target OS:** DOS (32-bit protected mode via DJGPP)
- **Compiler:** DJGPP (GCC-based, `i586-pc-msdosdjgpp-g++`)
- **Language:** C++23 (`-std=gnu++23`)
- **Libraries:**
  - [Embedded Template Library (ETL)](https://www.etlcpp.com/): Used for resource-constrained C++ development (found in `etl/` as a git submodule).
  - DJGPP System Libraries: `<dos.h>`, `<dpmi.h>`, `<go32.h>`, `<sys/nearptr.h>`.
- **Build System:** GNU Makefile. Increments build number in `build_number.txt` and generates `include/build_info.h`.

## Architectural Patterns

- **Resource Management:** `SystemContext` (in `include/system_context.h`) follows the RAII pattern to manage low-level DOS resources (video modes, interrupts, near pointer access). It ensures system state is restored on exit.
- **Low-level Access:** Uses `__djgpp_nearptr_enable()` for direct access to hardware memory (e.g., VGA memory at `0xA0000`).
- **Interrupt Handling:** Timer interrupts are managed via `InstallTimer` and `RemoveTimer` (in `include/timer.h`).
- **Singleton-like Lifecycle:** `SystemContext` is typically managed via `std::unique_ptr` and created through a static `Create()` method.

## Coding Conventions

- **Guidelines:** Follow the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) and [Google Python Style Guide](https://google.github.io/styleguide/pyguide.html).
- **Naming:**
  - Classes/Structs: `PascalCase`
  - Methods/Functions: `PascalCase`
  - Variables: `snake_case`
  - Constants: `kCamelCase` with `k` prefix (e.g., `kVgaStatusRegisterPort`)
  - Member Variables: `snake_case_` with trailing underscore.
- **Formatting:** Adheres to `.clang-format`. Use 2-space indentation.
- **Header Guards:** Use `#pragma once`.
- **Error Handling:** Log errors using `LogError` (from `include/logger.h`) and return `nullptr` or `false` for initialization failures.
- **ETL Usage:** Prefer ETL containers and utilities over the Standard Template Library (STL) where appropriate for memory efficiency and deterministic behavior, though STL is used (e.g., `std::unique_ptr`).

## Development Workflow

- **Environment Setup:** Use `env_shell.sh` to enter the development environment.
- **Building:** Run `make all` to generate `build/supademo.exe` and `build/supademo.iso`.
- **Testing/Debugging:** You can run and debug the final executable in a DOS emulator such as dosemu.
  - Run with: `dosemu -dumb build/supademo.exe`
- **Pre-commit:** Run `pre-commit run --all-files` before committing changes.

## Directory Structure

- `src/`: C++ implementation files (`.cc`).
- `include/`: C++ header files (`.h`).
- `etl/`: Embedded Template Library source and headers (git submodule).
- `build/`: Build artifacts (created during compilation).
- `scripts/`: Utility scripts for environment setup and installation.
- `build_number.txt`: Persists the current build number.

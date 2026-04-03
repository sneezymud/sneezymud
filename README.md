# SneezyMUD

[![License: AGPL v3](https://img.shields.io/badge/License-AGPL%20v3-blue.svg)](LICENSE.txt)
[![Discord](https://img.shields.io/discord/207349014543867904?label=Discord&logo=discord)](https://discord.gg/AE3xf8BQHr)

SneezyMUD is an open-source text-based MUD (Multi-User Dungeon) server with over 30 years of history, tracing its lineage back to DikuMUD. Players explore, fight monsters, complete quests, and interact with each other through typed commands.

## Production Setup

The easiest way to run a production instance of SneezyMUD is with Docker. See the [sneezymud-docker](https://github.com/sneezymud/sneezymud-docker) repository for full deployment instructions.

## Development Setup

### Dev Container (Recommended)

The easiest way to get started. Open the repo in VS Code and it handles everything - toolchain, database, debugging, intellisense. See [`.devcontainer/README.md`](.devcontainer/README.md) for details.

**Requirements:** [Docker Desktop](https://www.docker.com/products/docker-desktop/) (or Docker Engine on Linux), [VS Code](https://code.visualstudio.com/), and the [Dev Containers extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers).

### Via Docker

You can develop using the Docker setup as well, though it can introduce some extra complexity to the process. The sneezymud-docker repo also includes full instructions for setting up a development environment with Docker.

### From Source

For non-Docker development, build from source on Linux (native or WSL2). An in-depth guide can be found [in the wiki](<https://github.com/sneezymud/sneezymud/wiki/Setting-Up-A-Sneezy-Development-Environment-(non%E2%80%90Docker,-Linux-or-Windows-WSL)>).

#### Install Prerequisites

**In Ubuntu LTS:**

```bash
sudo apt update
sudo apt install build-essential \
  ca-certificates \
  ccache \
  clang \
  cmake \
  gdb \
  git \
  libclang-rt-dev \
  libboost-atomic1.83-dev \
  libboost-filesystem1.83-dev \
  libboost-program-options1.83-dev \
  libboost-regex1.83-dev \
  libboost-system1.83-dev \
  libcurl4-openssl-dev \
  libmariadb-dev \
  lld \
  llvm \
  netcat-openbsd \
  ninja-build \
  pkgconf
```

#### Clone Repository

```bash
git clone https://github.com/sneezymud/sneezymud
cd sneezymud
```

#### Database Setup

```bash
# Drops/recreates databases, grants access to your OS user, and loads seed data
./db/init-db.sh
```

Pass a username to grant access to a different MariaDB user: `./db/init-db.sh myuser`

#### Restoring a Production Backup

The seed data gets you a working server, but it's a curated subset of the game world - no player accounts, no accumulated game state, and none of the database changes that builders or admins may have made on the live server. If you have access to the [sneezymud/backups](https://github.com/sneezymud/backups) repo (requires membership in the SneezyMUD hackers team), you can replace your local data with a real nightly backup from production:

```bash
make restore-backup                        # Latest backup, local DB
make restore-backup DATE=2025-12-28        # Specific date
make restore-backup DOCKER=1               # Target a Docker dev environment
```

By default this targets a local MariaDB via socket auth and copies mutable files into `lib/mutable/`. With `DOCKER=1`, it targets the `sneezy-db` container and writes mutable files into the `sneezy-mutable` Docker volume instead.

This is useful when you need real player accounts and characters to reproduce a reported issue, want to test against the full breadth of persisted game state (shops, corpses, rent files, player equipment), or need to pick up database changes that haven't been folded back into the seed data yet.

Both databases are dropped and recreated, so a pre-restore snapshot is saved automatically in case something goes wrong.

## Build System

The easiest way to build is with Make:

```bash
make              # Build with default preset (dev-clang)
make run          # Build and run with sanitizer options
make help         # Show all available targets
```

The binary is output to `build/<preset-name>/code/code/sneezy` with a symlink at `code/sneezy`.

### Direct CMake Usage

For more control, you can use CMake directly:

```bash
cmake --preset <preset-name>        # Configure
cmake --build --preset <preset-name> # Build
```

### Available Presets

| Preset          | Compiler | Type    | ASan | UBSan |
| --------------- | -------- | ------- | ---- | ----- |
| `dev-gcc`       | GCC      | Debug   | Yes  | Yes   |
| `dev-clang`     | Clang    | Debug   | Yes  | Yes   |
| `release-gcc`   | GCC      | Release | Yes  | Yes   |
| `release-clang` | Clang    | Release | Yes  | Yes   |

### Example Build Commands

```bash
# Development build (default: dev-clang)
make

# Use a different preset
make PRESET=dev-gcc

# Production build
make PRESET=release-clang

# Clean current preset's build directory
make clean

# Clean all build directories
make clean-all

# Rebuild from scratch
make rebuild
```

### Code Quality Tools

The Makefile provides several targets for maintaining code quality:

```bash
# Format all C++ source files with clang-format
make format

# Format a specific file
make format FILE=code/code/foo.cc

# Run include-what-you-use analysis (dry run)
make iwyu-check

# Run include-what-you-use and apply fixes
make iwyu-fix

# Run comprehensive static analysis (critical clang-tidy checks + clangsa with CTU)
make analyze

# Export results in various formats
# Add CRITICAL=1 to only export critical issues
make analyze-export FMT=html              # Interactive HTML report (default)
make analyze-export FMT=text              # LLM-friendly text
make analyze-export FMT=sqlite            # SQLite database

# CI gating (exits non-zero if critical violations found)
make analyze-ci
```

### Build Profiling

For investigating build performance:

```bash
# Profile the build using ClangBuildAnalyzer (requires Clang preset)
make build-profile

# Generate Ninja build trace for visualization
make ninja-trace
```

### Build Acceleration

The build system automatically uses these optimizations when available:

| Technique | When Used                 | Benefit                             |
| --------- | ------------------------- | ----------------------------------- |
| ccache    | Auto-enabled if installed | Caches object files across builds   |
| PCH       | Always                    | Pre-parses common STL/Boost headers |
| lld       | Clang builds              | Faster linking                      |

## Running the Server

```bash
# Build and run with sanitizer options (recommended for development)
make run

# Build and run under gdb
make debug

# Or run the binary directly
./code/sneezy
```

The server logs to stdout. Use Ctrl+C to shut down cleanly.

### Command Line Options

```
Usage: sneezy [-p PORT] [-l LIBDIR] [-c CONFIG]

    -p PORT     Listen for Telnet connections on PORT
    -l LIBDIR   Use LIBDIR as the lib flatfiles directory
    -c CONFIG   Read configuration from CONFIG
```

### Defaults

| Setting        | Default              |
| -------------- | -------------------- |
| Telnet port    | 7900                 |
| Config file    | `./sneezy.cfg`       |
| Lib directory  | `./lib`              |
| Database names | `sneezy`, `immortal` |
| Database host  | `localhost`          |

When no database username is configured, it defaults to the current Unix user (like the `mysql` CLI).

### Configuration

Copy an example config file and edit as needed:

```bash
cp code/sneezy.cfg sneezy.cfg
# Edit sneezy.cfg with your database settings
```

### Connecting

Connect with any MUD client or telnet:

```bash
telnet localhost 7900
```

The first character created in a new instance automatically becomes a level 60 immortal.

## Testing

```bash
# C++ unit tests (builds automatically)
make test

# Functional tests (requires running server and .env config)
make test-func

# All tests
make test-all
```

### Continuous Integration

Pull requests run C++ builds, unit tests, and the functional test suite via GitHub Actions. Merges to master are built and published automatically.

> [!TIP]
> **Optional Discord notifications** can be enabled via repository secrets:
> - **New PR opened:** Set `DISCORD_PR_WEBHOOK_URL` to notify when a pull request is first opened.
> - **New build published:** Set `DISCORD_NEW_BUILD_WEBHOOK_URL` to notify when a new game image is available. Optionally set the `DISCORD_ADMIN_ROLE_ID` repository variable to mention a Discord role in the notification.

## Notable Directories

```
code/          External dependencies and config files

code/code/     Actual game source code

lib/           Various game data
  zonefiles/     Zone definitions
  mutable/       Player files, corpses, rent (writable at runtime)
  help/          In-game helpfile content

db/            Database seed data (schema + world data for fresh instances)

cmake/         CMake modules
scripts/       Build and utility scripts
tests/         Functional and C++ unit tests
web/           Old but still useful info about the game
```

## Contributing

1. Join the [SneezyMUD Discord](https://discord.gg/AE3xf8BQHr) to discuss changes
2. Fork the repository and create a feature branch
3. Run `make format` before committing (or configure pre-commit on your system - see `.pre-commit-config.yaml`)
4. Submit a pull request

See the [development wiki](https://github.com/sneezymud/sneezymud/wiki) for detailed guides.

## License

SneezyMUD is free and open-source software, licensed under the [GNU Affero General Public License v3.0](LICENSE.txt).

You are free to use, modify, and distribute this software. If you run a modified version as a network service, the AGPL requires you to make your source code available to users of that service.

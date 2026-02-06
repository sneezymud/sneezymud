# SneezyMUD

[![License: AGPL v3](https://img.shields.io/badge/License-AGPL%20v3-blue.svg)](LICENSE.txt)
[![Discord](https://img.shields.io/discord/207349014543867904?label=Discord&logo=discord)](https://discord.gg/AE3xf8BQHr)

SneezyMUD is an open-source text-based MUD (Multi-User Dungeon) server with over 30 years of history, tracing its lineage back to DikuMUD. Players explore, fight monsters, complete quests, and interact with each other through typed commands.

## Production Setup

The easiest way to run a production instance of SneezyMUD is with Docker. See the [sneezymud-docker](https://github.com/sneezymud/sneezymud-docker) repository for full deployment instructions.

## Development Setup

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
git clone --recurse-submodules https://github.com/sneezymud/sneezymud
cd sneezymud
```

#### Database Setup

```bash
# Start MariaDB
sudo systemctl start mariadb

# Create databases
sudo mariadb -e "CREATE DATABASE sneezy; CREATE DATABASE immortal;"

# Create user (replace $USER with your username)
sudo mariadb -e "CREATE USER '$USER'@'localhost'; \
  GRANT ALL ON sneezy.* TO '$USER'@'localhost'; \
  GRANT ALL ON immortal.* TO '$USER'@'localhost';"

# Load initial data
for db in immortal sneezy; do
  for phase in tables views data; do
    [ -d "_Setup-data/sql_$phase/$db" ] || continue
    for sql in _Setup-data/sql_$phase/$db/*.sql; do
      echo "Loading $sql"
      mysql $db < $sql
    done
  done
done
```

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

| Preset          | Compiler | Type    | ASan | UBSan | LTO     |
| --------------- | -------- | ------- | ---- | ----- | ------- |
| `dev-gcc`       | GCC      | Debug   | Yes  | Yes   | No      |
| `dev-clang`     | Clang    | Debug   | Yes  | Yes   | No      |
| `release-gcc`   | GCC      | Release | Yes  | Yes   | Yes     |
| `release-clang` | Clang    | Release | Yes  | Yes   | ThinLTO |

### Example Build Commands

```bash
# Development build (default: dev-clang)
make

# Use a different preset
make PRESET=dev-gcc

# Production build with LTO
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

Test setup is currently pretty minimal. Run the following from the project root:

```bash
# Boot smoke test (requires database and prior build)
make build verify

# Functional tests (requires running server)
func-test/run_test.sh
```

### Continuous Integration

Pull requests run automated builds and tests via GitHub Actions. Merges to master are built and published automatically via GitHub Actions.

> [!TIP]
> On successful publish, a Discord notification is sent if the `DISCORD_NEW_BUILD_WEBHOOK_URL` repository secret is configured. The notification includes a link to the merged PR, useful for letting admins know when the game can be updated to a new version. Optionally set the `DISCORD_ADMIN_ROLE_ID` repository variable to mention a Discord role in the notification.

## Notable Directories

```
code/          External dependencies and config files

code/code/     Actual game source code

lib/           Various game data
  zonefiles/     Zone definitions
  mutable/       Player files, corpses, rent (writable at runtime)
  help/          In-game helpfile content

_Setup-data/   Database setup
  sql_tables/    Table schemas
  sql_views/     View definitions
  sql_data/      Seed data
  migrations/    Database migrations

cmake/         CMake modules
scripts/       Build and utility scripts
func-test/     Functional tests
web/           Old but still useful info about the game
```

## Database Migrations

Ongoing changes to the database structure are stored as numbered migrations in `_Setup-data/migrations/`. Migrations already applied to the seed data in this repository are in `migrations/applied/` and don't need to be run on fresh installations.

## Contributing

1. Join the [SneezyMUD Discord](https://discord.gg/AE3xf8BQHr) to discuss changes
2. Fork the repository and create a feature branch
3. Run `make format` before committing (or configure pre-commit on your system - see `.pre-commit-config.yaml`)
4. Submit a pull request

See the [development wiki](https://github.com/sneezymud/sneezymud/wiki) for detailed guides.

## License

SneezyMUD is free and open-source software, licensed under the [GNU Affero General Public License v3.0](LICENSE.txt).

You are free to use, modify, and distribute this software. If you run a modified version as a network service, the AGPL requires you to make your source code available to users of that service.

# Dev Container for SneezyMUD

Complete development environment with intellisense, debugging, and a pre-seeded database - no local toolchain setup required. See the [VS Code Dev Containers documentation](https://code.visualstudio.com/docs/devcontainers/containers) for general reference.

## Prerequisites

- [Docker Desktop](https://www.docker.com/products/docker-desktop/) (or equivalent Docker Engine setup)
- [VS Code](https://code.visualstudio.com/) with the [Dev Containers extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers)

## First-time setup

1. Clone the repo and open it in VS Code
2. When prompted "Reopen in Container", click yes (or run **Dev Containers: Reopen in Container** from the command palette)
3. Wait for the container to build

The first build pulls base images and installs the full C++ toolchain. On first boot, the entrypoint seeds the database from `_Setup-data/`, which can take several minutes. Click "show log" in the notification toast to see progress. Subsequent opens reuse the cached image and persistent volumes, starting in seconds.

## Daily workflow

- **Build**: `make` (or Ctrl+Shift+B)
- **Run**: `make run`, or F5 to launch under GDB (the "Debug Sneezy" launch config)
- **Attach debugger**: "Attach to Sneezy" launch config to GDB-attach to a running server
- **Connect**: Mudlet or telnet to `localhost:7900`, or `tt++ -G sneezy localhost 7900` from the container terminal (TinTin++ is pre-installed)

## How it works

| Concern | How |
|---------|-----|
| Database | MariaDB runs as the container's main process (PID 1). On first boot, the entrypoint initializes and seeds both databases from `_Setup-data/`. A sentinel file tracks completion so partial failures are retried on next boot. Data persists in a named volume across restarts and rebuilds. Schema migrations run on game server boot. |
| DB connection | The game server connects to localhost via socket auth - no config needed. The entrypoint creates an `ubuntu` DB user with matching socket auth so terminals and builds work without credentials. |
| Intellisense | clangd runs inside the container with all headers and project dependencies available. The C/C++ (cpptools) extension is also installed - it provides the `cppdbg` debug adapter used by the launch configs. Both extensions are required. |
| Debugging | GDB via F5, or "Attach to Sneezy" for a running server. `SYS_PTRACE` and `seccomp=unconfined` are granted to allow attaching. AddressSanitizer and UndefinedBehaviorSanitizer options are set in `containerEnv` so they apply to all execution methods (F5, `make run`, running the binary directly). |
| Build | cmake + ninja + ccache with the `dev-clang` preset (Debug + sanitizers). Same compiler and dependencies as CI, which uses `release-clang`. The `build/`, `.cache/`, and `lib/mutable/` directories are named Docker volumes for I/O performance and isolation from host-native builds. |
| Packages | The Dockerfile installs tools beyond what's needed to build (curl, vim, nano, htop, ripgrep, etc.). The container is meant to feel like a workstation, not a minimal CI image. Package lists are preserved so `apt install` works without running `apt update` first. |

## Rebuilding

A rebuild is only needed when `.devcontainer/Dockerfile` or `.devcontainer/entrypoint.sh` changes. VS Code won't detect this automatically - trigger it manually:

- **Dev Containers: Rebuild Container** - rebuilds the image (use this one)
- **Dev Containers: Rebuild Without Cache** - full rebuild from scratch (rarely needed)

Rebuilding recreates the container, so manually installed extensions need reinstalling. Extensions listed in `devcontainer.json` are always reinstalled automatically.

### Resetting the database

Database data persists in the `sneezymud-db` volume. To re-seed from scratch (e.g., after `_Setup-data/` changes):

```sh
docker volume rm sneezymud-db
```

Then reopen the container. The entrypoint will detect the missing sentinel file and re-seed - no image rebuild needed. If Docker reports the volume is in use, close the dev container first.

## Customization

The `.vscode/` directory is committed to the repo with shared project settings. For personal preferences, use VS Code's **user-level settings** instead (Ctrl+Shift+P > "Preferences: Open User Settings"), which layer on top of workspace settings.

## Troubleshooting

**Discord webhook warning**: `Failed to open config file 'discord.cfg'` on server boot is expected. Discord integration is only used in production.

**Crash on startup or during gameplay**: The dev build runs with ASan and UBSan in strict mode (`halt_on_error=1`). Check the Debug Console for sanitizer reports - these catch real bugs that may be silent in release builds.

**Database not reachable**: Run `mariadb-admin ping` in the container. If it fails, check the Dev Containers output pane for logs.

**`tt++` not found**: The Dockerfile adds `/usr/games` to PATH, but some non-login shells may not pick it up. Use `/usr/games/tt++` directly.

**Permission denied on `build/`, `.cache/`, or `lib/mutable/`**: The entrypoint fixes ownership on each boot. If it didn't work, run `sudo chown -R ubuntu:ubuntu /workspaces/sneezymud/build /workspaces/sneezymud/.cache /workspaces/sneezymud/lib/mutable`.

## Notes

- Terminals and builds run as the non-root `ubuntu` user. MariaDB runs as root (PID 1).
- Your host git config (name, email, credential helpers) and SSH agent are forwarded into the container automatically. Your global gitignore is not - it references host paths. Add custom ignores to the project's `.gitignore` instead.
- The `ubuntu` home directory (`/home/ubuntu`) is a persistent named volume. Shell history, dotfiles, and authentication tokens survive container rebuilds. For extensions, add them to `devcontainer.json` (project-wide) or your user setting `dev.containers.defaultExtensions` (personal).
- The `sneezymud-build`, `sneezymud-ccache`, and `sneezymud-mutable` volumes similarly persist across rebuilds. Only `docker volume rm` resets them.
- **Windows**: Clone into the WSL2 filesystem (`/home/...`), not the Windows filesystem (`/mnt/c/...`). Bind mount performance across the VM boundary is significantly slower.
- **macOS**: Ensure Docker Desktop uses VirtioFS (Settings > General > "Virtual file sharing implementation"). It's the default on newer installs, but older ones may use gRPC-FUSE, which is noticeably slower.

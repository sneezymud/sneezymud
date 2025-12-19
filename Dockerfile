# syntax=docker/dockerfile:1
#
# Packaging-only Dockerfile for GitHub Actions CI/CD.
# Expects the binary and lib/ directory to be pre-built in the build context.
# For local development with in-container builds, see dev.Dockerfile.

ARG UBUNTU_VERSION=noble

FROM ubuntu:${UBUNTU_VERSION}
LABEL maintainer="SneezyMUD Development Team <https://discord.gg/F5zdYwWBzY>"
LABEL org.opencontainers.image.source="https://github.com/sneezymud/sneezymud"
LABEL org.opencontainers.image.description="SneezyMUD Game Server"

ENV DEBIAN_FRONTEND=noninteractive

# Sanitizer options for enhanced error reporting (ASan + UBSan enabled in production builds)
# halt_on_error=0 for UBSan allows continued execution after logging undefined behavior
ENV ASAN_OPTIONS=strict_string_checks=1:detect_stack_use_after_return=1:check_initialization_order=1:strict_init_order=1
ENV UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=0

RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
  apt-get update && \
  TZ=utc apt-get install --yes --no-install-recommends \
  ca-certificates \
  gdb \
  libboost-atomic1.83.0 \
  libboost-filesystem1.83.0 \
  libboost-program-options1.83.0 \
  libboost-regex1.83.0 \
  libboost-system1.83.0 \
  libcurl4 \
  libmariadb3 && \
  apt-get clean && \
  rm -rf /var/lib/apt/lists/*

# Rename the default 'ubuntu' user (UID/GID 1000) to 'sneezy'.
# UID 1000 must be preserved to match file ownership in the sneezy-mutable Docker volume.
# The monitor service running on the host sets volume ownership to UID 1000, so the in-container user must match
# or the game server will not have permission to read/write its data files.
RUN usermod -l sneezy -d /home/sneezy -m ubuntu && groupmod -n sneezy ubuntu

COPY --chown=sneezy:sneezy code/sneezy /home/sneezy/code/sneezy
COPY --chown=sneezy:sneezy lib /home/sneezy/lib
COPY --chown=sneezy:sneezy code/sneezy.cfg /home/sneezy/code/sneezy.cfg

WORKDIR /home/sneezy/code

EXPOSE 7900
USER sneezy
CMD ["./sneezy"]

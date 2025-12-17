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
  libmariadb3 \
  netcat-openbsd && \
  apt-get clean && \
  rm -rf /var/lib/apt/lists/*

# Use existing ubuntu user if present, otherwise create it
# useradd -r flag creates a system account, which is preferrable when running as a service
# useradd -m flag forces creation of home directory, which -r flag prevents by default
RUN getent group ubuntu >/dev/null || groupadd -r ubuntu && \
    id ubuntu >/dev/null 2>&1 || useradd -r -g ubuntu -m ubuntu

# Copy pre-built artifacts from build context
COPY --chown=ubuntu:ubuntu code/sneezy /home/ubuntu/code/sneezy
COPY --chown=ubuntu:ubuntu lib /home/ubuntu/lib
COPY --chown=ubuntu:ubuntu code/sneezy.cfg /home/ubuntu/code/sneezy.cfg

WORKDIR /home/ubuntu/code

EXPOSE 7900
USER ubuntu
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 CMD nc -z localhost 7900 || exit 1
CMD ["./sneezy"]

# syntax=docker/dockerfile:1

# Build arguments
ARG UBUNTU_VERSION=noble
ARG BRANCH="master"
ARG BUILD_OPTS="asan=1 optimize=1"

FROM ubuntu:${UBUNTU_VERSION} AS build

# Set this first to ensure it applies to all commands
ENV DEBIAN_FRONTEND=noninteractive

# Using cache mounts throughout and removing cache after install reduces image size and speeds up builds
RUN --mount=type=cache,target=/var/cache/apt \
  apt-get update && \
  TZ=utc apt-get install --yes --no-install-recommends \
  build-essential \
  ca-certificates \
  git \
  libboost-filesystem1.83-dev \
  libboost-program-options1.83-dev \
  libboost-regex1.83-dev \
  libboost-system1.83-dev \
  libboost1.83-dev \
  libcurl4-openssl-dev \
  libmariadb-dev \
  pkgconf \
  scons && \
  apt-get clean && \
  rm -rf /var/lib/apt/lists/*

RUN --mount=type=cache,target=/var/cache/apt \
  echo Building from branch: "${BRANCH}" && \
  git clone \
  --depth 1 \
  --shallow-submodules \
  --recurse-submodules \
  --single-branch \
  --branch "${BRANCH}" \
  --no-tags https://github.com/sneezymud/sneezymud /home/ubuntu/sneezymud

RUN --mount=type=cache,target=/home/ubuntu/sneezymud/code/objs \
  scons -C /home/ubuntu/sneezymud/code -j$(nproc) ${BUILD_OPTS} sneezy

FROM ubuntu:${UBUNTU_VERSION} AS run
LABEL maintainer="SneezyMUD Development Team <https://discord.gg/F5zdYwWBzY>"
LABEL org.opencontainers.image.source="https://github.com/sneezymud/sneezymud"
LABEL org.opencontainers.image.description="SneezyMUD Game Server"

ENV DEBIAN_FRONTEND=noninteractive

RUN --mount=type=cache,target=/var/cache/apt \
  apt-get update && \
  TZ=utc apt-get install --yes --no-install-recommends \
  ca-certificates \
  gdb \
  libasan8 \
  libboost-filesystem1.83.0 \
  libboost-program-options1.83.0 \
  libboost-regex1.83.0 \
  libboost-system1.83.0 \
  libcurl4 \
  libmariadb3 \
  netcat-openbsd && \
  apt-get clean && \
  rm -rf /var/lib/apt/lists/*

# useradd -r flag creates a system account, which is preferrable when running as a service
# useradd -m flag forces creation of home directory, which -r flag prevents by default
RUN groupadd -r ubuntu && useradd -r -g ubuntu -m ubuntu

COPY --from=build --chown=ubuntu:ubuntu /home/ubuntu/sneezymud/code/sneezy /home/ubuntu/code/sneezy
COPY --from=build --chown=ubuntu:ubuntu /home/ubuntu/sneezymud/lib /home/ubuntu/lib
COPY --from=build --chown=ubuntu:ubuntu /home/ubuntu/sneezymud/code/sneezy.cfg /home/ubuntu/code/sneezy.cfg

WORKDIR /home/ubuntu/code

EXPOSE 7900
USER ubuntu
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 CMD nc -z localhost 7900 || exit 1
CMD ["./sneezy"]

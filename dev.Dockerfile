# syntax=docker/dockerfile:1

FROM ubuntu:noble

ENV DEBIAN_FRONTEND=noninteractive

RUN --mount=type=cache,target=/var/cache/apt \
  apt-get update && \
  TZ=utc apt-get install --yes --no-install-recommends \
  build-essential \
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
  pkgconf \
  tintin++ && \
  apt-get clean && \
  rm -rf /var/lib/apt/lists/* && \
  git config --global --add safe.directory /sneezymud-docker/services/sneezymud

WORKDIR /sneezymud-docker/services/sneezymud

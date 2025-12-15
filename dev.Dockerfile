# syntax=docker/dockerfile:1

FROM ubuntu:noble

ENV DEBIAN_FRONTEND=noninteractive

RUN --mount=type=cache,target=/var/cache/apt \
  apt-get update && \
  TZ=utc apt-get install --yes --no-install-recommends \
  build-essential \
  ca-certificates \
  cmake \
  gdb \
  git \
  libasan8 \
  libubsan1 \
  libboost-filesystem1.83-dev \
  libboost-program-options1.83-dev \
  libboost-regex1.83-dev \
  libboost-system1.83-dev \
  libboost1.83-dev \
  libcurl4-openssl-dev \
  libmariadb-dev \
  mold \
  netcat-openbsd \
  ninja-build \
  pkgconf \
  tintin++ && \
  apt-get clean && \
  rm -rf /var/lib/apt/lists/* && \
  git config --global --add safe.directory /sneezymud-docker/services/sneezymud

WORKDIR /sneezymud-docker/services/sneezymud

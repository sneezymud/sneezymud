FROM ubuntu:focal AS build

RUN apt-get update && \
  DEBIAN_FRONTEND=noninteractive TZ=utc apt-get install --yes --no-install-recommends \
  build-essential \
  libboost-dev \
  libboost-program-options-dev \
  libboost-regex-dev \
  libboost-filesystem-dev \
  libboost-system-dev \
  pkg-config \
  libmariadbclient-dev \
  scons \
  libcurl4-openssl-dev \
  git \
  ca-certificates \
  gdb \
  netcat \
  tintin++ && \
  git config --global --add safe.directory /sneezymud-docker/sneezymud

WORKDIR /sneezymud-docker/sneezymud/code

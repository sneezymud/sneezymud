FROM ubuntu:focal AS build
LABEL maintainer Elmo Todurov <elmo.todurov@eesti.ee>

# Set this first to ensure it applies to all commands
ENV DEBIAN_FRONTEND=noninteractive

# Removing cache after install reduces image size
RUN apt-get update && TZ=utc apt-get install --yes --no-install-recommends build-essential libboost-dev libboost-program-options-dev libboost-regex-dev libboost-filesystem-dev libboost-system-dev libmariadbclient-dev scons libcurl4-openssl-dev git ca-certificates && apt-get clean && rm -rf /var/lib/apt/lists/*

ARG UID=1000
ARG BRANCH="master"

# This could possibly also be accomplished by passing the --no-cache flag
ARG FORCE_REBUILD=1

# Combining commands reduces the number of layers, which reduces image size
RUN useradd -m -u "$UID" sneezy && \
  echo Building from branch: "$BRANCH" && \
  git clone --depth 1 --shallow-submodules --recurse-submodules --single-branch --branch "$BRANCH" --no-tags https://github.com/sneezymud/sneezymud /home/sneezy/sneezymud && \
  scons -C /home/sneezy/sneezymud/code -j`nproc` -Q debug=1 sanitize=1 fortify=1 olevel=2 sneezy

FROM ubuntu:focal AS run

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && TZ=utc apt-get install --yes --no-install-recommends libboost-program-options1.71.0 libboost-regex1.71.0 libboost-filesystem1.71.0 libboost-system1.71.0 libmariadb3 libcurl4 libasan5 gdb && apt-get clean && rm -rf /var/lib/apt/lists/*

ARG UID=1000

RUN useradd -m -u "$UID" sneezy && \
  mkdir -p /home/sneezy/code/objs/ && \
  mkdir -p /home/sneezy/lib

WORKDIR /home/sneezy/code

COPY --from=build --chown=sneezy:sneezy /home/sneezy/sneezymud/code/sneezy /home/sneezy/code/sneezy
COPY --from=build --chown=sneezy:sneezy /home/sneezy/sneezymud/lib /home/sneezy/lib
COPY --from=build --chown=sneezy:sneezy /home/sneezy/sneezymud/code/sneezy.cfg /home/sneezy/code/sneezy.cfg

EXPOSE 7900
USER sneezy
CMD ./sneezy

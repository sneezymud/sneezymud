# syntax=docker/dockerfile:1

FROM ubuntu:noble

ARG DEBIAN_FRONTEND=noninteractive

# tintin++ is installed to /usr/games, which is not in the PATH by default on Ubuntu.
# Add it here so tintin++ can be easily found/run from the terminal inside the devcontainer.
ENV PATH="/usr/games:${PATH}"
ENV TZ=UTC

RUN --mount=type=cache,target=/var/cache/apt \
  apt-get update && \
  apt-get install --yes --no-install-recommends \
  bash-completion \
  build-essential \
  ca-certificates \
  ccache \
  clang \
  clangd \
  cmake \
  curl \
  gdb \
  git \
  htop \
  jq \
  less \
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
  man-db \
  mariadb-server \
  nano \
  netcat-openbsd \
  ninja-build \
  pkgconf \
  ripgrep \
  sudo \
  tintin++ \
  vim \
  wget && \
  usermod -aG sudo ubuntu && \
  echo "ubuntu ALL=(root) NOPASSWD:ALL" >/etc/sudoers.d/ubuntu && \
  chmod 0440 /etc/sudoers.d/ubuntu && \
  # The bind-mounted workspace is owned by the host user, which may not match
  # the container's ubuntu user. Without safe.directory, git refuses to operate.
  git config --system --add safe.directory /workspaces/sneezymud

COPY --chmod=755 .devcontainer/entrypoint.sh /entrypoint.sh
ENTRYPOINT ["/entrypoint.sh"]

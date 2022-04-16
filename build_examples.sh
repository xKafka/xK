#!/usr/bin/env bash

# This script is intended to be run from an Ubuntu Docker image (or other Debian-derived distros).
# It uses `apt-get` to install dependencies required by the examples,

set -euo pipefail

function install_qt() {
  apt-get install -y --no-install-recommends qt5-default qtbase5-dev
}

# call the above functions to install the required dependencies. As an example for qt:
# install_qt

# build with:
cmake -S . -B "./build"
cmake --build "./build"
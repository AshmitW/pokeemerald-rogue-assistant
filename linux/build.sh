#!/usr/bin/env bash
# Build RogueAssistant.so and the RogueAssistant launcher.
#
#   bash build.sh
#
# Builds directly with CMake when the dependencies are installed. If they are
# not, and a toolbox/podman container named by $ROGUE_CONTAINER exists, the
# build is retried inside it (used on Fedora, which has no mGBA package and
# where we keep dependencies out of the host system).
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD="$HERE/build-assistant"
CONTAINER="${ROGUE_CONTAINER:-mgba-build}"

have_deps() {
    command -v cmake >/dev/null 2>&1 || return 1
    command -v c++   >/dev/null 2>&1 || return 1
    command -v python3 >/dev/null 2>&1 || return 1
    pkg-config --exists sfml-graphics 2>/dev/null || return 1
    pkg-config --exists libenet 2>/dev/null || return 1
    return 0
}

do_build() {
    mkdir -p "$BUILD"
    cmake -S "$HERE" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release >/dev/null
    cmake --build "$BUILD" -j"$(nproc)"
}

if have_deps; then
    do_build
elif command -v toolbox >/dev/null 2>&1 && toolbox list -c 2>/dev/null | grep -q "[[:space:]]$CONTAINER[[:space:]]"; then
    echo "Dependencies not found on the host; building inside container '$CONTAINER'..."
    exec toolbox run -c "$CONTAINER" bash "$HERE/build.sh"
else
    cat >&2 <<'MSG'
error: missing build dependencies.

  Debian / Ubuntu:
    sudo apt install cmake g++ python3 pkg-config libsfml-dev libenet-dev

  Arch:
    sudo pacman -S cmake gcc python pkgconf sfml enet

  Fedora (no mGBA package - see README for the container route):
    sudo dnf install cmake gcc-c++ python3 pkgconf-pkg-config SFML-devel enet-devel

MSG
    exit 1
fi

echo
echo "built:"
echo "  $BUILD/RogueAssistant.so   (loaded by mGBA)"
echo "  $BUILD/RogueAssistant      (launcher - run this first)"

#!/usr/bin/env bash
# Rebuild RogueAssistant.so inside the toolbox container.
# Run from the host:  bash linux/build.sh
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONTAINER="mgba-build"
BUILD="$HERE/build-assistant"

mkdir -p "$BUILD"
toolbox run -c "$CONTAINER" sh -c "cd '$BUILD' && cmake -DCMAKE_BUILD_TYPE=Release '$HERE' >/dev/null && cmake --build . -j\$(nproc)"
echo
echo "built: $BUILD/RogueAssistant.so"

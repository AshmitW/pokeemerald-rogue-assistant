#!/usr/bin/env bash
# Launch the custom mGBA with the freshly built assistant staged beside its
# Lua script. Run from the host:  bash linux/run-mgba.sh
#
# (The project drive is mounted noexec-ish via fuseblk, so this script is not
# marked executable - invoke it with bash.)
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CONTAINER="mgba-build"
BUILD="$HERE/build-assistant"
DIST="$HERE/dist"

if [ ! -f "$BUILD/RogueAssistant.so" ]; then
    echo "error: $BUILD/RogueAssistant.so not built yet" >&2
    echo "  build it with: bash $HERE/build.sh" >&2
    exit 1
fi

mkdir -p "$DIST"
cp -f "$BUILD/RogueAssistant.so" "$DIST/"
cp -f "$BUILD/RogueAssistant" "$DIST/" 2>/dev/null || true
chmod +x "$DIST/RogueAssistant" 2>/dev/null || true
# Dev shortcut: copy the script directly rather than running the launcher, which
# would block on its instruction window. End users run ./RogueAssistant instead.
cp -f "$HERE/../RogueAssistantCpp/Assets/RogueAssistant_mGBA.lua" "$DIST/"

echo "Staged:"
echo "  $DIST/RogueAssistant       (launcher)"
echo "  $DIST/RogueAssistant.so"
echo "  $DIST/RogueAssistant_mGBA.lua"
echo
echo "In mGBA:  Tools > Scripting > File > Load Script"
echo "  $DIST/RogueAssistant_mGBA.lua"
echo

exec toolbox run -c "$CONTAINER" mgba-qt "$@"

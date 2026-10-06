# Linux build for Rogue Assistant, replacing the MSVC .vcxproj.
# Produces RogueAssistant.so - an mGBA Lua C module.
cmake_minimum_required(VERSION 3.16)
project(RogueAssistant CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../RogueAssistantCpp)

find_package(SFML 2.6 REQUIRED COMPONENTS graphics window system network)
find_package(PkgConfig REQUIRED)
pkg_check_modules(ENET REQUIRED libenet)

# Regenerate the embedded assets (replaces the Windows-only bin2cpp.exe).
add_custom_target(embed_assets
    COMMAND ${CMAKE_COMMAND} -E echo "Embedding assets..."
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/tools/embed_assets.py
            ${SRC_ROOT}/Assets ${SRC_ROOT}/Embedded
    BYPRODUCTS ${SRC_ROOT}/Embedded/bin2cpp_common.h
    COMMENT "Generating Embedded/ from Assets/")

add_library(RogueAssistant SHARED
    ${SRC_ROOT}/Src/Assets.cpp
    ${SRC_ROOT}/Src/DataStream.cpp
    ${SRC_ROOT}/Src/GameConnection.cpp
    ${SRC_ROOT}/Src/GameConnectionManager.cpp
    ${SRC_ROOT}/Src/GameConnectionRPCs.cpp
    ${SRC_ROOT}/Src/GameData.cpp
    ${SRC_ROOT}/Src/Log.cpp
    ${SRC_ROOT}/Src/LuaMain.cpp
    ${SRC_ROOT}/Src/Main.cpp
    ${SRC_ROOT}/Src/ObservedGameMemory.cpp
    ${SRC_ROOT}/Src/Timer.cpp
    ${SRC_ROOT}/Src/UserData.cpp
    ${SRC_ROOT}/Src/Behaviours/CommonBehaviour.cpp
    ${SRC_ROOT}/Src/Behaviours/HomeBoxBehaviour.cpp
    ${SRC_ROOT}/Src/Behaviours/MultiplayerBehaviour.cpp
    ${SRC_ROOT}/Src/UI/PrimaryUI.cpp
    ${SRC_ROOT}/Src/UI/Window.cpp)

add_dependencies(RogueAssistant embed_assets)

target_include_directories(RogueAssistant PRIVATE
    ${SRC_ROOT}/Inc
    ${SRC_ROOT}
    ${ENET_INCLUDE_DIRS})

# Lua symbols are deliberately left undefined: mgba-qt is linked
# --export-dynamic and provides them at load time. Linking liblua here would
# give us a second Lua runtime operating on mGBA's lua_State - the same
# fragile arrangement the Windows DLL is stuck with.
target_link_libraries(RogueAssistant PRIVATE
    sfml-graphics sfml-window sfml-system sfml-network
    ${ENET_LIBRARIES}
    pthread)

# Lua's require() looks for exactly "RogueAssistant.so", no "lib" prefix.
set_target_properties(RogueAssistant PROPERTIES PREFIX "" OUTPUT_NAME "RogueAssistant")

# Launcher: the Linux counterpart to RogueAssistant.exe. Extracts the embedded
# RogueAssistant_mGBA.lua next to the library and shows the connect instructions.
# Named "RogueAssistant" like the .so - on Linux the filenames don't collide.
add_executable(RogueAssistantLauncher
    ${CMAKE_CURRENT_SOURCE_DIR}/../RogueAssistantStub/RogueAssistantStub.cpp)

target_link_libraries(RogueAssistantLauncher PRIVATE RogueAssistant)

# RogueAssistant.so deliberately leaves lua_* undefined - mGBA supplies them when
# it dlopens the library. The launcher never reaches any Lua code path, so allow
# the unresolved symbols and force lazy binding, which means they are only ever
# resolved if actually called. (All of them are functions, so lazy binding applies;
# an undefined *data* symbol would still bind eagerly and fail.)
target_link_options(RogueAssistantLauncher PRIVATE
    -Wl,--allow-shlib-undefined
    -Wl,-z,lazy)

set_target_properties(RogueAssistantLauncher PROPERTIES
    OUTPUT_NAME "RogueAssistant"
    # Find RogueAssistant.so beside the executable rather than needing LD_LIBRARY_PATH
    BUILD_RPATH "$ORIGIN"
    INSTALL_RPATH "$ORIGIN")

# Rogue Assistant on Linux

## 1. Description

Rogue Assistant is a companion program for the game. The game uses it for
multiplayer sessions and for additional Pokemon storage.

This project makes two files for Linux:

- `RogueAssistant.so` is the library. The mGBA emulator loads this library.
- `RogueAssistant` is the launcher. It writes the script file and shows the
  connection instructions. It does the same task as `RogueAssistant.exe` on
  Windows.

This project does not supply compiled files. You must build them. The build takes
less than one minute.

There are two reasons for this condition:

- A compiled file uses the C library of the computer that built it. Different
  Linux distributions supply different versions of the C library. An older
  version cannot run a file that a newer version built.
- The library uses SFML. Some distributions supply SFML 2.5. Other distributions
  supply SFML 2.6. The two versions are not compatible.

Thus one compiled file cannot operate on all distributions.

## 2. Before you start

Rogue Assistant is a native Lua module. Lua loads a native module with the
`dlopen` function of the operating system. Some Lua libraries do not include this
function. The mGBA emulator can load Rogue Assistant only if its Lua library
includes `dlopen`.

Make sure that your mGBA emulator has this support. Refer to Table 1.

**Table 1 — mGBA support**

| Source of mGBA | Support | Reason |
|---|---|---|
| Debian or Ubuntu (`apt install mgba-qt`) | Yes | mGBA links to the `liblua5.4-0` package. This library includes `dlopen`. |
| Arch (`pacman -S mgba-qt`) | Yes | The package lists `lua` as an optional dependency for script support. |
| Fedora | No package | Fedora does not supply mGBA. Refer to Paragraph 5. |
| The official AppImage | No | The AppImage contains a Lua library that does not include `dlopen`. |

If mGBA shows the message `dynamic libraries not enabled; check your Lua
installation`, then the Lua library does not include `dlopen`. This is a property
of the mGBA program. A change to the file paths cannot correct this fault. You
must use a different mGBA emulator.

## 3. Build

The build needs these packages:

| Package | Purpose |
|---|---|
| `cmake`, `g++`, `python3`, `pkg-config` | Standard build tools |
| SFML 2.6 development files | The user interface |
| ENet development files | The multiplayer network |

The build needs SFML 2.6. Some distributions supply a different version:

- Debian 12 and Ubuntu 22.04 supply SFML 2.5. This version is too old.
- Arch supplies SFML 3. SFML 3 is not compatible with SFML 2. Install SFML 2
  from the AUR instead.

Install the packages that you do not have already:

```bash
# Debian 13 or Ubuntu 24.04 and later
sudo apt install cmake g++ python3 pkg-config libsfml-dev libenet-dev

# Arch (get SFML 2 from the AUR; the repository package is SFML 3)
sudo pacman -S cmake gcc python pkgconf enet
```

Then build:

```bash
git clone <this repository>
cd pokeemerald-rogue-assistant/linux
bash build.sh
```

The build writes `RogueAssistant.so` and `RogueAssistant` to the
`build-assistant` directory.

## 4. Operate the assistant

Start the launcher:

```bash
cd build-assistant
./RogueAssistant
```

The launcher writes the file `RogueAssistant_mGBA.lua` into the same directory as
the library. This is the correct location. The launcher then shows the connection
instructions.

**CAUTION: DO NOT MOVE THE SCRIPT FILE. LUA SEARCHES FOR THE LIBRARY IN THE
DIRECTORY OF THE SCRIPT FILE. IF YOU MOVE THE SCRIPT FILE TO A DIFFERENT
DIRECTORY, LUA CANNOT FIND THE LIBRARY.**

Then, in mGBA:

1. Load your ROM file.
2. Select **Tools → Scripting → File → Load Script**.
3. Select the file `RogueAssistant_mGBA.lua`.

The window of the assistant shows `Connected to Game`.

## 5. Fedora

Fedora does not supply an mGBA package. The official AppImage cannot load native
Lua modules. Thus you must build mGBA.

An mGBA build needs many dependencies. Build mGBA in a container. A container
keeps these dependencies off your system. You can then remove all of them with
one command.

```bash
toolbox create -y -c mgba-build

toolbox run -c mgba-build sudo dnf install -y \
    cmake gcc gcc-c++ make pkgconf-pkg-config lua-devel \
    qt5-qtbase-devel qt5-qtmultimedia-devel qt5-qttools-devel qt5-qtsvg-devel \
    SDL2-devel libzip-devel libpng-devel sqlite-devel \
    elfutils-libelf-devel libepoxy-devel libedit-devel SFML-devel enet-devel

curl -sSL -o mgba.tar.gz https://github.com/mgba-emu/mgba/archive/refs/tags/0.10.5.tar.gz
tar xzf mgba.tar.gz && mkdir -p mgba-build-dir

toolbox run -c mgba-build sh -c "cd mgba-build-dir && \
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
          -DUSE_FFMPEG=OFF -DBUILD_QT=ON -DUSE_LUA=ON ../mgba-0.10.5 && \
    cmake --build . -j\$(nproc) && sudo cmake --install ."
```

This mGBA links to the Lua library of Fedora. That library includes `dlopen`.
Thus this mGBA can load Rogue Assistant.

Start mGBA from the container:

```bash
toolbox run -c mgba-build mgba-qt
```

The script `build.sh` finds that the host does not have the dependencies. The
script then builds the files in the same container.

To remove the container and all its dependencies:

```bash
toolbox rm -f mgba-build
```

## 6. File locations

The assistant writes these files:

```
$XDG_DATA_HOME/pokabbie/rogue_assistant/       (default: ~/.local/share/...)
├── settings.ini
├── RogueAssistant.log
└── <rogueVersion>/<trainerId>/boxes.dat
```

The Windows version uses the directory `%appdata%\.pokabbie\rogue_assistant\`.
The Linux directory is the equivalent location. The file `boxes.dat` has the same
format on the two operating systems. Thus you can move this file between a
Windows computer and a Linux computer.

## 7. Troubleshooting

| Message | Cause | Correction |
|---|---|---|
| `module 'RogueAssistant' not found` | The script file is not in the directory of the library. | Load the script file from the directory that contains `RogueAssistant.so`. |
| `dynamic libraries not enabled` | The Lua library of mGBA does not include `dlopen`. | Use a different mGBA emulator. Refer to Table 1. |
| `Cannot connect to Game. Do you need to update RogueAssistant?` | The game and the assistant have different compatibility versions. The assistant refuses to connect, because the memory layout can be different. | Use a version of the assistant that agrees with your version of the game. |

You can play multiplayer sessions with Windows players. The two versions use the
same network protocol.

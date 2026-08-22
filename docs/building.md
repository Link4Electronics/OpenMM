# How to build

To be able to make changes to the OpenMM code a development environment is required. This page will explain how to set this up.

## Dependencies

OpenMM uses [CMake](https://cmake.org/) (3.20+) and builds natively on Linux. Required:

- SDL3 (`libsdl3-dev`, or build from source — see below)
- OpenGL development headers (`libgl1-mesa-dev`)
- FreeType (`libfreetype-dev`)

Optional:

- FFmpeg dev libraries (`libavformat-dev libavcodec-dev libswscale-dev libavutil-dev`) — enables intro video playback.

On Ubuntu 24.04 and older, SDL3 is not packaged; build it once:

```sh
git clone --depth 1 --branch release-3.2.16 https://github.com/libsdl-org/SDL sdl3
cmake -S sdl3 -B sdl3/build -DCMAKE_BUILD_TYPE=Release -DSDL_TESTS=OFF -DSDL_EXAMPLES=OFF
sudo cmake --install sdl3/build
```

(CI instead uses the [`libsdl-org/setup-sdl`](https://github.com/libsdl-org/setup-sdl) action, which builds and caches SDL automatically.)

## Git clone

Use [git](https://git-scm.com/) to clone the repo with the following command:
```
git clone https://github.com/0x1F9F1/OpenMM.git
```

Replace `0x1F9F1` with your own GitHub username if you created a fork already.

This will create the OpenMM directory.

## Build

From the repo root:

```sh
mkdir build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
```

The binary is copied to `game/OpenMM` after linking. Run it from your Midtown Madness directory (the folder containing `core.ar`, `ui.ar`, ...).

## Get started

Open the code in your favorite editor (VS Code / CLion both understand the CMake setup: open the repository root and select the `build` directory as the CMake build dir). If all the previous steps were done correctly, running `game/OpenMM` inside your MM1 install should build and run OpenMM. Feel free to play around with the code and make changes.

For more in-depth development information, see [methodology](./methodology.md).

## Contributing

* Run `tools/format.py` to ensure your code is properly formatted.
* Run `tools/asm.py`, checking there are no unused exports.
* Ensure your code compiles without warnings.

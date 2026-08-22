# OpenMM

![Preview](extra/preview.png)

OpenMM is an open source re-implementation of Midtown Madness Sneak Preview Beta / Build 1560.

This project is a partial rewrite of Midtown Madness 1, using assembly to provide functions that have not yet been reimplemented in C++.<br/>
The intention is to allow the fixing of bugs, implementation of new features and porting to platforms unsupported by the original.<br/>

## Status

Decompilation progress against the original build 1560 (`game/midtown.map`),
tracked with [reccmp](docs/reccmp.md):

Midtown Madness 1

<img src="reccmp-report/progress.svg" alt="decompilation progress" width="50%">

Midtown Madness 2 (not impl.)

## Changes

Notable changes include:
* OpenGL Renderer
* SDL Gamepad Support
* Audio Fixes
* Crash Fixes
* Input Fixes
* Stuttering Fixes
* Improved Debug Menu
* Improved Performance
* Improved/Fixed Text Rendering

## Building

```sh
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

Requires data files from the original game (`*.ar`, `midtown.map`) placed
in a directory named `game`. See [docs/building.md](docs/building.md) for details.

Optional: FFmpeg libraries (libavformat, libavcodec, libswscale) enable
intro video playback.

## License

This project is licensed under the **GNU General Public License v3.0** – see the [LICENSE](LICENSE.txt) file for details.

This project is **not affiliated with or endorsed by Microsoft.** "Midtown Madness" is a registered trademark of Microsoft Corporation. All game assets (data files, graphics, sounds, 3D models) remain the property of their respective owners. The reimplementation source code is the only portion covered by the GPLv3 license.

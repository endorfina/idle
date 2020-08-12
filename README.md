# idle

### About

*Boo!* 👻
Idle is an OpenGL vector graphics engine targeted for X11 and Android's NDK, with strong focus on `constexpr`.
Aside from more technical objectives, its original one was to help me deal with some lingering symptoms of ptsd.

For information about "Crimson", an example game developed alongside idle, visit [its homepage](https://idle.endorfina.dev/crimson).

### Dependencies

Requires a C++17 compiler to be available.
Shared library dependencies include freetype, alsa and GLX - instructions for installing which can be found in the following section.
Additionally, idle includes a slightly modified [SoLoud](http://sol.gfxile.net/soloud/) library used for xmod playback as well as [lodepng](https://github.com/lvandeve/lodepng) for image decoding.

Installing dependencies on **Solus**:
```sh
sudo eopkg it cmake libx11-devel alsa-lib-devel mesalib-devel freetype2-devel zlib-devel
```

Installing dependencies on **Ubuntu**:
```sh
sudo apt-get install cmake libx11-dev libasound2-dev libgl1-mesa-dev libfreetype6-dev libz-dev
```

Installing dependencies on **Fedora**:
```sh
sudo dnf install cmake libX11-devel alsa-lib-devel mesa-libGL-devel freetype-devel zlib-devel
```

### Building

Simply run the associated bash configure script and then `make`:
```sh
./configure.sh && make
```

The configure script accepts some flags, however, by default it will prepare a proper CMake release build.

Add the `-n` flag if you have a *need* for more speed. 😁

`make run` will start idle.

### All wrongs reserved

![GPLv3](https://www.gnu.org/graphics/gplv3-88x31.png) [![CC-BY-SA](https://i.creativecommons.org/l/by-sa/4.0/88x31.png)](http://creativecommons.org/licenses/by-sa/4.0/)

idle is licensed under **GNU General Public License version 3**.
Every asset of my authorship that's distributed alongside this software (i.e. images, music, etc.) is licensed under [Creative Commons Attribution-ShareAlike 4.0 International License](http://creativecommons.org/licenses/by-sa/4.0/) unless otherwise stated.


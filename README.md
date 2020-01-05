# idle

### About

*Boo!* 👻
Idle is an OpenGL vector graphics engine targeted for X11 and Android's NDK.
The project's prime objective is to help me deal with lingering symptoms of ptsd.

Requires a C++17 compiler to be available.
Depends on linking against freetype, alsa and GLX - instructions for installing these can be found in the section below.
Idle also includes a slightly modified [SoLoud](http://sol.gfxile.net/soloud/) library for xmod playback as well as [lodepng](https://github.com/lvandeve/lodepng) for image decoding.

### Building

Just run the associated bash script:
```sh
./configure.sh && make
```

Configure script accepts some flags, however by default it will prepare a proper `cmake` release build.
`-l` flag forces the build system to use both clang and libc++ for compilation.
`-n` translates to `-march=native -mtune=native`.
`-j` will try to merge the core library files into a jumbo build.
Personally I'd recommend running `./configure.sh -nj` for an optimal solution.
There are others but they're mostly for development (and quite straightforward).

### Dependencies

Installing dependencies on **Solus**:
```sh
sudo eopkg it libx11-devel alsa-lib-devel mesalib-devel freetype2-devel zlib-devel
```

Installing dependencies on **Ubuntu**:
```sh
sudo apt-get install libx11-dev libasound2-dev libgl1-mesa-dev libfreetype6-dev libz-dev
```

Installing dependencies on **Fedora**:
```sh
sudo dnf install libX11-devel alsa-lib-devel mesa-libGL-devel freetype-devel zlib-devel
```

### All wrongs reserved

![GPLv3](https://www.gnu.org/graphics/gplv3-88x31.png) [![CC-BY-SA](https://i.creativecommons.org/l/by-sa/4.0/88x31.png)](http://creativecommons.org/licenses/by-sa/4.0/)

The code is licensed under **GPL version 3**.
Every asset distributed alongside the software (*id est* images, music, etc.) is licensed under [Creative Commons Attribution-ShareAlike 4.0 International License](http://creativecommons.org/licenses/by-sa/4.0/).


### Android

In order to build for android, add these options to your gradle configuration:

```groovy
android {
    ...

    defaultConfig {
        minSdkVersion 14
        ...
        externalNativeBuild {
            cmake {
                arguments '-DANDROID_STL=c++_static',
                        '-DMAKESHIFT_UNITY=ON',
                        '-DFREETYPE_SOURCE_LOCATION=<Path to freetype2 source>'
            }
        }
    }
    ...
    externalNativeBuild {
        cmake {
            version '3.10.2'
            path '<Path to idle/src/CMakeLists.txt>'
        }
    }
}
```

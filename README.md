# About

This is an example showing how to integrate 
[MyyOpenGLHelpers](https://github.com/Miouyouyou/MyyOpenGLHelpers) with 
[CMake](http://cmake.org).

The [CMakeLists.txt](./CMakeLists.txt) is handcrafted and should be 
rather easy to understand and customise to various needs.

Note that the Android build is handled automatically through the apk
folder. See the [README in the apk folder](./apk) for informations about
how to build and run the the Android version, and the requirements for
these operations.

# Requirements

## Common requirements

To build this example, you will need, at least:

* CMake version 2.8 or higher
* OpenGL ES 3.x headers
* OpenGL ES 2.x libraries (e.g. : libGLESv2 on *Nix systems)
* EGL headers and libraries

To build the X11 variant, you will also need:

* X11 libraries and headers

To build the DRM variant, you will need:

* libdrm (headers and libraries)
* gbm (headers and libraries)

To build the Android APK, consult the README in the apk folder

# Building

## X11 variant

Clone this repository somewhere and then do:

```bash
cd /tmp
mkdir build_dir
cd build_dir
cmake /path/to/project/sources -DMYY_X11=ON
make
```

Then you can run `Program`.

## DRM variant

```bash
cd /tmp
mkdir build_dir
cd build_dir
cmake /path/to/project/sources -DMYY_DRM=ON
make
```


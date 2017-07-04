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
* OpenGL ES 2.0 headers
* OpenGL ES 2.0 libraries (e.g. : libGLESv2 on \*Nix systems)
* EGL headers and libraries

# Building

## X11 variant

To build the X11 variant, you will also need:

* X11 libraries and headers if you want to compile the X11 variant

Clone this repository somewhere and then do:

```bash
cd /tmp
mkdir build_dir
cd build_dir
cmake /path/to/the/cloned/MyyOpenGLHelpers-example-cmake -DMYY_X11=ON
make
```

## DRM variant

To build the DRM variant, you will also need:

* Generic Buffer Management (GBM) libraries and headers
* Direct Rendering Manager (DRM) libraries, headers, kernel drivers
  and OpenGL drivers

```bash
cd /tmp
mkdir build_dir
cd build_dir
cmake /path/to/the/cloned/MyyOpenGLHelpers-example-cmake -DMYY_DRM=ON
make
```

## Android variant

To build the Android variant, see the README.md in the
[Android folder](./apk)

# Running

Run `Program_X11` to run the X11 version, `Program_DRM` to run the
KMS/DRM version. You'll need a mouse and keyboard plugged in, and
accessible using libevdev if you intend to use the DRM/KMS version.


# vision-lib
Automatically exported from code.google.com/p/vision-lib

vision-lib

Libor's Vision Library: Fast Extensible C Library for Early Computer Vision

LVL: Libor's Vision Library

Small, fast, extensible C library for early computer vision. Not using opencv. Can be used with the Low Level Virtual Machine (LLVM) and its clang project, or any other C compiler. The emphasis is on edge detection and the objectives are generality, speed and accuracy.

Installation from Source

Note that while it is possible to install precompiled binaries, the full value of this package is obtained by writing your own programs and linking them to the LVL library. Thus the ability to compile sources is very useful in this case. CMake makes it easy on any platform and all its necessary files are provided in the LVL source release.

Required * Any C compiler * CMake

Recommended * An image display utility that can be invoked from the command line (eg. ImageMagick's display) * An image formats conversion utility to convert between .jpg .png etc. and .ppm , (eg. XnView on Windows)

Equivalent facilities are commonly available on most platforms.

Documentation: for full details, see: Manual(pdf)

Author: Dr Libor Spacek, Centre for Machine Perception, Czech Technical University in Prague.
Email: spacelib AT fel.cvut.cz, url: http://cmp.felk.cvut.cz/~spacelib'>http://cmp.felk.cvut.cz/~spacelib


# **LVL: Libor's Vision Library** #
Small, fast, extensible _C_ library for early computer vision. Not using opencv.
Can be used with the Low Level Virtual Machine (LLVM) and its clang project, or any other
_C_ compiler. The emphasis is on edge detection and the objectives are generality, speed and accuracy.

---

**Installation from Source**

Note that while it is possible to install precompiled binaries, the full value of this package is obtained by writing your own programs and linking them to the LVL library. Thus the ability to compile sources is very useful in this case. CMake makes it easy on any platform and all its necessary files are provided in the LVL source release.

**Required**
  * Any _C_ compiler
  * [CMake](http://www.cmake.org/)

**Recommended**
  * An image display utility that can be invoked from the command line (eg. _[ImageMagick's](http://www.imagemagick.org/script/index.php) display_)
  * An image formats conversion utility to convert between .jpg .png etc. and .ppm , (eg. [XnView](http://www.xnview.com/) on Windows)

Equivalent facilities are commonly available on most platforms.

**Documentation:** for full details, see: [Manual(pdf)](http://vision-lib.googlecode.com/files/lvl.pdf)

---

**Author:** Dr Libor Spacek,
Centre for Machine Perception, Czech Technical University in Prague.<br>
Email: spacelib AT fel.cvut.cz, url: <a href='http://cmp.felk.cvut.cz/~spacelib'>http://cmp.felk.cvut.cz/~spacelib</a>
<hr />
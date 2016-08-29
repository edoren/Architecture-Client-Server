## Introduction

This is the repository for the Architecture Client / Server course.

| **`Build Status`** |
----------------------
| [![Build Status](https://travis-ci.org/edoren/ArchitectureClientServer.svg?branch=master)](https://travis-ci.org/edoren/ArchitectureClientServer) |

## Download
To download this repository and its submodules you must use Git and clone it
with:

**Via HTTPS**
```
git clone --recursive https://github.com/edoren/ArchitectureClientServer.git
```

**Via SSH**
```
git clone --recursive git@github.com:edoren/ArchitectureClientServer.git
```

## Build
This repository use CMake as build system, as well as a C++11 compatible
compiler, so be sure to have those installed.

After all the instructions you should find the task binaries in the `bin`
folder located in the place you execute CMake.

### Linux And OS X
Go to the project directory and execute:

```
mkdir build
cd build
cmake ..
make
```

### Windows
On Windows you could use any MinGW based compiler like [MinGW-w64]
(https://sourceforge.net/projects/mingw-w64) or [TDM-GCC]
(http://tdm-gcc.tdragon.net/) to build the CMake project, right
now Visual Studio is not supported.

Make sure CMake and MinGW are in the PATH and open a CMD in the project
directory and execute:

```
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
```

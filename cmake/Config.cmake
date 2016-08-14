# Version 1.0

# Configuration module that set some useful variables to detect OS, compiler
# and compiler architecture

# It defines the following variables:
#  OS => Windows - Microsoft Windows
#        OSX     - Mac OSX
#        Linux   - Linux OS
#        FreeBSD - FreeBSD OS
#  COMPILER => MSVC  - Microsoft Visual C++
#              Clang - Clang compiler
#              GCC   - GNU Compiler Collection
#  COMPILER_ARCH => It could be 64 or 32

#*****************************************************************************#
#
# MIT License
#
# Copyright (c) 2016 Manuel Sabogal
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
#*****************************************************************************#

if (CMAKE_PROJECT_NAME STREQUAL "")
    message(FATAL_ERROR
            "Config.cmake must be imported before 'project' command.")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(OS "Windows")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(OS "OSX")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(OS "Linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    set(OS "FreeBSD")
else()
    message(FATAL_ERROR "Unsupported operating system.")
endif()

if(MSVC)
    set(COMPILER "MSVC")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(COMPILER "Clang")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(COMPILER "GCC")
else()
    message(FATAL_ERROR "Unsupported compiler.")
endif()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(COMPILER_ARCH 64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(COMPILER_ARCH 32)
else()
    message(FATAL_ERROR "Unsupported compiler architecture.")
endif()

c-open: CANopen stack
=====================
[![Build Status](https://github.com/rtlabs-com/c-open/workflows/build/badge.svg?branch=master)](https://github.com/rtlabs-com/c-open/actions?workflow=build)

This repository contains a CANopen stack for both master and
slaves. The stack implements most of CiA 301 and 305 (LSS). The stack
is written to an OS abstraction layer and can also be used in a bare
metal application. Using the abstraction layer, the stack can run on
Linux, Windows or on an RTOS.

A simple slave is included to serve as an example of how to use the
stack. The slave can also be used to run the CiA Conformance Test
Tool.

Also included is a simple master example that lists all slaves on the
bus and a comprehensive set of unit-tests.

Cloning
=======

Clone the source:

```
$ git clone --recurse-submodules https://github.com/rtlabs-com/c-open.git
```

This will clone the repository with submodules. If you already cloned
the repository without the `--recurse-submodules` flag then run this
in the c-open folder:

```
$ git submodule update --init --recursive
```

Prerequisites for all platforms
===============================

 * CMake 3.14 or later

Windows
=======

 * Visual Studio 2017 or later
 * Kvaser CANlib SDK

You can use a windows or unix shell as preferred. The following
instructions are for a unix shell. CMake is assumed to be in your
path.

The windows build supports Kvaser devices and requires the Kvaser
CANlib SDK. CMake should find the SDK but if not a hint can be given
by setting `-DCANLIB_ROOT_DIR="C:\Program Files (x86)\Kvaser\Canlib"`
or similar during configuration.

```
$ cmake -B build.win32 -A Win32
$ cmake --build build.win32 --config Release
$ cmake --build build.win32 --config Release --target check
```

This builds the project and runs the unit tests.

Linux
=====

 * GCC 4.6 or later

```
$ cmake -B build
$ cmake --build build --target all check
```

This builds the project and runs the unit tests.

rt-kernel
=========

 * Workbench 2020.1 or later

You should use a bash shell, such as for instance the Command Line in
your Toolbox installation. Set the BSP variable to the name of the BSP
you wish to build for. Set the RTK variable to the path of your
rt-kernel tree.

Standalone project
------------------

This creates standalone makefiles.

```
$ RTK=/path/to/rt-kernel BSP=xmc48relax cmake \
   -B build.xmc48relax \
   -DCMAKE_TOOLCHAIN_FILE=cmake/tools/toolchain/rt-kernel.cmake \
   -G "Unix Makefiles"
$ cmake --build build.xmc48relax
```

Workbench project
-----------------

This creates a Makefile project that can be imported to Workbench. The
project will be created in the build directory. The build directory
should be located outside of the source tree.

```
$ RTK=/path/to/rt-kernel BSP=xmc48relax cmake \
   -B build.xmc48relax -S /path/to/c-open \
   -DCMAKE_TOOLCHAIN_FILE=cmake/tools/toolchain/rt-kernel.cmake \
   -DCMAKE_ECLIPSE_EXECUTABLE=/opt/rt-tools/workbench/Workbench \
   -DCMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT=TRUE \
   -G "Eclipse CDT4 - Unix Makefiles"
```

A source project will also be created in the c-open tree. This project
can also be imported to Workbench. After importing, right-click on the
project and choose *New* -> *Convert to a C/C++ project*. This will
setup the project so that the indexer works correctly and the
Workbench revision control tools can be used.

The library and the unit tests will be built. Note that the tests
require a stack of at least 6 kB. You may have to increase
CFG_MAIN_STACK_SIZE in your bsp include/config.h file.

Contributions
=============

Contributions are welcome. If you want to contribute you will need to
sign a Contributor License Agreement and send it to us either by
e-mail or by physical mail. More information is available
[here](https://rt-labs.com/contribution).

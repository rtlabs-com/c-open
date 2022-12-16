.. _getting-started:

Getting started with C-Open
===========================

In this tutorial you will learn how to clone and build the
C-Open software on a linux system.

Verifying the development tools
-------------------------------
Run the commands listed below to verify that required tools are installed in your development environment.

::

  git --version
  cmake --version

Cloning the repository
----------------------

#. Clone the source::

       git clone --recurse-submodules https://github.com/rtlabs-com/c-open.git

   This will clone the repository with submodules. If you already cloned the
   repository without the ``--recurse-submodules`` flag then run this in the
   c-open folder::

       git submodule update --init --recursive

#. Change to the new directory using ``cd c-open``.

Building for Linux
------------------

#. Use the following commands to build and run the unit tests::

      cmake -B build
      cmake --build build --target all check

Building for Unix
------------------

You can use a windows or unix shell as preferred. The following instructions
are for a unix shell.

The windows build supports Kvaser devices and requires the Kvaser CANlib SDK.
CMake should find the SDK but if not a hint can be given by setting
``-DCANLIB_ROOT_DIR="C:\Program Files (x86)\Kvaser\Canlib"`` or similar during
configuration.

#. Run the following commands to build and run the unit tests::

    $ cmake -B build.win32 -A Win32
    $ cmake --build build.win32 --config Release
    $ cmake --build build.win32 --config Release --target check

Building for RT-Kernel
----------------------
You should use a bash shell, such as for instance the Command Line in your
Toolbox installation. Set the BSP variable to the name of the BSP you wish to
build for. Set the RTK variable to the path of your rt-kernel tree.

Standalone project
^^^^^^^^^^^^^^^^^^

#. The following creates standalone makefiles::

    $ RTK=/path/to/rt-kernel BSP=xmc48relax cmake \
       -B build.xmc48relax \
       -DCMAKE_TOOLCHAIN_FILE=cmake/tools/toolchain/rt-kernel.cmake \
       -G "Unix Makefiles"
    $ cmake --build build.xmc48relax

Workbench project
^^^^^^^^^^^^^^^^^

#. Run this to create a Makefile project that can be imported to Workbench. The
   project will be created in the build directory. The build directory should
   be located outside of the source tree::

     $ RTK=/path/to/rt-kernel BSP=xmc48relax cmake \
        -B build.xmc48relax -S /path/to/c-open \
        -DCMAKE_TOOLCHAIN_FILE=cmake/tools/toolchain/rt-kernel.cmake \
        -DCMAKE_ECLIPSE_EXECUTABLE=/opt/rt-tools/workbench/Workbench \
        -DCMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT=TRUE \
        -G "Eclipse CDT4 - Unix Makefiles"

   A source project will also be created in the c-open tree. This project can
   also be imported to Workbench.

#. After importing, right-click on the project and choose ``New -> Convert
   to a C/C++ project``. This will setup the project so that the indexer works
   correctly and the Workbench revision control tools can be used.

The library and the unit tests will be built. Note that the tests require a
stack of at least 6 kB. You may have to increase ``CFG_MAIN_STACK_SIZE`` in
your bsp ``include/config.h`` file.

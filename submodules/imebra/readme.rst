Imebra V5
*********

Introduction
============

Imebra is a C++ DICOM library able to:

- parse & build DICOM files
- decompress/compress/process the embedded DICOM images
- send and receive DIMSE commands and responses as an SCU or SCP

A JNI interface allows to use the library also with the JAVA language, including on Android OS.

Objective-C wrappers (compatible with Swift) are also available.


Creating the source distribution from the code in the VCS
=========================================================

If you downloaded the source distribution then you can skip this part and jump directly to "Compiling Imebra".

This section describes how to create an Imebra Source Distribution containing the full
C++source code, the Java wrappers for Android, the HTML documentation, the test units code coverage
information.

The Imebra image is built on Ubuntu 20.04 with the following apt packages:

- *git* to clone the repository
- *doxygen* to generate documentation
- *g++* for the compilation of the library
- *lcov* for code coverage
- *python3* for some tools (get version, generate documentation)
- *python3-dev* 
- *python3-pip*
- *ant* to generate the source distribution
- *cmake* to build the c++ and objective-c versions of the library
- *swig* to produce java and python bridges
- *liblog4cxx-dev* to test logging features
- *libdcmtk-dev* for interoperability testing
- *default-jdk* for Java testing
- *junit4* for Java testing
- *dcmtk* for interoperability testing

Additionally, 2 Python packages must be present (installable with pip):

- *breathe* to generate the documentation
- *sphinx_rtd_theme* the documentation theme

In order to install the packages, type the following commands as root or prepend them with sudo:
::

  apt-get update && \
  apt-get install -y \
    tzdata \
    git \
    doxygen \
    g++ \
    lcov \
    python3 \
    python3-dev \
    python3-pip \
    ant \
    cmake \
    swig \
    liblog4cxx-dev \
    libdcmtk-dev \
    default-jdk \
    junit4 \
    dcmtk && \
  pip3 install Sphinx==2.4.4 && \
  pip3 install breathe==4.14.2 && \
  pip3 install sphinx_rtd_theme

Then clone the Imebra git repository:
::

    git clone git@bitbucket.org:binarno/imebra_git.git

Finally, cd into the imebra folder and execute ant to build the source distribution that includes:

- full C++ source code
- generated Java source code for the Android wrapper
- html documentation
- unit tests
- code coverage report

::

    cd imebra
    ant


Versioning
==========

Public releases of Imebra V5 are versioned according to the rules defined in `Semantic versioning <http://semver.org/>`_.

Nightly releases are versioned as public releases but their minor version or patch number are set to 999 (the master branch
or the major version branches have a minor version set to 999, the minor version branches have a patch number set to 999).

For instance:

- version 5.1.999.45 indicates a nightly build of version 5.1, build from the commit number 45 in the branch.
- version 5.999.999.678 indicates a nightly build of version 5 (master branch), build from the commit number 678 in the branch.


Compiling Imebra
================

This section explains how to compile Imebra from the source distribution.
You can download the source distribution from https://imebra.com, or you can create a source distribution
by following the instructions at "Creating the source distribution from the code in the VCS"

The result of the Imebra compilation is a shared library for your operating system of choice.

The Imebra Source Distribution includes:

- the source code for the library
- the source code for the tests
- pre-built documentation
- pre-generated JNI headers for Java


Compiling the C++ version of Imebra
-----------------------------------

Prerequisites
.............

In order to build the library from the Imebra Source Distribution you need:

- a modern C++ compiler (GCC, clang, Visual Studio, etc)
- CMake version 2.8 or newer (https://cmake.org/)

Building Imebra
...............

The library folder contains a CMakeLists file, which contains the information needed by
CMake to generate a solution file for your platform (a make file, a VisualStudio solution, etc).

To generate the Imebra shared library, execute the following steps:

1. Create a folder that will contain the result of the compilation (artifacts)
2. cd into the created artifacts folder
3. execute cmake with the path to the Imebra's library folder as parameter
4. execute cmake with the --build option, and on Windows optionally specify the build configuration

For instance:

::

    md artifacts
    cd artifacts
    cmake imebra_location/library
    cmake --build .

The first CMake command will generate a solution file for the your platform: this will be a 
make file on Linux, a VisualStudio solution of Windows, an XCode project on Mac.

The second CMake command with the --build option will launch make, VisualStudio or the build
chain defined on your machine.

On Windows, the last cmake command can be followed by --config release or --config debug, like
this:

::

    cmake --build . --config release

or

::

    cmake --build . --config debug



Compiling the Android version of Imebra
---------------------------------------

Prerequisites
.............

In order to build the Android version of Imebra you need:

- Apache Ant
- the Android SDK
- the Android NDK

Building Imebra
...............

The Android version of the library needs both the Java source code (located in the wrappers/javaWrapper folder)
and the C++ source code (located in the library folder)

To generate the Imebra Jar library:

- Launch Android Studio
- Open the gradle project in the wrappers/javaWrapper folder
- Build the project

    




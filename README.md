# VolData

Small framwork for loading, management and conversion of volume grid data, supporting OpenVDB, NanoVDB, DICOM and dense or sparse serialized grid data. Volume animations, stored as discretized grids per frame on disk, are also supported.

# Submodules

Either clone the repository using `--recursive` flag, or run `git submodule update --init` in the root folder to pull the submodules after cloning.

# Dependencies (Ubuntu)

    apt-get install -y build-essential cmake
    apt-get install libopenvdb-dev # for OpenVDB support

# Build

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -Wno-dev && cmake --build build --parallel

# Usage

See `grid.h` and `volume.h` for the general interface and the `tools/` directory for examples.

# Licence

This library is under MIT license and freely usable, however note that the [Imebra](https://imebra.com/) submodule used for loading DICOM grids is licenced under GPL.

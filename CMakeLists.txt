project(voldata LANGUAGES C CXX CUDA)
cmake_minimum_required(VERSION 3.10)

# ---------------------------------------------------------------------
# path management

# output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")

# ---------------------------------------------------------------------
# options

# ---------------------------------------------------------------------
# submodules

#add_subdirectory(submodules/cppgl)
#add_subdirectory(submodules/pybind11)

set(OPENVDB_BUILD_BINARIES OFF CACHE BOOL "disable openvdb binaries" FORCE)
#set(OPENVDB_BUILD_PYTHON_MODULE ON CACHE BOOL "enable openvdb python module" FORCE)
add_subdirectory(submodules/openvdb)

include_directories(submodules)
#include_directories(submodules/cppgl/src)
#include_directories(submodules/openvdb/nanovdb)

# ---------------------------------------------------------------------
# compiler flags

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-pthread -fopenmp -march=native)
add_compile_options(-Wall -Wno-unused-parameter -Wno-unused-variable -Wno-deprecated-declarations)

# ---------------------------------------------------------------------
# install shaders

#install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/shader DESTINATION bin)

# ---------------------------------------------------------------------
# traverse source tree

include_directories(src)
add_subdirectory(src)

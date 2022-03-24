cmake_minimum_required(VERSION 3.16)

PROJECT(voldata
    VERSION 0.2
    DESCRIPTION "3D volume loading/management framework."
    LANGUAGES C CXX
)

# ---------------------------------------------------------------------
# options

option(VOLDATA_BUILD_TOOLS "Build voldata tools." ON)

# ---------------------------------------------------------------------
# compiler setup

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	message(STATUS "No release type specified. Setting to 'Release'.")
	set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------------------
# dependencies

include_directories(submodules)

# cereal
include_directories(submodules/cereal/include)

# openvdb (prefer system version)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/submodules/openvdb/cmake")
find_package(OpenVDB QUIET)
if (NOT OpenVDB_FOUND)
    set(OPENVDB_BUILD_BINARIES OFF CACHE BOOL "" FORCE)
    set(OPENVDB_ENABLE_UNINSTALL OFF CACHE BOOL "" FORCE)
    set(OPENVDB_CORE_SHARED OFF CACHE BOOL "" FORCE)
    set(OPENVDB_CORE_STATIC ON CACHE BOOL "" FORCE)
    add_subdirectory(submodules/openvdb)
    include_directories(submodules/openvdb)
else()
    include_directories(${OpenVDB_INCLUDE_DIR})
endif()

# nanovdb
include_directories(submodules/openvdb/nanovdb)

# imebra
set(IMEBRA_SHARED_STATIC STATIC CACHE BOOL "" FORCE)
add_subdirectory(submodules/imebra)
include_directories(submodules/imebra/library/include)

# ---------------------------------------------------------------------
# directory management

include_directories(include)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# ---------------------------------------------------------------------
# traverse source tree

add_subdirectory(src)

if (VOLDATA_BUILD_TOOLS)
    add_subdirectory(tools)
endif()

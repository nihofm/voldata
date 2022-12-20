cmake_minimum_required(VERSION 3.16)

PROJECT(voldata
    VERSION 0.2
    DESCRIPTION "3D volume loading/management framework."
    LANGUAGES C CXX
)

# ---------------------------------------------------------------------
# options

option(VOLDATA_BUILD_GLM ON)
option(VOLDATA_BUILD_OPENVDB ON)
option(VOLDATA_BUILD_TOOLS OFF)

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

# glm
if (VOLDATA_BUILD_GLM)
    add_subdirectory(submodules/glm)
    include_directories(submodules/glm)
endif()

# cereal
include_directories(submodules/cereal/include)

# openvdb
if (VOLDATA_BUILD_OPENVDB)
    set(OPENVDB_BUILD_BINARIES OFF CACHE BOOL "" FORCE)
    set(OPENVDB_ENABLE_UNINSTALL OFF CACHE BOOL "" FORCE)
    set(OPENVDB_CORE_SHARED OFF CACHE BOOL "" FORCE)
    set(OPENVDB_CORE_STATIC ON CACHE BOOL "" FORCE)
    add_subdirectory(submodules/openvdb)
    include_directories(submodules/openvdb)
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

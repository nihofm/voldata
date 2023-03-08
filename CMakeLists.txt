cmake_minimum_required(VERSION 3.16)

PROJECT(voldata
    VERSION 0.2
    DESCRIPTION "3D volume loading/management framework."
    LANGUAGES C CXX
)

# ---------------------------------------------------------------------
# options

option(VOLDATA_BUILD_GLM "" ON)
option(VOLDATA_BUILD_TOOLS "" ON)

# ---------------------------------------------------------------------
# compiler setup

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	message(STATUS "No release type specified. Setting to 'Release'.")
	set(CMAKE_BUILD_TYPE Release)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------
# dependencies

include_directories(submodules)

# glm
if (VOLDATA_BUILD_GLM)
    add_subdirectory(submodules/glm)
    include_directories(submodules/glm)
endif()

# imebra
set(IMEBRA_SHARED_STATIC STATIC)
add_subdirectory(submodules/imebra)
include_directories(submodules/imebra/library/include)

# cereal
include_directories(submodules/cereal/include)

# openvdb
list(APPEND CMAKE_MODULE_PATH "/usr/local/lib/cmake/OpenVDB") # support default non-standard OpenVDB install location
find_package(OpenVDB)
if (OpenVDB_FOUND)
    message(STATUS "Found OpenVDB version: ${OpenVDB_VERSION}, compiling with OpenVDB support...")
    set(VOLDATA_WITH_OPENVDB ON)
    add_definitions(-DVOLDATA_WITH_OPENVDB)
    include_directories(${OpenVDB_INCLUDE_DIRS})
else()
    message(STATUS "OpenVDB NOT found, compiling without OpenVDB support...")
    add_definitions(-UVOLDATA_WITH_OPENVDB)
endif()

# ---------------------------------------------------------------------
# directory management

include_directories(src)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# ---------------------------------------------------------------------
# compile main library

file(GLOB SOURCES "src/*.cpp")
add_library(voldata SHARED ${SOURCES})
target_link_libraries(voldata stdc++fs imebra tbb)

if (VOLDATA_WITH_OPENVDB)
    target_link_libraries(voldata ${OpenVDB_LIBRARIES})
endif()

# ---------------------------------------------------------------------
# optionally compile tools

if (VOLDATA_BUILD_TOOLS)
    # simple executable to print grid info
    add_executable(voldata_print tools/print.cpp)
    target_link_libraries(voldata_print voldata)
    # simple executable to load, convert and serialize grid formats
    add_executable(voldata_serialize tools/serialize.cpp)
    target_link_libraries(voldata_serialize voldata)
endif()

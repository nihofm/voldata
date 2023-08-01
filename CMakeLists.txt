cmake_minimum_required(VERSION 3.16)

PROJECT(voldata
    VERSION 0.4
    DESCRIPTION "Small framework to load/manage/convert volumetric grid data."
    LANGUAGES C CXX
)

# ---------------------------------------------------------------------
# options

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

# imebra
set(IMEBRA_SHARED_STATIC STATIC)
add_subdirectory(submodules/imebra)

# json11
add_subdirectory(submodules/json11)

# ---------------------------------------------------------------------
# directory management

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# ---------------------------------------------------------------------
# compile main library

file(GLOB SOURCES "src/*.cpp")
add_library(voldata SHARED ${SOURCES})

# includes
target_include_directories(voldata PUBLIC src)
target_include_directories(voldata PUBLIC submodules)
target_include_directories(voldata PUBLIC submodules/glm)
target_include_directories(voldata PRIVATE submodules/cereal/include)
target_include_directories(voldata PRIVATE submodules/imebra/library/include)

# linking
target_link_libraries(voldata stdc++fs imebra tbb json11)

# try to find openvdb
list(APPEND CMAKE_MODULE_PATH "/usr/local/lib/cmake/OpenVDB") # support default non-standard OpenVDB install location
find_package(OpenVDB)
if (OpenVDB_FOUND)
    message(STATUS "Found OpenVDB version: ${OpenVDB_VERSION}, compiling with OpenVDB support...")
    add_definitions(-DVOLDATA_WITH_OPENVDB)
    target_include_directories(voldata PRIVATE ${OpenVDB_INCLUDE_DIRS})
    target_link_libraries(voldata ${OpenVDB_LIBRARIES})
else()
    message(STATUS "OpenVDB NOT found, compiling without OpenVDB support...")
    add_definitions(-UVOLDATA_WITH_OPENVDB)
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

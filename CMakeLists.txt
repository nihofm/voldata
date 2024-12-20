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
target_compile_options(imebra PRIVATE -Wno-catch-value)
target_compile_options(imebra PRIVATE -Wno-ignored-qualifiers)
target_compile_options(imebra PRIVATE -Wno-range-loop-construct)

# json11
add_subdirectory(submodules/json11)

# openvdb
add_subdirectory(submodules/openvdb)

# ---------------------------------------------------------------------
# directory management

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# ---------------------------------------------------------------------
# compile main library

file(GLOB SOURCES "src/*.cpp")
add_library(voldata STATIC ${SOURCES})

# includes
target_include_directories(voldata PUBLIC src)
target_include_directories(voldata PUBLIC submodules)
target_include_directories(voldata PUBLIC submodules/cereal/include)
target_include_directories(voldata PUBLIC submodules/glm)
target_include_directories(voldata PUBLIC submodules/imebra/library/include)
target_include_directories(voldata PUBLIC submodules/openvdb/openvdb)
target_include_directories(voldata PUBLIC submodules/openvdb/nanovdb)

# linking
target_link_libraries(voldata PUBLIC stdc++fs imebra tbb json11 openvdb)

# ---------------------------------------------------------------------
# optionally compile tools

if (VOLDATA_BUILD_TOOLS)
    # simple executable to print grid info
    add_executable(voldata_print tools/print.cpp)
    target_link_libraries(voldata_print PRIVATE voldata)
    # simple executable to load, convert and serialize grid formats
    add_executable(voldata_serialize tools/serialize.cpp)
    target_link_libraries(voldata_serialize PRIVATE voldata)
endif()

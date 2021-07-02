# target name to generate here
set(TARGET voldata)
#set(TARGET_PY volpy)

# glob source files
file(GLOB HEADERS "*.h")
file(GLOB SOURCES "*.cpp")

# define targets
add_library(${TARGET} SHARED ${SOURCES} ${HEADERS})
#generate_export_header(${TARGET}) # TODO win support

#pybind11_add_module(${TARGET_PY} ${SOURCES})
#pybind11_strip(${TARGET_PY})
#pybind11_extension(${TARGET_PY})
#set_target_properties(${TARGET_PY} PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")

# install
install(TARGETS ${TARGET} DESTINATION bin)
install(DIRECTORY . DESTINATION include/${TARGET} FILES_MATCHING PATTERN "*.h")

# ----------------------------------------------------------
# dependencies

target_link_libraries(${TARGET} stdc++fs)
#target_link_libraries(${TARGET} cppgl)
#target_link_libraries(${TARGET_PY} PRIVATE cppgl stdc++fs)

# CUDA
#find_package(CUDA REQUIRED)
#include_directories(${CUDA_INCLUDE_DIRS})
#target_link_libraries(${TARGET} ${CUDA_LIBRARIES})
#target_link_libraries(${TARGET_PY} PRIVATE ${CUDA_LIBRARIES})

# pybind11
#find_package(Python3 COMPONENTS Development REQUIRED)
#target_include_directories(${TARGET} PUBLIC ${Python3_INCLUDE_DIRS})
#target_link_libraries(${TARGET} ${Python3_LIBRARIES} pybind11::embed)

# openvdb
target_link_libraries(${TARGET} openvdb)
#target_link_libraries(${TARGET_PY} PRIVATE openvdb)

# imebra
target_link_libraries(${TARGET} imebra)

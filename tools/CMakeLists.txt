# simple executable to print grid info
add_executable(voldata_print print.cpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_link_libraries(voldata_print voldata)

# simple executable to load, convert and store to dense grid
add_executable(voldata_serialize serialize.cpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
target_link_libraries(voldata_serialize voldata)

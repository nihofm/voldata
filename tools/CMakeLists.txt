# simple executable to print grid info
add_executable(voldata_print print.cpp)
target_link_libraries(voldata_print voldata)

# simple executable to load, convert and store other grid formats
add_executable(voldata_serialize serialize.cpp)
target_link_libraries(voldata_serialize voldata)
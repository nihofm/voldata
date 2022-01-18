file(GLOB SOURCES "*.cpp")

add_library(voldata SHARED ${SOURCES})
target_link_libraries(voldata stdc++fs openvdb imebra)

file(GLOB SOURCES "*.cpp")

add_library(voldata SHARED ${SOURCES})
target_link_libraries(voldata stdc++fs imebra tbb)

if (VOLDATA_WITH_OPENVDB)
    target_link_libraries(voldata ${OpenVDB_LIBRARIES})
endif()

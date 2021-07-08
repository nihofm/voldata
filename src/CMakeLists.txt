# target name to generate here
set(TARGET voldata)

# glob source files
file(GLOB HEADERS "*.h")
file(GLOB SOURCES "*.cpp")

# define targets
add_library(${TARGET} SHARED ${SOURCES} ${HEADERS})
#generate_export_header(${TARGET}) # TODO win support

# install
install(TARGETS ${TARGET} DESTINATION lib)
install(DIRECTORY . DESTINATION include/${TARGET} FILES_MATCHING PATTERN "*.h")

# ----------------------------------------------------------
# dependencies

target_link_libraries(${TARGET} stdc++fs openvdb imebra)

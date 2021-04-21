# target name to generate here
set(TARGET voldata_print)

# glob source files
file(GLOB_RECURSE HEADERS "*.h")
file(GLOB_RECURSE SOURCES "*.cpp")

# compile executable
add_executable(${TARGET} ${HEADERS} ${SOURCES})
target_link_libraries(${TARGET} voldata)

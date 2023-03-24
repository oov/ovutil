cmake_minimum_required(VERSION 3.13)
project(ovutil C)
enable_language(RC)
enable_testing()

file(GLOB sources "${PROJECT_SOURCE_DIR}/src/*.c" "${PROJECT_SOURCE_DIR}/src/*/*.c" "${PROJECT_SOURCE_DIR}/include/*.h")
find_program(CLANG_FORMAT_EXE clang-format)
add_custom_target(${PROJECT_NAME}-format ALL
  COMMAND ${CLANG_FORMAT_EXE} -style=file -i ${sources}
)

set(OVBASE_PATH "" CACHE STRING "path to ovbase project root")
if (NOT "${OVBASE_PATH}" STREQUAL "")
  add_subdirectory("${OVBASE_PATH}" "${CMAKE_BINARY_DIR}/ovbase")
endif()

add_subdirectory(src)

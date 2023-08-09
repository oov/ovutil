cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(FORMAT_SOURCES "execute clang-format" ON)

project(ovutil C)
if(WIN32)
  enable_language(RC)
endif()
enable_testing()

if(FORMAT_SOURCES)
  file(GLOB sources "${PROJECT_SOURCE_DIR}/src/*.c" "${PROJECT_SOURCE_DIR}/src/*/*.c" "${PROJECT_SOURCE_DIR}/include/ovutil/*.h")
  find_program(CLANG_FORMAT_EXE clang-format)
  add_custom_target(${PROJECT_NAME}-format ALL
    COMMAND ${CLANG_FORMAT_EXE} -style=file -i ${sources}
  )
endif()

set(OVBASE_PATH "" CACHE STRING "path to ovbase project root")
if (NOT "${OVBASE_PATH}" STREQUAL "")
  add_subdirectory("${OVBASE_PATH}" "${CMAKE_BINARY_DIR}/ovbase")
endif()

add_subdirectory(src)

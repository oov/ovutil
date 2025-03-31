option(USE_ADDRESS_SANITIZER "use address sanitizer" OFF)
option(USE_COMPILER_RT "use compiler-rt runtime" OFF)
option(USE_NO_PTHREAD "add -no-pthread" OFF)
option(USE_LTO "use LTO" ON)
option(TARGET_EMSCRIPTEN "target emscripten" OFF)
option(TARGET_WASI_SDK "target wasi-sdk" OFF)
set(LDNAME "lld" CACHE STRING "ld name")

set(is_clang "$<C_COMPILER_ID:Clang>")
set(v16_or_later "$<VERSION_GREATER_EQUAL:$<C_COMPILER_VERSION>,16>")
set(v18_or_later "$<VERSION_GREATER_EQUAL:$<C_COMPILER_VERSION>,18>")
set(v19_or_later "$<VERSION_GREATER_EQUAL:$<C_COMPILER_VERSION>,19>")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

include(FetchContent)
if (NOT TARGET ovbase) # if ovbase is not added in the parent project
  find_package(ovbase QUIET)
  if (NOT ovbase_FOUND) # If ovbase is not installed
    FetchContent_Declare(
      ovbase
      GIT_REPOSITORY https://github.com/oov/ovbase
      GIT_TAG        caace762bea6a59bfe4f9fc51a703099e3f65e9e
    )
    FetchContent_MakeAvailable(ovbase)
  endif()
endif()

add_library(ovutil_intf INTERFACE)
target_compile_options(ovutil_intf
INTERFACE
  -mstackrealign
  -Wall
  -Wextra
  -Werror
  -Weverything
  -Wshadow
  -Werror=return-type
  -pedantic-errors
  -Wno-declaration-after-statement
  -Wno-padded
  $<$<AND:${is_clang},${v19_or_later}>:-Wno-pre-c11-compat>
  $<$<AND:${is_clang},${v18_or_later}>:-Wno-switch-default>
  $<$<AND:${is_clang},${v16_or_later}>:-Wno-unsafe-buffer-usage>
  -ffunction-sections
  -fdata-sections
  $<$<BOOL:${USE_ADDRESS_SANITIZER}>:-fsanitize=address>
  $<$<CONFIG:Debug>:-O0>
  $<$<CONFIG:Release>:-O2>
  $<$<BOOL:${USE_LTO}>:-flto>
)
target_link_options(ovutil_intf
INTERFACE
  $<$<NOT:$<STREQUAL:${LDNAME},>>:-fuse-ld=${LDNAME}>
  -Wl,--gc-sections
  # -Wl,--print-gc-sections
  -static
  $<$<BOOL:${TARGET_WASI_SDK}>:-Wl,--import-memory,--export-memory,--max-memory=67108864>
)

set(SOURCE_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(DESTINATION_INCLUDE_DIR ${PROJECT_BINARY_DIR}/include)
configure_file(${SOURCE_INCLUDE_DIR}/ovutil/parallel.h ${DESTINATION_INCLUDE_DIR}/ovutil/parallel.h COPYONLY)
configure_file(${SOURCE_INCLUDE_DIR}/ovutil/str.h ${DESTINATION_INCLUDE_DIR}/ovutil/str.h COPYONLY)
configure_file(${SOURCE_INCLUDE_DIR}/ovutil/win32.h ${DESTINATION_INCLUDE_DIR}/ovutil/win32.h COPYONLY)

add_library(ovutil STATIC
  parallel/get_number_of_logical_cores.c
  parallel/get_number_of_physical_cores.c
  parallel/ovparallel.c

  str/exclude_trailing_path_delimiter.c
  str/extract_file_extension.c
  str/extract_file_name.c
  str/include_trailing_path_delimiter.c
  str/sanitize.c

  $<$<BOOL:${WIN32}>:win32/create_unique_file.c>
  $<$<BOOL:${WIN32}>:win32/create_unique_temp_file.c>
  $<$<BOOL:${WIN32}>:win32/delete_file.c>
  $<$<BOOL:${WIN32}>:win32/disable_family_windows.c>
  $<$<BOOL:${WIN32}>:win32/encoding.c>
  $<$<BOOL:${WIN32}>:win32/file_contains.c>
  $<$<BOOL:${WIN32}>:win32/file_exists.c>
  $<$<BOOL:${WIN32}>:win32/get_file_attributes.c>
  $<$<BOOL:${WIN32}>:win32/get_long_path_name.c>
  $<$<BOOL:${WIN32}>:win32/get_module_file_name.c>
  $<$<BOOL:${WIN32}>:win32/get_temp_dir.c>
  $<$<BOOL:${WIN32}>:win32/get_window_text.c>
  $<$<BOOL:${WIN32}>:win32/hinstance.c>
  $<$<BOOL:${WIN32}>:win32/is_same_file.c>
  $<$<BOOL:${WIN32}>:win32/message_box.c>
  $<$<BOOL:${WIN32}>:win32/path_relative_path_to.c>
  $<$<BOOL:${WIN32}>:win32/set_client_size.c>
  $<$<BOOL:${WIN32}>:win32/write_file.c>
)
set_target_properties(ovutil PROPERTIES
RUNTIME_OUTPUT_DIRECTORY
  ${PROJECT_BINARY_DIR}/bin
ARCHIVE_OUTPUT_DIRECTORY
  ${PROJECT_BINARY_DIR}/lib
LIBRARY_OUTPUT_DIRECTORY
  ${PROJECT_BINARY_DIR}/lib
)
set_property(TARGET ovutil PROPERTY PUBLIC_HEADER
  ${DESTINATION_INCLUDE_DIR}/ovutil/parallel.h
  ${DESTINATION_INCLUDE_DIR}/ovutil/str.h
  ${DESTINATION_INCLUDE_DIR}/ovutil/win32.h
)
target_include_directories(ovutil
PUBLIC
  $<INSTALL_INTERFACE:include>
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)
target_compile_definitions(ovutil
PRIVATE
  $<$<BOOL:${WIN32}>:_WIN32_WINNT=0x0501>
  $<$<CONFIG:Release>:NDEBUG>
)
target_compile_options(ovutil
PUBLIC
  $<$<AND:$<BOOL:${WIN32}>,$<BOOL:${USE_COMPILER_RT}>>:--rtlib=compiler-rt>
  $<$<BOOL:${TARGET_EMSCRIPTEN}>:-sUSE_PTHREADS>
)
target_link_options(ovutil
PUBLIC
  $<$<AND:$<BOOL:${WIN32}>,$<BOOL:${USE_COMPILER_RT}>>:--rtlib=compiler-rt>
  $<$<AND:$<BOOL:${WIN32}>,$<BOOL:${USE_NO_PTHREAD}>>:-no-pthread>
  $<$<NOT:$<BOOL:${WIN32}>>:-pthread>
  $<$<BOOL:${USE_ADDRESS_SANITIZER}>:-fsanitize=address>
  $<$<BOOL:${TARGET_EMSCRIPTEN}>:-sUSE_PTHREADS -sSTANDALONE_WASM>
PRIVATE
  -static
)
target_link_libraries(ovutil
PRIVATE
  ovbase
  ovutil_intf
)
install(TARGETS ovutil ovutil_intf
        EXPORT ovutil-export
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include/ovutil)
install(EXPORT ovutil-export
        FILE ovutil-config.cmake
        DESTINATION share/cmake/ovutil/
        EXPORT_LINK_INTERFACE_LIBRARIES)
export(EXPORT ovutil-export
       FILE "${CMAKE_BINARY_DIR}/ovutil-config.cmake")

add_executable(test_ovutil_parallel parallel/test.c)
list(APPEND tests test_ovutil_parallel)

add_executable(test_ovutil_str str/test.c)
list(APPEND tests test_ovutil_str)

if(WIN32)
  add_executable(test_ovutil_win32 win32/test.c)
  list(APPEND tests test_ovutil_win32)
endif()

foreach(target ${tests})
  add_test(NAME ${target} COMMAND ${target})
  target_compile_definitions(${target}
  PRIVATE
    SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
  )
  target_link_libraries(${target} PRIVATE
    ovbase
    ovutil
    $<$<BOOL:${WIN32}>:shlwapi>
    ovutil_intf
  )
endforeach(target)

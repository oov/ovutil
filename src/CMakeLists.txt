add_library(ovutil STATIC
  mo/mo.c

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
list(APPEND targets ovutil)

add_executable(test_ovutil_mo mo/test.c)
list(APPEND targets test_ovutil_mo)

add_executable(test_ovutil_parallel parallel/test.c)
list(APPEND targets test_ovutil_parallel)

add_executable(test_ovutil_str str/test.c)
list(APPEND targets test_ovutil_str)

if(WIN32)
  add_executable(test_ovutil_win32 win32/test.c)
  list(APPEND targets test_ovutil_win32)
endif()

foreach(target ${targets})
  if(target MATCHES "^test_")
    add_test(NAME ${target} COMMAND ${target})
    target_compile_definitions(${target} PRIVATE
      SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    )
  endif()
  target_include_directories(${target} PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
  )
  target_compile_definitions(${target} PRIVATE
    $<$<BOOL:${WIN32}>:_WIN32_WINNT=0x0501>
    $<$<CONFIG:Release>:NDEBUG>
  )
  target_compile_options(${target} PRIVATE
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
    -ffunction-sections
    -fdata-sections
    $<$<CONFIG:Debug>:-O0>
    $<$<CONFIG:Release>:-O2>
    -flto
  )
  target_link_options(${target} PRIVATE
    -fuse-ld=lld
    -Wl,--gc-sections
    # -Wl,--print-gc-sections
  )
  target_link_libraries(${target} PRIVATE
    ovbase
    $<$<NOT:$<STREQUAL:${target},ovutil>>:ovutil>
    $<$<BOOL:${WIN32}>:shlwapi>
  )
endforeach(target)

add_library(ovutil STATIC
  str_atoi64.c
  str_atou64.c
  str_exclude_trailing_path_delimiter.c
  str_extract_file_extension.c
  str_extract_file_name.c
  str_include_trailing_path_delimiter.c
  str_sanitize.c
  str_utoa64.c
  $<$<BOOL:${WIN32}>:win32_create_unique_file.c>
  $<$<BOOL:${WIN32}>:win32_create_unique_temp_file.c>
  $<$<BOOL:${WIN32}>:win32_delete_file.c>
  $<$<BOOL:${WIN32}>:win32_disable_family_windows.c>
  $<$<BOOL:${WIN32}>:win32_encoding.c>
  $<$<BOOL:${WIN32}>:win32_file_contains.c>
  $<$<BOOL:${WIN32}>:win32_file_exists.c>
  $<$<BOOL:${WIN32}>:win32_get_file_attributes.c>
  $<$<BOOL:${WIN32}>:win32_get_long_path_name.c>
  $<$<BOOL:${WIN32}>:win32_get_module_file_name.c>
  $<$<BOOL:${WIN32}>:win32_get_temp_dir.c>
  $<$<BOOL:${WIN32}>:win32_get_window_text.c>
  $<$<BOOL:${WIN32}>:win32_hinstance.c>
  $<$<BOOL:${WIN32}>:win32_is_same_file.c>
  $<$<BOOL:${WIN32}>:win32_message_box.c>
  $<$<BOOL:${WIN32}>:win32_path_relative_path_to.c>
  $<$<BOOL:${WIN32}>:win32_set_client_size.c>
  $<$<BOOL:${WIN32}>:win32_write_file.c>
)
list(APPEND targets ovutil)

add_executable(test_ovutil_str str_test.c)
list(APPEND targets test_ovutil_str)

if(WIN32)
  add_executable(test_ovutil_win32 win32_test.c)
  list(APPEND targets test_ovutil_win32)
endif()

foreach(target ${targets})
  if(target MATCHES "^test_")
    add_test(NAME ${target} COMMAND ${target})
  endif()
  target_include_directories(${target} PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
  )
  target_compile_definitions(${target} PRIVATE
    $<$<BOOL:${WIN32}>:_WIN32_WINNT=0x0502>
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

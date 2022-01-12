include_directories(
  "${ovbase_SOURCE_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include"
)
add_compile_definitions(
  $<$<BOOL:${WIN32}>:__STDC_NO_THREADS__>
  $<$<BOOL:${WIN32}>:_WIN32_WINNT=0x0502>
  $<$<BOOL:${USE_STR}>:USE_STR>
  $<$<BOOL:${USE_WSTR}>:USE_WSTR>
  $<$<BOOL:${LEAK_DETECTOR}>:LEAK_DETECTOR>
  $<$<BOOL:${ALLOCATE_LOGGER}>:ALLOCATE_LOGGER>
  $<$<CONFIG:Release>:NDEBUG>
)
add_compile_options(
  $<$<AND:$<BOOL:${WIN32}>,$<BOOL:${USE_COMPILER_RT}>>:--rtlib=compiler-rt>
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
)
add_link_options(
  -Wl,--gc-sections
  # -Wl,--print-gc-sections
  $<$<AND:$<BOOL:${WIN32}>,$<BOOL:${USE_COMPILER_RT}>>:--rtlib=compiler-rt>
  $<$<BOOL:${WIN32}>:-no-pthread>
)
link_libraries(
  ovbase
  $<$<BOOL:${WIN32}>:shlwapi>
)
add_library(ovutil STATIC
  str.c
  $<$<BOOL:${WIN32}>:win32.c>
)
add_executable(test_ovutil_str str_test.c)
list(APPEND tests test_ovutil_str)
if(WIN32)
  add_executable(test_ovutil_win32 win32_test.c str.c)
  list(APPEND tests test_ovutil_win32)
endif()

foreach(target ${tests})
  add_test(NAME ${target} COMMAND ${target})
endforeach(target)

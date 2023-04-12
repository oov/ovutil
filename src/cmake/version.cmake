# Based on
# https://bravenewmethod.com/2017/07/14/git-revision-as-compiler-definition-in-build-with-cmake/
# https://github.com/tikonen/blog/tree/master/cmake/git_version
cmake_minimum_required(VERSION 3.0.0)

find_package(Git REQUIRED)
execute_process(
  COMMAND ${GIT_EXECUTABLE} tag --points-at HEAD
  WORKING_DIRECTORY "${local_dir}"
  OUTPUT_VARIABLE _git_tag
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
if ("${_git_tag}" STREQUAL "")
  set(_git_tag "vX.X.X")
endif()
message(STATUS "git tag: ${_git_tag}")

execute_process(
  COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
  WORKING_DIRECTORY "${local_dir}"
  OUTPUT_VARIABLE _git_revision
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
if ("${_git_revision}" STREQUAL "")
  set(_git_revision "unknown")
endif()
message(STATUS "git revision: ${_git_revision}")

set(output_file_temp "${output_file}.tmp")
configure_file(${input_file} ${output_file_temp} @ONLY)
file(MD5 ${output_file_temp} temp_md5)

if(EXISTS ${output_file})
  file(MD5 ${output_file} main_md5)
else()
  set(main_md5 "")
endif()

if(NOT "${main_md5}" STREQUAL "${temp_md5}")
  file(RENAME ${output_file_temp} ${output_file})
else()
  file(REMOVE ${output_file_temp})
endif()

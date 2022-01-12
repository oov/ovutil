cmake_minimum_required(VERSION 3.0.0)

string(REGEX MATCH "[^/]+$" FILENAME "${url}")
set(ZIP_PATH "${local_dir}/${FILENAME}")
if(NOT EXISTS "${ZIP_PATH}")
  file(DOWNLOAD "${url}" "${ZIP_PATH}")
endif()
file(REMOVE_RECURSE "${local_dir}/${dir}")
file(MAKE_DIRECTORY "${local_dir}/${dir}")

execute_process(
  COMMAND CMAKE -E tar xzvf ${ZIP_PATH}
  WORKING_DIRECTORY "${local_dir}/${dir}"
)

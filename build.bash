#!/usr/bin/env bash
set -eu

CUR_DIR="${PWD}"
cd "$(dirname "${BASH_SOURCE:-$0}")"

REBUILD=0
SKIP_TESTS=0
CMAKE_BUILD_TYPE=Release
ARCHS="i686 x86_64"
CMAKE_C_COMPILER=${CMAKE_C_COMPILER:-clang}
CMAKE_TOOL_CHANIN_FILE=${CMAKE_C_COMPILER:-}
while [[ $# -gt 0 ]]; do
  case $1 in
    -d|--debug)
      CMAKE_BUILD_TYPE=Debug
      shift
      ;;
    -a|--arch)
      ARCHS="$2"
      shift 2
      ;;
    -r|--rebuild)
      REBUILD=1
      shift
      ;;
    -s|--skip-tests)
      SKIP_TESTS=1
      shift
      ;;
    -*|--*)
      echo "Unknown option $1"
      exit 1
      ;;
    *)
      shift
      ;;
  esac
done

cd build

for arch in $ARCHS; do
  builddir="${PWD}/${CMAKE_BUILD_TYPE}/${arch}"
  if [ "${REBUILD}" -eq 1 ] || [ ! -e "${builddir}-ovbase/CMakeCache.txt" ]; then
    rm -rf "${builddir}-ovbase"
    cmake -S ../3rd/ovbase -B "${builddir}-ovbase" --preset debug \
      -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
      -DFORMAT_SOURCES=OFF \
      -DCMAKE_INSTALL_PREFIX=local \
      -DCMAKE_C_COMPILER="${CMAKE_C_COMPILER}" \
      -DCMAKE_TOOLCHAIN_FILE="${CMAKE_TOOL_CHANIN_FILE}"
  fi
  cmake --build "${builddir}-ovbase"
  if [ "${SKIP_TESTS}" -eq 0 ]; then
    ctest --test-dir "${builddir}-ovbase" --output-on-failure --output-junit testlog.xml
  fi

  if [ "${REBUILD}" -eq 1 ] || [ ! -e "${builddir}-ovutil/CMakeCache.txt" ]; then
    rm -rf "${builddir}-ovutil"
    cmake -S .. -B "${builddir}-ovutil" --preset debug \
      -DOVBASE_PATH="${builddir}-ovbase" \
      -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
      -DFORMAT_SOURCES=OFF \
      -DCMAKE_INSTALL_PREFIX=local \
      -DCMAKE_C_COMPILER="${CMAKE_C_COMPILER}" \
      -DCMAKE_TOOLCHAIN_FILE="${CMAKE_TOOL_CHANIN_FILE}"
  fi
  cmake --build "${builddir}-ovutil"
  if [ "${SKIP_TESTS}" -eq 0 ]; then
    ctest --test-dir "${builddir}-ovutil" --output-on-failure --output-junit testlog.xml
  fi
done

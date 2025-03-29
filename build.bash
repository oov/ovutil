#!/usr/bin/env bash
set -eu

CUR_DIR="${PWD}"
cd "$(dirname "${BASH_SOURCE:-$0}")"

INSTALL_TOOLS=1
REBUILD=0
SKIP_TESTS=0
CMAKE_BUILD_TYPE=Release
FORMAT_SOURCES=ON
while [[ $# -gt 0 ]]; do
  case $1 in
    --no-install-tools)
      INSTALL_TOOLS=0
      shift
      ;;
    -d|--debug)
      CMAKE_BUILD_TYPE=Debug
      shift
      ;;
    --no-format)
      FORMAT_SOURCES=OFF
      shift
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

if [ "${INSTALL_TOOLS}" -eq 1 ]; then
  mkdir -p "build/tools"
  if [ ! -e "${PWD}/build/tools/setup-llvm-mingw.bash" ]; then
    curl -o "${PWD}/build/tools/setup-llvm-mingw.bash" -sOL https://raw.githubusercontent.com/oov/ovbase/751f7da20311b3e1ebc49839d121ca0dd05e9952/setup-llvm-mingw.bash
  fi
  . "${PWD}/build/tools/setup-llvm-mingw.bash" --dir "${PWD}/build/tools"
fi

ARCHDIR=${ARCHDIR:-`uname -m`}
destdir="build/${PWD}/${CMAKE_BUILD_TYPE}/${ARCHDIR}"
CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX:-"${destdir}/local"}
CMAKE_C_COMPILER=${CMAKE_C_COMPILER:-clang}
CMAKE_TOOL_CHANIN_FILE=${CMAKE_TOOL_CHANIN_FILE:-}

if [ "${REBUILD}" -eq 1 ] || [ ! -e "${destdir}/CMakeCache.txt" ]; then
  rm -rf "${destdir}"
  cmake -S . -B "${destdir}" --preset debug \
    -DCMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}" \
    -DFORMAT_SOURCES="${FORMAT_SOURCES}" \
    -DCMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}" \
    -DCMAKE_C_COMPILER="${CMAKE_C_COMPILER}" \
    -DCMAKE_TOOLCHAIN_FILE="${CMAKE_TOOL_CHANIN_FILE}"
fi
cmake --build "${destdir}"
if [ "${SKIP_TESTS}" -eq 0 ]; then
  ctest --test-dir "${destdir}" --output-on-failure --output-junit testlog.xml
fi
cmake --install "${destdir}"

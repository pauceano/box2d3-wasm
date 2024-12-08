#!/usr/bin/env bash
# FLAVOUR=simd TARGET_TYPE=Debug ./shell/1_build_wasm.sh
set -eo pipefail
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
BOX2D_DIR="$(realpath "$DIR/../../box2d")"

Red='\033[0;31m'
Green='\033[0;32m'
Blue='\033[0;34m'
Purple='\033[0;35m'
NC='\033[0m' # No Color

BASENAME=Box2D
FLAVOUR_EMCC_OPTS=()
case "$FLAVOUR" in
  standard)
    ;;
  simd)
    BASENAME="$BASENAME.simd"
    FLAVOUR_EMCC_OPTS=(${FLAVOUR_EMCC_OPTS[@]} -msimd128)
    ;;
  *)
    >&2 echo -e "${Red}FLAVOUR not set.${NC}"
    >&2 echo -e "Please set FLAVOUR to 'standard' or 'simd'. For example, with:"
    >&2 echo -e "${Purple}export FLAVOUR='simd'${NC}"
    exit 1
    ;;
esac

# we used to use -s ENVIRONMENT=web for a slightly smaller build, until Node.js compatibility was requested in https://github.com/Birch-san/box2d-wasm/issues/8
EMCC_OPTS=(
  -fno-rtti
  -s MODULARIZE=1
  -s EXPORT_NAME=Box2D
  -s ALLOW_TABLE_GROWTH=1
  --memory-init-file 0
  -s FILESYSTEM=0
  -s SUPPORT_LONGJMP=0
  -s EXPORTED_FUNCTIONS=_malloc,_free
  -s ALLOW_MEMORY_GROWTH=1
  ${FLAVOUR_EMCC_OPTS[@]}
  )
DEBUG_OPTS=(
  -g3
)
RELEASE_OPTS=(-O3)

case "$TARGET_TYPE" in
  Debug)
    EMCC_OPTS=(
      ${EMCC_OPTS[@]}
      ${DEBUG_OPTS[@]}
      -s ASSERTIONS=2
      -s DEMANGLE_SUPPORT=1
      )
    ;;

  RelWithDebInfo)
    # consider setting --source-map-base if you know where
    # Box2D will be served from.
    EMCC_OPTS=(
      ${EMCC_OPTS[@]}
      ${RELEASE_OPTS[@]}
      ${DEBUG_OPTS[@]}
      )
    ;;
  
  Release)
    EMCC_OPTS=(
      ${EMCC_OPTS[@]}
      ${RELEASE_OPTS[@]}
      -flto
      --closure 1
      -s IGNORE_CLOSURE_COMPILER_ERRORS=1
      )
    ;;
  
  *)
    >&2 echo -e "${Red}TARGET_TYPE not set.${NC}"
    >&2 echo -e "Please set TARGET_TYPE to 'Debug' or 'Release'. For example, with:"
    >&2 echo -e "${Purple}export TARGET_TYPE='Debug'${NC}"
    exit 1
    ;;
esac
>&2 echo -e "TARGET_TYPE is $TARGET_TYPE"

BARE_WASM="$BASENAME.bare.wasm"

>&2 echo -e "${Blue}Building bare WASM${NC}"
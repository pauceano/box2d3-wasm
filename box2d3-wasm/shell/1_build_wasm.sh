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
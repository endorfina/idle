#!/bin/bash

# (ɔ) 2020 endorfina <dev.endorfina@outlook.com>
# GPLv3

readonly PROJECT_NAME='idle'
readonly SOURCE_DIR='cpp'
readonly BUILD_DIR='.cxx'
readonly COMPC_FILE='compile_commands.json'

if [[ -t 1 && -t 2 ]]
then
  ESC=$(printf '\033')'['
  readonly ESC

  readonly color_red=$ESC'1;31m'
  readonly color_norm=$ESC'0m'
else
  readonly color_red=
  readonly color_norm=
fi

die()
{
  echo >&2 '💀' "${color_red}${BASH_SOURCE[0]##*/} !!${color_norm}" "$@"
  exit 1
}

cd "$(dirname "${BASH_SOURCE[0]}")" || die "Couldn't move to the root directory"

[[ -d $SOURCE_DIR ]] || die "Source dir \"$SOURCE_DIR\" not found!"

BUILD_TYPE='Release'
CM_OPTS=()
CM_OPTS_CXX=()

if [[ $# -gt 0 && $1 == dev ]]
then
    shift
    set -- '-njdcx' "$@"
fi

for CLI_ARG in "$@"
do
  CLI_ARG=${CLI_ARG#-}

  while [[ -n $CLI_ARG ]]
  do
    OPT=${CLI_ARG:0:1}
    CLI_ARG=${CLI_ARG#?}

    case $OPT in
      d)
        BUILD_TYPE='Debug'
        ;;

      l)
        CM_OPTS+=("-DCMAKE_CXX_COMPILER=clang++")
        CM_OPTS_CXX+=("-stdlib=libc++")
        ;;

      n)
        CM_OPTS_CXX+=("-march=native" "-mtune=native")
        ;;

      u)
        CM_OPTS+=("-DCMAKE_UNITY_BUILD=ON")  # since cmake 3.16
        ;;

      j)
        CM_OPTS+=("-DMAKESHIFT_UNITY=ON")
        ;;

      c)
        CM_OPTS+=("-DCMAKE_EXPORT_COMPILE_COMMANDS=ON")
        ;;

      s)
        CM_OPTS+=("-DPRUNE_SYMBOLS=ON")
        ;;

      x)
        CM_OPTS+=("-DX11_USE_CLIENTMESSAGE=ON")
        ;;

      v)
        CM_OPTS+=("-DCMAKE_VERBOSE_MAKEFILE=ON")
        ;;

      *)
        echo >&2 "${0##*/}: Ignoring unknown option \"$OPT\""
        ;;
    esac
  done
done

ARGS=("-H${SOURCE_DIR}" "-B${BUILD_DIR}") # TODO: change to -S for cmake 3.14 compliance
                                          #       once it becomes available on android
ARGS+=("-DCMAKE_BUILD_TYPE=${BUILD_TYPE}")

command -v 'ninja' &>/dev/null && ARGS+=('-GNinja')

[[ ${#CM_OPTS_CXX[*]} -gt 0 ]] && ARGS+=("-DCMAKE_CXX_FLAGS=${CM_OPTS_CXX[*]}")

[[ ${#CM_OPTS[*]} -gt 0 ]] && ARGS+=("${CM_OPTS[@]}")

readonly ARGS

# READY TO BOOT

[[ ! -r $SOURCE_DIR/lodepng/lodepng.h ]] && git submodule update --init --recursive

if [[ -d $BUILD_DIR ]]
then
  echo 'Clearing existing cmake configuration files'
  find "$BUILD_DIR" -type f -delete
fi

hearts_iter=0
hearts=('💖' '🧡' '💛' '💚' '💙' '💜')
readonly hearts

echo "Using following CMake args:"
echo '--'
for ARG_ITER in "${ARGS[@]}"
do
  echo -n "    ${hearts[$hearts_iter]} "
  if [[ $ARG_ITER == -* ]]
  then
    echo "${color_red}${ARG_ITER:1:1} :${color_norm} '${ARG_ITER:2}'"
  else
    echo "$ARG_ITER"
  fi
  (( hearts_iter++ ))
  [[ $hearts_iter -ge ${#hearts[*]} ]] && hearts_iter=0
done

echo '--'
cmake "${ARGS[@]}" || die "CMake configuration failed. Verbatim CLI arguments: \"${ARGS[*]}\""

[[ ! -f "$SOURCE_DIR/$COMPC_FILE" \
  && -f "$BUILD_DIR/$COMPC_FILE" ]] \
  && ln -s "../$BUILD_DIR/$COMPC_FILE" "$SOURCE_DIR/$COMPC_FILE"

readonly INDENT=$'\t' # required by GNU make

tee "$SOURCE_DIR/Makefile" << _EOF | sed 's~\.\./~~' > 'Makefile'
## Makefile generated by ${0##*/} ##

${PROJECT_NAME}:
${INDENT}cmake --build '../${BUILD_DIR}'

test:
${INDENT}cmake --build '../${BUILD_DIR}' --target '${PROJECT_NAME}-test-run'

clean:
${INDENT}cmake --build '../${BUILD_DIR}' --target 'clean'

run: ${PROJECT_NAME}
${INDENT}@cd '../out/' && './${PROJECT_NAME}' || true

.PHONY: ${PROJECT_NAME} test clean run
_EOF


#!/bin/bash

# (ɔ) 2020 endorfina <dev.endorfina@outlook.com>
# GPLv3

readonly PROJECT_NAME='idle'
readonly SOURCE_DIR='cpp'
readonly BUILD_DIR='.cxx'
readonly COMPC_FILE='compile_commands.json'

readonly PROGNAME=${BASH_SOURCE[0]##*/}

if [[ -t 1 && -t 2 ]]
then
  readonly ESC=$(printf '\033')'['
  readonly color_blue=$ESC'0;34m'
  readonly color_red=$ESC'1;31m'
  readonly color_norm=$ESC'0m'
else
  readonly color_blue=
  readonly color_red=
  readonly color_norm=
fi

die()
{
  printf >&2 '💀 %s\n' "${color_red}${PROGNAME} !!${color_norm} $*"
  exit 1
}

cd "$(dirname "${BASH_SOURCE[0]}")" || die "Couldn't move to the root directory"

[[ -d $SOURCE_DIR ]] || die "Source dir \"$SOURCE_DIR\" not found!"

BUILD_TYPE='Release'
CM_OPTS=()
CM_OPTS_CXX=()
CM_LOG_LEVEL=2 # default: 2 (warnings)

if [[ $# -gt 0 && $1 == dev ]]
then
  shift
  set -- '-ncgSlp' "$@"
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

      C)
        CM_OPTS+=('-DCMAKE_CXX_COMPILER=clang++')
        CM_OPTS_CXX+=('-stdlib=libc++')
        ;;

      n)
        CM_OPTS_CXX+=('-march=native' '-mtune=native')
        ;;

      u)
        CM_OPTS+=('-DCMAKE_UNITY_BUILD=ON')  # since cmake 3.16
        ;;

      J)
        CM_OPTS+=('-DMAKESHIFT_UNITY=OFF')
        ;;

      c)
        CM_OPTS+=('-DCMAKE_EXPORT_COMPILE_COMMANDS=ON')
        ;;

      s)
        CM_OPTS+=('-DPRUNE_SYMBOLS=ON')
        ;;

      S)
        CM_OPTS+=('-DPRUNE_SYMBOLS=OFF')
        ;;

      g)
        CM_OPTS+=('-DCOMPILE_GALLERY=ON')
        ;;

      p)
        CM_OPTS+=('-DCOMPILE_FPS_COUNTERS=ON')
        ;;

      X)
        CM_OPTS+=('-DX11_USE_CLIENTMESSAGE=OFF')
        ;;

      v)
        CM_OPTS+=('-DCMAKE_VERBOSE_MAKEFILE=ON')
        ;;

      l)
        (( ++CM_LOG_LEVEL ))
        ;;

      L)
        CM_LOG_LEVEL=0
        ;;

      q)
        CM_OPTS+=('--log-level=WARNING')
        ;;

      *)
        echo >&2 "$PROGNAME: Ignoring unknown option \"$OPT\""
        ;;
    esac
  done
done

ARGS=("-S$SOURCE_DIR" "-B$BUILD_DIR" "-DCMAKE_BUILD_TYPE=$BUILD_TYPE")

command -v 'ninja' &>/dev/null && ARGS+=('-GNinja')

[[ ${#CM_OPTS_CXX[*]} -gt 0 ]] && ARGS+=("-DCMAKE_CXX_FLAGS=${CM_OPTS_CXX[*]}")

[[ ${#CM_OPTS[*]} -gt 0 ]] && ARGS+=("${CM_OPTS[@]}")

ARGS+=("-DLOG_LEVEL=$CM_LOG_LEVEL")

readonly ARGS

# READY TO BOOT
# There should be no side effects before this line

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
for arg_iter in "${ARGS[@]}"
do
  echo -n "    ${hearts[$hearts_iter]} "
  case $arg_iter in
    --*=*)
      arg_iter=${arg_iter#--}
      echo "${color_red}# :${color_blue} ${arg_iter%%=*} =${color_norm} '${arg_iter#*=}'"
      ;;

    --*)
      echo "${color_red}# :${color_blue} ${arg_iter#--}${color_norm}"
      ;;

    -*)
      echo "${color_red}${arg_iter:1:1} :${color_norm} '${arg_iter:2}'"
      ;;

    *)
      echo "$arg_iter"
      ;;
  esac
  (( ++hearts_iter ))
  [[ $hearts_iter -ge ${#hearts[*]} ]] && hearts_iter=0
done

echo '--'
cmake "${ARGS[@]}" || die "CMake configuration failed. Verbatim CLI arguments: \"${ARGS[*]}\""

[[ ! -f "$SOURCE_DIR/$COMPC_FILE" \
  && -f "$BUILD_DIR/$COMPC_FILE" ]] \
  && ln -s "../$BUILD_DIR/$COMPC_FILE" "$SOURCE_DIR/$COMPC_FILE"

readonly filehost=http://endorfina.dev

download_file()
{
  if [[ ! -f assets/$1 ]]
  then
    echo -n " 🌠  Fetching asset '$color_blue$1$color_norm'" && \
    curl -L -o "assets/$1" "$filehost/$1" && \
    echo '--'
  fi
}

: && download_file 'space-1.png' \
  && download_file 'path4368.png' \
  || die "Failed to download an asset."

# required by GNU make
readonly INDENT=$'\t'

# this allows us to get the boot commands straight from cmake
readonly RUN_SED_FILTER='/^CLI_RUN_COMMAND:/{s~^.*:STRING=~'"${INDENT}"'@~; s~cd '"'$(pwd)~cd '"'..~;p;q;}'

tee "$SOURCE_DIR/Makefile" << _EOF | sed -e "s~cd '..'[& ]*~~" -e 's~\.\./~~' > 'Makefile'
## Makefile generated by $PROGNAME ##

${PROJECT_NAME}:
${INDENT}cmake --build '../${BUILD_DIR}'

vars:
${INDENT}cmake -N -LH --build '../${BUILD_DIR}'

test:
${INDENT}cmake --build '../${BUILD_DIR}' --target '${PROJECT_NAME}-test-run'

clean:
${INDENT}cmake --build '../${BUILD_DIR}' --target 'clean'

exec:
$(cmake -N -L --build "$BUILD_DIR" 2>/dev/null | sed -nE "$RUN_SED_FILTER")

run: ${PROJECT_NAME} exec

perf:
$(cmake -N -L --build "$BUILD_DIR" 2>/dev/null \
    | sed -nE "${RUN_SED_FILTER/'p;q'/'s~\([[:space:]]*('"'[^']*'"')[^)]*\)$~perf stat \1~;p;q'}")

.PHONY: ${PROJECT_NAME} test clean run exec vars
_EOF


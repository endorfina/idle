#!/usr/bin/env bash

# (ɔ) 2020 endorfina <dev.endorfina@outlook.com>
# GPLv3

readonly PROJECT_NAME='idle'
readonly SOURCE_DIR='src'
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

      o)
        CM_OPTS+=("-DOUTPUT_DIR=$(pwd)")
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

[[ -r $SOURCE_DIR/cherry/cherry.sh ]] || git submodule update --init -j 2

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

try_download_assets()
{
  (cd assets/ || die "Cannot access the 'assets' directory"

  local filehost=https://idle.endorfina.dev/assets

  for filename in "$@"
  do
    [[ -f $filename ]] && continue

    echo "-- 🌠 Fetching asset '$color_blue$filename$color_norm'"

    curl --proto '=https' --tlsv1.2 -LOf\# --max-redirs 1 "$filehost/$filename" \
        || die "Failed to download an asset"
  done)
}

ASSETS=(
    space-1.png
    path4368.png
    icon.png
)
try_download_assets "${ASSETS[@]}"

# required by GNU make
readonly INDENT=$'\t'

# this allows us to get the boot commands straight from cmake
readonly exec_sed_filter='s~^~\t@~; s~cd '"'$(pwd)~cd '"'..~;'
readonly perf_sed_filter='s~\([[:space:]]*('"'[^']*'"')[^)]*\)$~perf stat \1~;'

make_target()
{
  local command_file=$BUILD_DIR/run_command
  local name=$1
  shift

  if [[ -n $command_file ]]
  then
    echo "$name:"
    sed -E -e "$*" "$command_file"
    echo
  fi
}

readonly RECONFIG_CLI="'../$PROGNAME'"$(for iarg in "$@"; do printf " '%s'" "$iarg"; done)

tee "$SOURCE_DIR/Makefile" << _EOF \
  | sed \
    -e "s~'\\.'~'$SOURCE_DIR'~" \
    -e "s~cd '\\.\\.'[& ]*~~" \
    -e 's~\.\./'"${BUILD_DIR//./\\.}~$BUILD_DIR~" \
    -e 's~\.\./~./~g' \
  > 'Makefile'
## Makefile generated by $PROGNAME ##

${PROJECT_NAME}: is_configured
${INDENT}cmake --build '../$BUILD_DIR'

is_configured:
${INDENT}@test -d '../$BUILD_DIR' || $RECONFIG_CLI

vars: is_configured
${INDENT}cmake -N -LH --build '../$BUILD_DIR'

reconfig: erase
${INDENT}${RECONFIG_CLI}

test: is_configured
${INDENT}cmake --build '../$BUILD_DIR' --target '${PROJECT_NAME}-test-run'

clean:
${INDENT}cmake --build '../$BUILD_DIR' --target 'clean'

ccmake:
${INDENT}ccmake -B '../$BUILD_DIR' -S '.'

erase:
${INDENT}rm -rf '../$BUILD_DIR'

$(make_target exec "$exec_sed_filter")

run: $PROJECT_NAME exec

$(make_target perf "$exec_sed_filter$perf_sed_filter")

.PHONY: $PROJECT_NAME is_configured test vars clean erase exec run perf
_EOF


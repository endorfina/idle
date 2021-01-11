#!/usr/bin/env bash

# (ɔ) 2020 endorfina <dev.endorfina@outlook.com>
# GPLv3

readonly project_name='idle'
readonly cxx_source_dir='cpp'
readonly cmake_build_dir='.cxx'
readonly cc_json_filename='compile_commands.json'

readonly program_name=${BASH_SOURCE[0]##*/}

if [[ -t 1 && -t 2 ]]
then
  readonly ESC=$'\e['
  readonly color_green=$ESC'0;32m'
  readonly color_blue=$ESC'0;34m'
  readonly color_red=$ESC'1;31m'
  readonly color_norm=$ESC'0m'
else
  readonly color_green=
  readonly color_blue=
  readonly color_red=
  readonly color_norm=
fi

die()
{
  printf >&2 '💀 %s\n' "${color_red}${program_name} !!${color_norm} $*"
  exit 1
}

set -o pipefail

cd "$(dirname "${BASH_SOURCE[0]}")" || die "Couldn't move to the root directory"

[[ -d $cxx_source_dir ]] || die "Source dir \"$cxx_source_dir\" not found!"

cmake_build_type='Release'
cmake_options=()
cmake_cxx_flags=()
cmake_log_level=2 # default: 2 (warnings)

if [[ $# -gt 0 && $1 == dev ]]
then
  shift
  set -- '-ncgSlpJ' "$@"
fi

for arg_iter in "$@"
do
  arg_iter=${arg_iter#-}

  while [[ -n $arg_iter ]]
  do
    opt_iter=${arg_iter:0:1}
    arg_iter=${arg_iter#?}

    case $opt_iter in
      d)
        cmake_build_type='Debug'
        ;;

      C)
        cmake_options+=('-DCMAKE_CXX_COMPILER=clang++')
        cmake_cxx_flags+=('-stdlib=libc++')
        ;;

      n)
        cmake_cxx_flags+=('-march=native' '-mtune=native')
        ;;

      u)
        cmake_options+=('-DCMAKE_UNITY_BUILD=ON')  # since cmake 3.16
        ;;

      J)
        cmake_options+=('-DMAKESHIFT_UNITY=OFF')
        ;;

      c)
        cmake_options+=('-DCMAKE_EXPORT_COMPILE_COMMANDS=ON')
        ;;

      s)
        cmake_options+=('-DPRUNE_SYMBOLS=ON')
        ;;

      S)
        cmake_options+=('-DPRUNE_SYMBOLS=OFF')
        ;;

      g)
        cmake_options+=('-DCOMPILE_GALLERY=ON')
        ;;

      p)
        cmake_options+=('-DCOMPILE_FPS_COUNTERS=ON')
        ;;

      X)
        cmake_options+=('-DX11_USE_CLIENTMESSAGE=OFF')
        ;;

      o)
        cmake_options+=("-DOUTPUT_DIR=$(pwd)")
        ;;

      v)
        cmake_options+=('-DCMAKE_VERBOSE_MAKEFILE=ON')
        ;;

      l)
        (( ++cmake_log_level ))
        ;;

      L)
        cmake_log_level=0
        ;;

      q)
        cmake_options+=('--log-level=WARNING')
        ;;

      *)
        echo >&2 "$program_name: Ignoring unknown option \"$opt_iter\""
        ;;
    esac
  done
done

cmake_cli_args=("-S$cxx_source_dir" "-B$cmake_build_dir" "-DCMAKE_BUILD_TYPE=$cmake_build_type")

command -v 'ninja' &>/dev/null && cmake_cli_args+=('-GNinja')

[[ ${#cmake_cxx_flags[*]} -gt 0 ]] && cmake_cli_args+=("-DCMAKE_CXX_FLAGS=${cmake_cxx_flags[*]}")

[[ ${#cmake_options[*]} -gt 0 ]] && cmake_cli_args+=("${cmake_options[@]}")

cmake_cli_args+=("-DLOG_LEVEL=$cmake_log_level")

readonly cmake_cli_args

# READY TO BOOT
# There should be no side effects before this line

[[ -r $cxx_source_dir/png/lodepng/lodepng.h ]] || git submodule update --init -j 2

if [[ -d $cmake_build_dir ]]
then
  echo 'Clearing existing cmake configuration files'
  find "$cmake_build_dir" -type f -delete
fi

hearts_iter=0
hearts=('💖' '🧡' '💛' '💚' '💙' '💜')
readonly hearts

echo "Using following CMake args:"
echo '--'
for arg_iter in "${cmake_cli_args[@]}"
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
cmake "${cmake_cli_args[@]}" \
    | sed -E 's~(-[[:space:]]+(done|found|yes|success))$~'"$color_green"'\1'"$color_norm~" \
    || die "CMake configuration failed. Verbatim CLI arguments: \"${cmake_cli_args[*]}\""

[[ ! -f "$cxx_source_dir/$cc_json_filename" \
  && -f "$cmake_build_dir/$cc_json_filename" ]] \
  && ln -s "../$cmake_build_dir/$cc_json_filename" "$cxx_source_dir/$cc_json_filename"

./fetch_assets.sh || die "Failed to download assets"


make_target()
{
  local command_file=$cmake_build_dir/run_command
  local name=$1
  shift
  local additional_cmds=()

  while [[ $# -gt 0 ]]
  do
      additional_cmds+=(-e "$1")
      shift
  done

  if [[ -r $command_file ]]
  then
    echo "$name:"
    sed -E \
        -e 's~#.*$~~' \
        -e '/^[[:space:]]$/d' \
        -e 's~^~\t@~' \
        -e "s~'$(pwd | sed 's~\.~\.~g')~'.~" \
        -e 's~cd[[:space:]]+'"'\\.'"'[[:space:]]*&&[[:space:]]*~~' \
        -e "s~'\\./([^/']+/)~'\\1~g" \
        "${additional_cmds[@]}" \
        "$command_file"
    echo
  fi
}

readonly reconfig_cli_quoted="'./$program_name'"$(for iarg in "$@"; do printf " '%s'" "$iarg"; done)

sed -E 's~^[[:space:]]+~\t~' > 'Makefile' << _EOF
## Makefile generated by $program_name ##

${project_name}: is_configured
  cmake --build '$cmake_build_dir'

is_configured:
  @test -d '$cmake_build_dir' || $reconfig_cli_quoted

vars: is_configured
  cmake -N -LH --build '$cmake_build_dir'

reconfig: erase
  ${reconfig_cli_quoted}

test: is_configured
  cmake --build '$cmake_build_dir' --target '${project_name}-test-run'

clean:
  cmake --build '$cmake_build_dir' --target 'clean'

ccmake:
  ccmake -B '$cmake_build_dir' -S '$cxx_source_dir'

erase:
  rm -rf '$cmake_build_dir'

$(make_target exec)

run: $project_name exec

$(make_target perf 's~\([[:space:]]*('"'[^']*'"')[^)]*\)$~perf stat \1~;')

.PHONY: $project_name is_configured test vars clean erase exec run perf
_EOF


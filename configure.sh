#!/bin/bash

readonly PROJECT_NAME='idle'
readonly SOURCE_DIR='cpp'
readonly BUILD_DIR='.cxx'
readonly COMPC_FILE='compile_commands.json'

die()
{
    local color_red
    local color_norm

    if [[ -t 2 ]]
    then
        local esc
        esc=$(printf '\033')

        color_red=$esc'[1;31m'
        color_norm=$esc'[0m'
    fi

    echo >&2 "${color_red}${0##*/} !!${color_norm}" "$@"
    exit 1
}

cd "$(dirname "${BASH_SOURCE[0]}")" || die "Couldn't move to the root directory"

BUILD_TYPE='Release'

[[ -d ${SOURCE_DIR} ]] || die "Source dir \"${SOURCE_DIR}\" not found!"

CM_OPTS=()
CM_OPTS_CXX=()

for CLI_ARG in "$@"
do
    CLI_ARG=${CLI_ARG#-}

    while [[ -n ${CLI_ARG} ]]
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

            v)
                CM_OPTS+=("-DCMAKE_VERBOSE_MAKEFILE=ON")
                ;;

            *)
                echo >&2 "${0##*/}: Unknown option \"${OPT}\"."
                ;;
        esac
    done
done

ARGS=("-H${SOURCE_DIR}" "-B${BUILD_DIR}") # TODO: change to -S for cmake 3.14 compliance
                                          # once it becomes available on android
ARGS+=("-DCMAKE_BUILD_TYPE=${BUILD_TYPE}")

command -v 'ninja' &>/dev/null && ARGS+=('-GNinja')

[[ ${#CM_OPTS_CXX[*]} -gt 0 ]] && ARGS+=("-DCMAKE_CXX_FLAGS=${CM_OPTS_CXX[*]}")

[[ ${#CM_OPTS[*]} -gt 0 ]] && ARGS+=("${CM_OPTS[@]}")

[[ ! -r $SOURCE_DIR/lodepng/lodepng.h ]] && git submodule update --init --recursive

if [[ -d $BUILD_DIR ]]
then
    echo 'Clearing existing cmake configuration files'
    find "$BUILD_DIR" -type f -delete
fi

echo "CMake args: ${ARGS[*]}"
echo '--'
cmake "${ARGS[@]}" || die "CMake configuration failed. Given cli arguments: \"${ARGS[*]}\""

[[ ! -f "${SOURCE_DIR}/${COMPC_FILE}" \
    && -f "${BUILD_DIR}/${COMPC_FILE}" ]] \
    && ln -s "../${BUILD_DIR}/${COMPC_FILE}" "${SOURCE_DIR}/${COMPC_FILE}"

readonly INDENT=$'\t' # required by GNU make

tee "${SOURCE_DIR}/Makefile" << _EOF | sed 's~\.\./~~' > 'Makefile'
## Makefile generated by ${0##*/} ##

${PROJECT_NAME}:
${INDENT}cmake --build '../${BUILD_DIR}'

test:
${INDENT}cmake --build '../${BUILD_DIR}' --target '${PROJECT_NAME}-test-run'

clean:
${INDENT}cmake --build '../${BUILD_DIR}' --target 'clean'

run: ${PROJECT_NAME}
${INDENT}cd '../out/' && './${PROJECT_NAME}'

.PHONY: ${PROJECT_NAME} test clean run
_EOF


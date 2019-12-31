#!/bin/bash

readonly PROJECT_NAME='idle'
readonly SOURCE_DIR='src'
readonly BUILD_DIR='cmake-build'
readonly COMPC_FILE='compile_commands.json'

BUILD_TYPE='Release'

if [[ ! -d "${SOURCE_DIR}" ]]
then
    echo "Source dir \"${SOURCE_DIR}\" not found!"
    exit 1
fi

while getopts 'dlncujv' OPT
do
    case $OPT in
        d)
            BUILD_TYPE='Debug'
            ;;

        l)
            CM_OPTS="${CM_OPTS} -DCMAKE_CXX_COMPILER=clang++"
            CM_OPTS_CXX="${CM_OPTS_CXX} -stdlib=libc++"
            ;;

        n)
            CM_OPTS_CXX="${CM_OPTS_CXX} -march=native -mtune=native"
            ;;

        u)
            CM_OPTS="${CM_OPTS} -DCMAKE_UNITY_BUILD=ON"  # since cmake 3.16
            ;;

        j)
            CM_OPTS="${CM_OPTS} -DMAKESHIFT_UNITY=ON"
            ;;

        c)
            CM_OPTS="${CM_OPTS} -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
            ;;

        v)
            CM_OPTS="${CM_OPTS} -DCMAKE_VERBOSE_MAKEFILE=ON"
            ;;

        *)
            echo "${0##*/}: Unknown option \"${OPT}\"."
            ;;
    esac
done

trim() {
    local var="$*"
    # leading
    var="${var#"${var%%[![:space:]]*}"}"
    # trailing
    var="${var%"${var##*[![:space:]]}"}"
    echo -n "$var"
}

ARGS[0]="-H${SOURCE_DIR}"               # TODO: change to -S for cmake 3.14 compliance
ARGS[1]="-B${BUILD_DIR}"                # once it becomes available on android
ARGS[2]="-DCMAKE_BUILD_TYPE=${BUILD_TYPE}"

command -v 'ninja' &>/dev/null && \
    ARGS+=('-GNinja')

if [[ -n "$CM_OPTS_CXX" ]]
then
    CM_OPTS_CXX=$(trim "$CM_OPTS_CXX")
    ARGS+=("-DCMAKE_CXX_FLAGS=${CM_OPTS_CXX}")
fi

if [[ -n "$CM_OPTS" ]]
then
    CM_OPTS=$(trim "$CM_OPTS")

    for OPT in ${CM_OPTS}
    do
        ARGS+=("$OPT")
    done
fi

if [[ -d "$BUILD_DIR" ]]
then
    echo 'Clearing existing cmake configuration'
    rm -r "${BUILD_DIR}/"
fi

echo ">> cmake ${ARGS[*]}"

if cmake "${ARGS[@]}"
then
    test ! -f "${SOURCE_DIR}/${COMPC_FILE}" -a -f "${BUILD_DIR}/${COMPC_FILE}" \
        && ln -s "../${BUILD_DIR}/${COMPC_FILE}" "${SOURCE_DIR}/${COMPC_FILE}"

    readonly INDENT=$'\t' # required by GNU make

    cat - > "${SOURCE_DIR}/Makefile" << EOF
## File generated automatically by ${0##*/} ##

${PROJECT_NAME}:
${INDENT}cmake --build '../${BUILD_DIR}'

test:
${INDENT}cmake --build '../${BUILD_DIR}' --target '${PROJECT_NAME}-test-run'

clean:
${INDENT}cmake --build '../${BUILD_DIR}' --target 'clean'

.PHONY: ${PROJECT_NAME} test clean
EOF

    sed 's~\.\./~~' "${SOURCE_DIR}/Makefile" > 'Makefile'

fi

#!/usr/bin/env bash

# (ɔ) 2020 endorfina <dev.endorfina@outlook.com>
# GPLv3

readonly PROGNAME=${0##*/}
readonly BUILD_DIR='.cxx'
readonly MOVED_BUILD_DIR='.cxx_moved'

if [[ -t 1 && -t 2 ]]
then
    readonly ESC=$(printf '\033')'['
    readonly color_red=$ESC'1;31m'
    readonly color_norm=$ESC'0m'
else
    readonly color_red=
    readonly color_norm=
fi

cleanup()
{
    if [[ -d $MOVED_BUILD_DIR ]]
    then
        [[ -d $BUILD_DIR ]] && rm -rf "$BUILD_DIR"
        mv "$MOVED_BUILD_DIR" "$BUILD_DIR"

        echo '✨ Returned everything to normal ✨'
    fi

}

die()
{
    while [[ $# -gt 0 ]]
    do
        echo >&2 "💀 ${color_red}${PROGNAME} !!${color_norm} $1"
        shift
    done
    exit 1
}

die_safely()
{
    cleanup
    die "$@"
}

cd "$(dirname "$0")" || die "Couldn't move to the root directory"

if [[ -d $MOVED_BUILD_DIR ]]
then
    [[ -d $BUILD_DIR ]] \
        || mv "$MOVED_BUILD_DIR" "$BUILD_DIR"

    die "'$MOVED_BUILD_DIR' directory found." \
        "🔧 Issued a fix but the issue may require manual aid." \
        "(hint: rm -rf '$BUILD_DIR')"
fi

if [[ -d "$BUILD_DIR" ]]
then
    mv "$BUILD_DIR" "$MOVED_BUILD_DIR"
    echo "Moved existing build files to '$MOVED_BUILD_DIR' 👻"
fi

try_builds()
{
    for flags in "$@"
    do
        # shellcheck disable=SC2015
        ./configure.sh "$flags" -q \
            && time make -s \
            && make -s test \
            || die_safely "Configuration '$color_red$flags$color_norm' has failed to build"

        echo

        find "$BUILD_DIR/out" -type f | while read -r filename
        do
            if [[ -x "$filename" ]]
            then
                echo -n "💽 $flags : "
                ls -lGh "$filename"
                echo '____'
            fi >&2
        done
    done
}

tmp_file=$(mktemp) || die_safely 'Failed to create a temp file'
readonly tmp_file

BUILDS=(
    llll
    dev
    J
    X
    L
    n)
try_builds "${BUILDS[@]}" 2> "$tmp_file"

echo
cat "$tmp_file"
rm "$tmp_file"

cleanup


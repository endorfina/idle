#!/usr/bin/env bash

# (ɔ) 2020 endorfina <dev.endorfina@outlook.com>
# GPLv3

readonly PROGNAME=${0##*/}
readonly BUILD_DIR='.cxx'
readonly SOURCE_DIR='cpp'
readonly IMPORTANT_FILES=("$BUILD_DIR" 'Makefile' "$SOURCE_DIR/Makefile")

if [[ -t 1 && -t 2 ]]
then
    readonly ESC=$(printf '\033')'['
    readonly color_red=$ESC'1;31m'
    readonly color_norm=$ESC'0m'
else
    readonly color_red=
    readonly color_norm=
fi

get_move_name()
{
    (dirname "$1"
    echo -n '/.'
    basename "$1" | sed 's~^\.~~'
    echo -n '_backup'
    ) | tr -d '\n'
}

save_important_stuff()
{
    local f moved
    for f in "${IMPORTANT_FILES[@]}"
    do
        moved=$(get_move_name "$f")

        if [[ -e $moved ]]
        then
            die_safely "'$moved' found." \
                "🔧 Issued a cleanup but building may require manual aid." \
                "(hint: rm -rf '$f')"
        fi

        mv -v "$f" "$moved" || die_safely "Failed to move '$f'"
    done
}

cleanup()
{
    local f moved
    for f in "${IMPORTANT_FILES[@]}"
    do
        moved=$(get_move_name "$f")

        if [[ -e $moved ]]
        then
            [[ -d $f ]] && rm -rf "$f"
            mv -vf "$moved" "$f"
        fi
    done

    echo '✨ Returned everything to normal ✨'
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

cd "$(dirname "$0")" || die "Couldn't move to the root directory"

save_important_stuff

tmp_file=$(mktemp) || die_safely 'Failed to create a temp file'
readonly tmp_file

readonly BUILDS=(
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


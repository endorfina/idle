#!/bin/sh

# (ɔ) 2020 endorfina <dev.endorfina@outlook.com>
# GPLv3

readonly PROGNAME=${0##*/}
readonly BUILD_DIR='.cxx'
readonly MOVED_BUILD_DIR='.cxx_moved'

if test -t 1 -a -t 2
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
    if test -d "$MOVED_BUILD_DIR"
    then
        test -d "$BUILD_DIR" && rm -rf "$BUILD_DIR"
        mv "$MOVED_BUILD_DIR" "$BUILD_DIR"

        echo "✨ Returned everything to normal ✨"
    fi
}

die()
{
    printf >&2 '💀 %s\n' "${color_red}${PROGNAME} !!${color_norm} $*"
    exit 1
}

die_safely()
{
    cleanup
    die "$@"
}

cd "$(dirname "$0")" || die "Couldn't move to the root directory"

test -d "$MOVED_BUILD_DIR" && die "'$MOVED_BUILD_DIR' directory found. Please cleanup before continuing further"

if test -d "$BUILD_DIR"
then
    mv "$BUILD_DIR" "$MOVED_BUILD_DIR"
    echo "Moved existing build files to '$MOVED_BUILD_DIR' 👻"
fi

while read -r flags
do
    # shellcheck disable=SC2015
    ./configure.sh "$flags" -q \
        && time make \
        && make test \
        || die_safely "Configuration '$color_red$flags$color_norm' has failed to build"

    echo

    find "$BUILD_DIR/out" -type f | while read -r filename
    do
        if test -x "$filename"
        then
            echo -n '💽 '
            ls -lGh "$filename"
        fi
    done

    echo

done << END
lll
dev
J
L
X
END

cleanup


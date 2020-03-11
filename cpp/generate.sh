#!/bin/bash

# (ɔ) 2020 endorfina <dev.endorfina@outlook.com>
# GPLv3

SOURCE=$(sed -E -e 's~//.*$~~' \
    -e 's~^[[:space:]]+~~' \
    -e 's~[[:space:]]+$~~' \
    -e 's~([^_ a-zA-Z0-9])[[:space:]]+~\1~g' \
    -e 's~[[:space:]]+([^_ a-zA-Z0-9])~\1~g' \
    -e '/^$/d')

indent()
{
    echo -n '    '
}

readonly OUTPUT=$(mktemp)
[[ -z $OUTPUT || ! -w $OUTPUT ]] && exit 1

cat << _EOF
/*  Generated on $(date) by ${BASH_SOURCE[0]##*/}  */

#pragma once
#include <string_view>

#define Ss(byte) static_cast<char>(0x##byte)

namespace shaders
{
namespace ${*}
{
_EOF

while [[ $SOURCE =~ [[:space:]]*@@[[:space:]]*([_[:alnum:]]+)[^_#[:alnum:]]*([^@]+\})(.*)$ ]]
do
    echo "constexpr unsigned int source_pos_${BASH_REMATCH[1]} = $(wc -c < "$OUTPUT");"

    echo "${BASH_REMATCH[2]}" >> "${OUTPUT}"
    dd if=/dev/zero bs=1 count=1 >> "${OUTPUT}" 2>/dev/null

    SOURCE="${BASH_REMATCH[3]}"
done

cat << _EOF

constexpr unsigned int source_size_uncompressed = $(wc -c < "$OUTPUT");

constexpr char _raw_data []
{
_EOF
indent

counter=0

for iter in $(gzip -cn9q < "$OUTPUT" \
    | od -t x1 -An \
    | tr '\t ' '\n' \
    | sed -e '/^$/d' \
        -e 's~$~),~' \
        -e 's~^~Ss(~' \
    | tr '\n' ' ' \
    | sed -E 's~,[[:space:]]*$~~')
do
    (( counter = counter + 1 ))
    echo -n "$iter"

    if [[ $counter -eq 12 ]]
    then
        echo
        indent
        counter=0
    fi
done

cat << _EOF

};

constexpr std::string_view get_data()
{
    return { _raw_data, sizeof(_raw_data) };
}

}  // namespace ${*}
}  // namespace shaders

#undef Ss
_EOF

rm "$OUTPUT" >&2


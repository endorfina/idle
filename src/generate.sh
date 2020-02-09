#!/bin/bash

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

get_file_size()
{
    local var
    var=$(wc -c < "${1}")
    var=${var#"${var%%[![:space:]]*}"}
    var=${var%"${var##*[![:space:]]}"}
    echo -n "$var"
}

readonly OUTPUT=$(mktemp)
[[ -z $OUTPUT || ! -w $OUTPUT ]] && exit 1

cat << _EOF
/*  Generated on $(date)  */

#pragma once
#include<string_view>

#define Ss(byte) static_cast<char>(0x##byte)

namespace
_EOF

[[ $# -gt 0 ]] && echo "${1}"

echo $'{\n'

while [[ $SOURCE =~ [[:space:]]*@@[[:space:]]*([a-zA-Z0-9_]+)[^a-zA-Z0-9_#]*([^@]+\})(.*)$ ]]
do
    echo "constexpr unsigned int _shader_source_pos_${BASH_REMATCH[1]}_ = $(get_file_size "$OUTPUT");"

    echo "${BASH_REMATCH[2]}" >> "${OUTPUT}"
    dd if=/dev/zero bs=1 count=1 >> "${OUTPUT}" 2>/dev/null

    SOURCE="${BASH_REMATCH[3]}"
done

cat << _EOF

constexpr unsigned int _shader_source_size_uncompressed_ = $(get_file_size "$OUTPUT");

constexpr char _shader_raw_data_ []
{
_EOF
indent

counter=0

for i in $(gzip -cn9q < "$OUTPUT" \
    | od -t x1 -An \
    | tr "\t " "\n" \
    | sed -e '/^$/d' \
        -e 's~$~),~' \
        -e 's~^~Ss(~' \
    | tr "\n" " " \
    | sed -E 's~,[[:space:]]*$~~')
do
    (( counter = counter + 1 ))
    echo -n "${i}"

    if [[ $counter -eq 12 ]]
    then
        echo
        indent
        counter=0
    fi
done

cat << _EOF

};

constexpr std::string_view get_shader_file()
{
    return { _shader_raw_data_, sizeof(_shader_raw_data_) };
}

}  // namespace

#undef Ss
_EOF

rm "$OUTPUT" >&2


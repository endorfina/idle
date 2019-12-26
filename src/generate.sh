#!/bin/bash

SOURCE=$(sed -E -e 's~//.*$~~' \
    -e 's~^[[:space:]]*~~' \
    -e 's~[[:space:]]*$~~' \
    -e 's~[[:space:]]*([^_ a-zA-Z0-9])[[:space:]]*~\1~g' \
    -e '/^$/d')

indent()
{
    echo -n '    '
}

readonly OUTPUT=$(mktemp)
test ! -w "$OUTPUT" && exit 1

echo '/*'
indent
echo 'Generated on' "$(date)"
printf '*/\n\n#pragma once\n#include<string_view>\n\n#define Ss(byte) static_cast<char>(0x##byte)\n\nnamespace'

if [[ $# -gt 0 ]]
then
    echo " ${1}"
else
    echo
fi

echo '{'
echo

get_file_size()
{
    wc -c < "${1}" \
        | head -n1 \
        | sed -E 's~[[:space:]]*([0-9]+)[[:space:]]*~\1~'
}

while [[ $SOURCE =~ [[:space:]]*@@[[:space:]]*([a-zA-Z0-9_]+)[^a-zA-Z0-9_#]*([^@]+\})(.*)$ ]]
do
    echo 'constexpr unsigned int' "_shader_source_pos_${BASH_REMATCH[1]}_" '=' "$(get_file_size "$OUTPUT");"

    echo "${BASH_REMATCH[2]}" >> "${OUTPUT}"
    dd if=/dev/zero bs=1 count=1 >> "${OUTPUT}" 2>/dev/null

    SOURCE="${BASH_REMATCH[3]}"
done

echo
echo 'constexpr unsigned int _shader_source_size_uncompressed_ =' "$(get_file_size "$OUTPUT");"
echo
echo 'constexpr char _shader_raw_data_ []'
echo '{'
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

    if test "$counter" -eq 12
    then
        echo
        indent
        counter=0
    fi
done

echo
echo '};'
echo
echo 'constexpr std::string_view get_shader_file()'
echo '{'
indent
echo 'return { _shader_raw_data_, sizeof(_shader_raw_data_) };'
echo '}'
echo
echo '}  // namespace'
echo
echo '#undef Ss'

rm "$OUTPUT" >&2


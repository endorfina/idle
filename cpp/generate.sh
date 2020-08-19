#!/bin/bash

# (ɔ) 2020 endorfina <dev.endorfina@outlook.com>
# GPLv3

SOURCE=$(sed -E -e 's~//.*$~~' \
    -e 's~^[[:space:]]+~~' \
    -e 's~[[:space:]]+$~~' \
    -e 's~[[:space:]]+([^_ [:alnum:]])~\1~g' \
    -e 's~([^_ [:alnum:]])[[:space:]]+~\1~g' \
    -e '/^$/d' \
    | tr '\n' '$' \
    | sed -E \
        -e 's~([^[:alnum:]])\$+~\1~g' \
        -e 's~([^$])(#|@@)~\1\$\2~g' \
    | tr '$' '\n')

NAMESPACE=namespace
[[ $# -gt 0 ]] && NAMESPACE+=" $*"
readonly NAMESPACE

indent()
{
    echo -n '    '
}

readonly OUTPUT=$(mktemp)
[[ -z $OUTPUT || ! -w $OUTPUT ]] && exit 1


# echo '//// SOURCE (WHITESPACE IS FOR LOSERS): ////'
# echo "$SOURCE" | sed -E 's~[^[:alnum:][:punct:]]~█~g;s~^~//  ~'

cat << _END
/*  Generated on $(date) by ${BASH_SOURCE[0]##*/}  */

#pragma once
#include <array>
#include <string_view>

#define Ss(byte) static_cast<char>(0x##byte)

namespace shaders
{
$NAMESPACE
{
enum source_info : unsigned
{
_END

while [[ $SOURCE =~ @@[[:space:]]*([_[:alnum:]]+)[^[:punct:][:alnum:]]*([^@]+\})(.*)$ ]]
do
    printf '    pos_%s = 0x%03x,\n' "${BASH_REMATCH[1]}" "$(wc -c < "$OUTPUT")"

    printf '%s\0' "${BASH_REMATCH[2]}" >> "$OUTPUT"

    SOURCE=${BASH_REMATCH[3]}
done

cat << _END

    size_uncompressed = $(wc -c < "$OUTPUT")
};

constexpr std::array deflated_data
{
_END
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
    (( ++counter ))
    echo -n "$iter"

    if [[ $counter -eq 15 ]]
    then
        echo
        indent
        counter=0
    fi
done

cat << _END

};

constexpr std::string_view get_view()
{
    return { deflated_data.data(), deflated_data.size() };
}

}  // $NAMESPACE
}  // namespace shaders

#undef Ss
_END

rm "$OUTPUT" >&2


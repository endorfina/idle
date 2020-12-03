#!/usr/bin/env bash

# (ɔ) 2020 endorfina <dev.endorfina@outlook.com>
# GPLv3

cd "$(dirname "${BASH_SOURCE[0]}")/assets" || die "Cannot access the 'assets' directory"

ASSETS=(
    space-1.png
    path4368.png
    icon.png
    debug_tex.png
)

readonly ASSETS
readonly filehost='https://idle.endorfina.dev/assets'

missing_assets=()

for filename in "${ASSETS[@]}"
do
    [[ -f $filename ]] && continue

    missing_assets+=('-O' "$filehost/$filename")
done

if [[ -t 1 ]]
then
  readonly ESC=$(printf '\033')'['
  readonly color_blue=$ESC'0;34m'
  readonly color_norm=$ESC'0m'
else
  readonly color_blue=
  readonly color_norm=
fi

curl_opts=('--proto' '=https'
    '--tlsv1.2'
    '-Lf#'
    '--max-redirs' 1)

readonly version_regex='curl[[:space:]]+([0-9]+)\.([0-9]+)'

if [[ $(curl --version | head -n1) =~ $version_regex ]]
then
    readonly curl_ver_major=${BASH_REMATCH[1]}
    readonly curl_ver_minor=${BASH_REMATCH[2]}

    [[ $curl_ver_major -gt 7 || $curl_ver_major -eq 7 && $curl_ver_minor -ge 66 ]] \
        && curl_opts+=(-Z)
fi

if [[ ${#missing_assets[*]} -gt 0 ]]
then
    echo -n "-- 🌠 Fetching assets: $color_blue"
    printf '%s' "${missing_assets[*]}" \
        | sed -E 's~-O[[:space:]]+[^[:space:]]+/~\n  -- ~g'
    echo "$color_norm"

    curl "${curl_opts[@]}" "${missing_assets[@]}"
fi


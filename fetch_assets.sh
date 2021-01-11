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

opt_force_all=no
opt_missing_only=no

while [[ $# -gt 0 ]]
do
    opt_iter=${1#-}
    shift

    while [[ -n $opt_iter ]]
    do
        case ${opt_iter:0:1} in
            f)
                opt_force_all=yes
                ;;

            m)
                opt_missing_only=yes
                ;;

        esac
        opt_iter=${opt_iter#?}
    done
done

readonly opt_{force_all,missing_only}

if [[ -t 1 ]]
then
    readonly ESC=$(printf '\033')'['
    readonly color_green=$ESC'1;32m'
    readonly color_blue=$ESC'0;34m'
    readonly color_norm=$ESC'0m'
else
    readonly color_green=
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

download_args=()
download_list=()

for filename in "${ASSETS[@]}"
do
    if [[ $opt_force_all == no ]]
    then
        if [[ $opt_missing_only == yes ]]
        then
            [[ -f $filename ]] && continue

        else
            download_args+=('-z' "./$filename")
        fi
    fi

    download_args+=('-O' "$filehost/$filename")

    [[ -f $filename ]] \
        && download_list+=("$filename$color_green*") \
        || download_list+=("$filename")
done

readonly download_{args,list}

if [[ ${#download_list[*]} -gt 0 ]]
then
    readonly list_item_prefix=${color_blue}$'\n  -- '

    printf '%s\n' "-- 🌠 Remote assets: ${download_list[*]/#/$list_item_prefix}${color_norm}"

    curl "${curl_opts[@]}" "${download_args[@]}"
fi


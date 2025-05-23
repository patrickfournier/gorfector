#!/bin/bash

src_dir=$1
build_dir=$2
pot_file=$3

shift 3
langs=$*

for lang in ${langs}; do
    if [[ -f "${build_dir}/${lang}.po" ]]; then
        rm "${build_dir}/${lang}.po"
    fi

    if [[ -f "${src_dir}/${lang}.po" ]]; then
        msgmerge -q -o "${build_dir}/${lang}.po" "${src_dir}/${lang}.po" "${pot_file}"
    else
        msginit --input="${pot_file}" --output="${build_dir}/${lang}.po" --locale="$lang" --no-translator
    fi

    mv "${build_dir}/${lang}.po" "${src_dir}/${lang}.po"
    ln -rs "${src_dir}/${lang}.po" "${build_dir}/${lang}.po"
done

printf "%s\n" $langs > "${build_dir}/LINGUAS"

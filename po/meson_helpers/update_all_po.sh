#!/bin/bash

pot_file=$1
linguas_file=$2
build_dir=$3
src_dir=$4

# Reads the LINGUAS file and outputs each word, one per line, from non-comment lines.
# Ignores empty lines and lines starting with # (and optional leading whitespace).
get_linguas() {
    if [[ ! -f "${linguas_file}" ]]; then
        echo "Error: LINGUAS file not found." >&2
        return 1
    fi

    while IFS= read -r line || [[ -n "$line" ]]; do
        # Skip empty lines and lines that start with # (possibly preceded by whitespace)
        if [[ -n "$line" && ! "$line" =~ ^\s*# ]]; then
            # Process the line: split into words and print each word on a new line.
            # $line is intentionally unquoted here to allow word splitting by IFS.
            for word in $line; do
                echo "$word"
            done
        fi
    done < "${linguas_file}"
}

langs=$(get_linguas)
for lang in $langs; do
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

#!/bin/bash

out_dir=$1
package_name=$2
# linguas_file=$3
shift 3

echo "Info: Compiling .po files to .mo files"

if [ $# -eq 0 ]; then
    echo "Info: No .po files provided to compile."
    exit 0 # Exit successfully if there are no files to process
fi

for po_file in "$@"
do
    lang=$(basename "$po_file" .po)
    mo_dir="${out_dir}/locale/${lang}/LC_MESSAGES"
    mkdir -p "$mo_dir" # Create the directory if it doesn't exist

    # Construct the output .mo file path
    mo_file="${mo_dir}/${package_name}.mo"

    echo "Compiling $po_file to $mo_file"
    # Use --check to validate the .po file format
    # Use --output-file or -o to specify the output file
    msgfmt --check --output-file="$mo_file" "$po_file"

    # Check if msgfmt command was successful
    if [ $? -ne 0 ]; then
        echo "Error: msgfmt failed for $po_file"
        exit 1 # Exit script with an error status
    fi
done

echo "All .mo files compiled successfully."

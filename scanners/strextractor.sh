#!/bin/bash

# Reads all JSON files in the scanners directory and extracts the strings from the options.
# The strings are then reformatted to enclose them in `_()` and saved to a file that looks like C++.
# The output file is named `scanners_strings.h` and is located in the current directory.
# The output file is parsable by xgettext.

echo "Extracting strings from $1 to $2"

cat $1/*/*.json | \
jq '[.options[].title,.options[].description,.options[].string_list[]] | map(select((. | length) > 0))' | \
sed 's/^  "/ _("/g' | \
sed -r 's/",?$/"),/g' > $2

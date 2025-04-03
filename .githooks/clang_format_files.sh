#!/bin/bash

# Regexp for grep to only choose some file extensions for formatting
exts="\.\(cpp\|hpp\)$"

# The formatter to use
formatter=`which clang-format-21`
formatter_config=".clang-format"
formatter_options="--sort-includes -Werror"

# Check availability of the formatter
if [ -z "$formatter" ]
then
  1>&2 echo "$formatter not found. Pre-commit formatting will not be done."
  exit 0
fi

tmp_dir=`mktemp -d`

# check if tmp dir was created
if [[ ! "$tmp_dir" || ! -d "$tmp_dir" ]]; then
  echo "Could not create temporary directory"
  exit 1
fi

cp "$formatter_config" "$tmp_dir/.clang-format"

function cleanup {
  rm -rf "$tmp_dir"
  echo "Cleaning up temporary files..."
}

# register the cleanup function to be called on the EXIT signal
trap cleanup EXIT

exit_status=0

# Format staged files
while read file; do
  if [[ ! -f "$file" ]]; then
    continue
  fi

  echo -n "Checking format for $file..."
  # Create directories for file
  dirpath="${file%/*}"
  if [[ "$dirpath" != "$file" ]]; then
    mkdir -p "$tmp_dir/${file%/*}"
  fi
  # Get the file from index
  git show ":$file" > "$tmp_dir/$file"
  "$formatter" $formatter_options -n --ferror-limit=1 "$tmp_dir/$file"

  if [[ $? != 0 ]]; then
  	echo " failed."
  	echo "  Run '$formatter $formatter_options -i $file' to reformat the file."
  	exit_status=1
  else
  	echo " done."
  fi
done <<<$(git diff --cached --name-only --diff-filter=ACMR | grep "$exts")

exit $exit_status


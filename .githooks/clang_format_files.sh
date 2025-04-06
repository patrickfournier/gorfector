#!/bin/bash
# This script is used to format C++ files using clang-format.
# It is intended to be used as a pre-commit hook in git.
# It checks the staged files for formatting issues and either
# reformats them or prints a message indicating that they are
# already formatted correctly.
# Usage: clang_format_files.sh [-n]
# -n: dry run, only check formatting without modifying files

# Regexp for grep to only choose some file extensions for formatting
exts="\.\(cpp\|hpp\)$"

# The formatter to use
formatter=$(which clang-format-21)
formatter_config=".clang-format"
formatter_options="--sort-includes -Werror"

# Check availability of the formatter
if [ -z "$formatter" ]
then
  1>&2 echo "$formatter not found. Pre-commit formatting will not be done."
  exit 0
fi


# Create a temporary directory to store files
tmp_dir=$(mktemp -d)
if [[ ! "$tmp_dir" || ! -d "$tmp_dir" ]]; then
  echo "Could not create temporary directory"
  exit 2
fi

cp "$formatter_config" "$tmp_dir/.clang-format"

function cleanup {
  rm -rf "$tmp_dir"
}

# Register the cleanup function to be called on the EXIT signal
trap cleanup EXIT

exit_status=0

no_file_found=1
reformatted_files=0

# Format staged files
while read file; do
  if [[ ! -f "$file" ]]; then
    continue
  fi

  no_file_found=0
  echo "Checking format for $file..."

  # Create directories for file
  dirpath="${file%/*}"
  if [[ "$dirpath" != "$file" ]]; then
    mkdir -p "$tmp_dir/${file%/*}"
  fi

  # Get the file from index
  git show ":$file" > "$tmp_dir/$file"

  # Check if the file is formatted correctly
  "$formatter" $formatter_options -n --ferror-limit=1 "$tmp_dir/$file" 2> /dev/null

  if [[ $? != 0 ]]; then
    echo "  One or more errors detected."
  	if [[ $1 == "-n" ]]; then
    	echo "  Run '$formatter $formatter_options -i $file' to reformat this file."
    	echo
    	exit_status=1
    else
    	echo "  Reformatting."
    	"$formatter" $formatter_options -i "$file"
    	reformatted_files=$((reformatted_files + 1))
    	echo
    fi
  else
  	echo " Format is correct."
  fi
done <<<$(git diff --cached --name-only --diff-filter=ACMR | grep "$exts")

if [[ $no_file_found == 1 ]]; then
  echo "No files found to format."
  echo "If you want to format a specific file, run:"
  echo "'$formatter $formatter_options -i <file>'"
  exit 0
fi

if [[ $1 == "-n" ]]; then
  if [[ $exit_status != 0 ]]; then
    echo "Some files are not formatted correctly."
    echo "Run '${BASH_SOURCE[0]}' to reformat all staged files."
    echo "Then review the changes and stage them."
  else
    echo "All files are formatted correctly."
  fi
else
  if [[ $reformatted_files != 0 ]]; then
    echo "Some files were reformatted."
    echo "Please review the changes and stage them."
  fi
fi

exit $exit_status


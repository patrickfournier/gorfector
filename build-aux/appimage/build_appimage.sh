#!/bin/bash
# This script builds the AppImage for the application.
# Run it from the root of the repository:
# ./build-aux/appimage/build_appimage.sh <build_dir> <app_dir>

if [ "$#" -lt 2 ]; then
    echo "Error: Missing required arguments."
    echo "Usage: $0 <build_dir> <app_dir> [build_type]"
    exit 1
fi

# Check if the script is being run from the root of the repository
if [ ! -d ".git" ]; then
    echo "Error: This script must be run from the root of the repository."
    exit 1
fi

set -euo pipefail

build_dir=$1
app_dir=$2
build_type=${3:-release} # Default to 'release' if the third argument is not provided

if [ ! -f "linuxdeploy-x86_64.AppImage" ]; then
    curl -L https://github.com/linuxdeploy/linuxdeploy/releases/download/1-alpha-20250213-2/linuxdeploy-x86_64.AppImage -o linuxdeploy-x86_64.AppImage
    chmod +x linuxdeploy-x86_64.AppImage
fi

if [ -d "${build_dir}" ]; then
    rm -rf "${build_dir}"
fi

if [ -d "${app_dir}" ]; then
    rm -rf "${app_dir}"
fi

meson setup "${build_dir}" --prefix=/usr --buildtype "${build_type}"
cd "${build_dir}" || exit 1
meson install --destdir ../"${app_dir}"

cd ../"${app_dir}" || exit
rm -f usr/bin/gorfector-tests
mv usr/share/metainfo/com.patrickfournier.gorfector.metainfo.xml usr/share/metainfo/com.patrickfournier.gorfector.appdata.xml

cd ..
./linuxdeploy-x86_64.AppImage --appdir "${app_dir}" --output appimage

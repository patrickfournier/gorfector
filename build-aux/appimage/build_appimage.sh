#!/bin/bash
# This script builds the AppImage for the application.
# Run it from the root of the repository.
# meson setup must have been given the --prefix=/usr option.

if [ ! -f "linuxdeploy-x86_64.AppImage" ]; then
    curl -L https://github.com/linuxdeploy/linuxdeploy/releases/download/1-alpha-20250213-2/linuxdeploy-x86_64.AppImage -o linuxdeploy-x86_64.AppImage
    chmod +x linuxdeploy-x86_64.AppImage
fi

if [ -d "AppDir" ]; then
    rm -rf "AppDir"
fi

cd buildDir
meson install --no-rebuild --destdir ../AppDir

cd ../AppDir
rm usr/bin/gorfector-tests
mv usr/share/metainfo/com.patrickfournier.gorfector.metainfo.xml usr/share/metainfo/com.patrickfournier.gorfector.appdata.xml

cd ..
./linuxdeploy-x86_64.AppImage --appdir AppDir --output appimage


#!/bin/sh

mkdir tmpbuild
cd tmpbuild
export CMAKE_INSTALL_PREFIX=./wormux-files
cmake ../..
make

# pwd = /mac/tmpbuild
rm -rf ../Wormux.app
echo "Create Wormux.app file"
mkdir -p ../Wormux.app/Contents/Resources
mkdir -p ../Wormux.app/Contents/MacOS
mkdir -p ../Wormux.app/Contents/Frameworks


# pwd = /mac
cd ..

echo "Copy data into .app file"
echo "Add icon and info.plist"
cp Info.plist.in ./Wormux.app/Contents/info.plist
cp ../data/wormux_128x128.icns ./Wormux.app/Contents/Resources/wormux.icns


#pwd = /mac/tmpbuild
cd tmpbuild

echo "Add bin"
cp src/wormux ../Wormux.app/Contents/MacOS/wormux

echo "Add data"
#cp -r $INSTALL_PREFIX/share/wormux ./Wormux.app/Contents/Resources/data
cp -r /usr/local/share/locale ../Wormux.app/Contents/Resources/locale

#pwd = /mac
cd ..

echo "Remove temps files"
#rm -rf tmpbuild
echo "Build done"


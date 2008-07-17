#!/bin/sh

#################################################
#  Script for compile with cmake under MacOS X  #
#           For     W O R M U X                 #
#           done by Plorf, Auria,               #
#         lynxlynxlynx and Snaggle              #
#################################################


#
# Set environment for compile
#
 
MAC=`pwd`/
ROOT=${MAC}../
SRC=${ROOT}src/


# Set up build flags
# to build as universal, add 'universal' as the first
# command line parameter when running this script
# eg ./cmake_build.sh universal

export MACOSX_DEPLOYMENT_TARGET=10.4
export FAT_CFLAGS="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch ppc -arch i386 -I/Developer/SDKs/MacOSX10.4u.sdk/usr/include"
export FAT_LDFLAGS="-Wl,-syslibroot,/Developer/SDKs/MacOSX10.4u.sdk -arch ppc -arch i386 -L/Developer/SDKs/MacOSX10.4u.sdk/usr/lib"

APP_VERSION=0.8svn
BUNDLE_NAME=Wormux
DMG_TARGET="${BUNDLE_NAME}-${APP_VERSION}"
DMG_OUT=${BUNDLE_NAME}-${APP_VERSION}-`uname -p`

if [ "$1" = "universal" ]
then
    echo "*******************************"
    echo ""
    echo "Universal build mode enabled !"
    echo ""
    echo "*******************************"
    export CFLAGS="${FAT_CFLAGS} ${CFLAGS}"
    export CXXFLAGS="${CFLAGS}"
    export LDFLAGS="${FAT_LDFLAGS} ${LDFLAGS}"
    BUNDLE_NAME=Wormux
    DMG_OUT=${BUNDLE_NAME}-${APP_VERSION}-Universal
fi



#
# Set files for CMake and compilation
#

# Copy the library libSDLmain_UB.a
if [ -e libSDLmain_UB.a ]
then
    cp libSDLmain_UB.a ${ROOT}
else
    echo "*** ${MAC}libSDLmain_UB.a Not Found ***"
    exit
fi

# Copy the librarie libintl.a
if [ -e libintl.a ]
then
    cp libintl.a ${ROOT}
else
    echo "*** ${MAC}libintl.a Not Found ***"
    exit
fi


#
# Clean temps files, and create Folders
#

TMP=${MAC}tmpbuild/
if [ -e ${TMP} ]
then
    echo "*****************"
    echo "Clean tmpbuild"
    rm -rf ${TMP}*
    echo "*****************"
else
    mkdir ${TMP}
fi

APP=${MAC}Wormux.app
if [ -e ${APP} ]
then
    echo "******************"
    echo "Clean package .app"
    rm -rf ${APP}/*
    echo "******************"
fi

if [ -e ${DMG_OUT}.dmg ]
then
    echo "******************"
    echo "Clean package ${DMG_OUT}.dmg"
    rm -rf ${DMG_OUT}.dmg
    echo "******************"
fi

echo "Create Wormux.app file"
mkdir -p ${APP}
mkdir -p ${APP}/Contents/MacOS/
mkdir -p ${APP}/Contents/Frameworks/
RES=${APP}/Contents/Resources/
mkdir -p ${RES}data/
mkdir -p ${RES}locale/

# Add icon and info.plist and PkgInfo
cp ${MAC}Info.plist.in ${APP}/Contents/Info.plist
cp ${MAC}PkgInfo.in ${APP}/Contents/PkgInfo
cp ${ROOT}data/wormux_128x128.icns ${RES}Wormux.icns



ARCHIVE=${MAC}Wormux-0.8svn.tar.bz2
if [ -e ${ARCHIVE} ]
then
    echo "******************"
    echo "Remove old archive" 
    rm -f ${ARCHIVE}
    echo "******************"
fi

#export CMAKE_INSTALL_PREFIX=./wormux-files


#
# Compile
#

cd ${TMP}
#awk '/^SET\(WORMUX_PATCH/ { sub(/^/,"#") } { print }' ${ROOT}CMakeLists.txt > tmp.$$.$$
#cp ${ROOT}CMakeLists.txt tmp.$$.$$.2
#mv tmp.$$.$$ ${ROOT}CMakeLists.txt
cmake ../.. --graphviz=viz.dot -DDATA_PATH=${RES} -DBIN_PATH=${APP}/Contents/MacOS/ -DBUILD=Release
#mv tmp.$$.$$.2 ${ROOT}CMakeLists.txt

make 
make install


#
# Generate .app File
#

# Do a simple test for check if data is well copied
if [ -e ${RES}/data/wormux_default_config.xml ]
then
    echo "Default_config ok"
else
    echo "*** ERROR : No Default_config ! : Probably no datas copied ***"
    exit 2 
fi


#
# Copy frameworks into package
#

echo "Copy all frameworks"
cd ${MAC};

# If frameworks are not available, they'll be download from this mirror
MIRROR=http://plorf.homeip.net/wormux/lib/
if [ ! -e "${MAC}frameworks.tar.bz2" ]
then 
    echo "Frameworks will be downloaded from ${MIRROR} (3MB)";
    curl ${MIRROR}frameworks.tar.bz2 -o ${MAC}frameworks.tar.bz2;
fi
    tar xfj ${MAC}frameworks.tar.bz2 -C ${APP}/Contents/Frameworks;
    echo "Frameworks copy done"


#
# Make .dmg and .tar.bz2 file
#

echo ""
echo "Creating the distributable disk image"
echo ""

/bin/mv ${APP} ${DMG_OUT}.app
APP=${DMG_OUT}.app

/usr/bin/hdiutil create -type SPARSE -size 85m -fs HFS+ -volname "${DMG_TARGET}" -attach ${BUNDLE_NAME}-${APP_VERSION}.sparseimage
/bin/cp -R ${APP} "/Volumes/${DMG_TARGET}"

/usr/bin/hdiutil unmount "/Volumes/${DMG_TARGET}"
/usr/bin/hdiutil convert -imagekey zlib-level=9 -format UDZO ${BUNDLE_NAME}-${APP_VERSION}.sparseimage -o ${DMG_OUT}.dmg
/bin/rm -f ${BUNDLE_NAME}-${APP_VERSION}.sparseimage


#
# Create Archive
#
#echo "Make archive ${ARCHIVE}"
#tar cfj ${ARCHIVE} ${APP}
#echo "Archive ${ARCHIVE} done"
    

#
# Clean environment
#

# Remove copy of libSDLmain_UB.a
if [ -e ${ROOT}libSDLmain_UB.a ]
then
    rm ${ROOT}libSDLmain_UB.a
fi

# Remove copy of libintl.a
if [ -e ${ROOT}libintl.a ]
then
    rm ${ROOT}libintl.a
fi

echo "Build done"

exit 0

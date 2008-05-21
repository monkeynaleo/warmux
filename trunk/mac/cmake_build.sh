#!/bin/sh

#
# Set environment for compile
#
 
MAC=`pwd`/
ROOT=${MAC}../
SRC=${ROOT}src/
cp libSDLmain_UB.a ${ROOT}
cp libintl.a ${ROOT}
mkdir ${MAC}tmpbuild
TMP=${MAC}tmpbuild/
mkdir ${MAC}tmpdata
TMPDATA=${MAC}tmpdata/

ARCHIVE=${MAC}Wormux-0.8svn.tar.bz2

export CMAKE_INSTALL_PREFIX=./wormux-files

#
# Compile
#

cd ${TMP}

awk '/^SET\(WORMUX_PATCH/ { sub(/^/,"#") } { print }' ${ROOT}CMakeLists.txt > tmp.$$.$$
cp ${ROOT}CMakeLists.txt tmp.$$.$$.2
mv tmp.$$.$$ ${ROOT}CMakeLists.txt
#cmake ../.. -DPREFIX:PATCH=${TMPDATA}
cmake ../..
mv tmp.$$.$$.2 ${ROOT}CMakeLists.txt

make
make install

#
# Clean environment
#

rm ${ROOT}/libSDLmain_UB.a ${ROOT}/libintl.a 


#
# Generate .app File
#

echo "Create Wormux.app file"

APP=${MAC}Wormux.app/
rm -rf ${APP} ${ARCHIVE}

mkdir -p ${APP}
mkdir -p ${APP}Contents/Resources/
mkdir -p ${APP}Contents/MacOS/
mkdir -p ${APP}Contents/Frameworks/
RES=${APP}Contents/Resources/
mkdir -p ${RES}data/
mkdir -p ${RES}locale/

echo "Copy data into .app file"

# Copy data files into .app
echo "Copy /data"
#cp -R ${TMPDATA}share/wormux/* ${RES}data/
INSTALL=/usr/local/share/
cp -R ${INSTALL}wormux/* ${RES}data

# Copy locale files into .app
#echo "Error : wormux in english, so i think it's not working... search where is it"
#cp -R ${INSTALL}locale ${RES}locale


# Install the locale files
#echo "Copying locale files to           ${RES}/locale"
#make -w -C ${ROOT}/po -e all-local-yes DESTDIR=${RES}
#make -w -C ${ROOT}/po -e install-data-local-yes localedir=${RES}/locale

TMPLOCALE=;
for TMPLOCALE in /usr/share/locale/??;
do [[ -e $lang/LC_MESSAGES/wormux.mo ]] || continue;
    mkdir -p $target/$lang/LC_MESSAGES;
    cp $lang/LC_MESSAGES/wormux.mo $target/$lang/LC_MESSAGES/wormux.mo;
done

# Add icon and info.plist and PkgInfo
cp ${MAC}Info.plist.in ${APP}Contents/info.plist
cp ${MAC}PkgInfo.in ${APP}Contents/
cp ${ROOT}data/wormux_128x128.icns ${RES}Wormux.icns


echo "Add bin"
cp ${TMP}src/wormux ${APP}Contents/MacOS/wormux
#cp ${TMPDATA}bin/wormux ${APP}Contents/MacOs/wormux

echo "Copy all frameworks"
cd ${MAC};
MIRROR=http://plorf.homeip.net/wormux/lib/

if [ ! -e "${MAC}frameworks.tar.bz2" ]
then 
    echo "Frameworks will be downloaded from ${MIRROR} (3MB)";
    curl ${MIRROR}frameworks.tar.bz2 -o ${MAC}frameworks.tar.bz2;
fi
    tar xfj ${MAC}frameworks.tar.bz2 -C ${APP}Contents/Frameworks;
    echo "Frameworks copy done"

#
# Make .dmg and .tar.bz2 file
#

    echo "Make archive ${ARCHIVE}"
    #tar cfj ${ARCHIVE} ${APP}
    #echo "Archive ${ARCHIVE} done"

    
echo "Remove temps files"
#rm -rf ${MAC}tmpbuild
echo "Build done"


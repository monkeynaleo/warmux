#!/bin/bash

getwarmuxversion() {
    local version=$(grep PACKAGE_VERSION src/config.h|cut -d' ' -f 3|tr -d '"')
    if [ "$version" = "svn" ]; then
	local rev=""

	if [ -d ".svn" ]; then
	    rev=$(svn info --xml |grep revis |awk -F '"' '{print $2}' |head -n 1)
	    rev=r${rev}
	elif [ -d ".git" ]; then
	    rev=$(git svn log --oneline --limit=1 | cut -d' ' -f1)
	fi

	if [ "$rev" = "" ]; then
	    rev="`date +%Y%m%d`"
	fi
	version=${version}-${rev}
    fi

    if [ "$version" = "" ]; then
	version="unknown"
    fi

    echo $version
}

copylib() {
    local lib=$1
    local dest=$2

    if [ "$lib" = "" ] || [ "$dest" = "" ]; then
	echo "copylib: missing arguments" 1>&2
    fi

    echo "* Copying $lib to $dest..."

    cp $lib $dest/ 2> /dev/null
    if [ $? -eq 0 ]; then
	return
    fi

    cp /lib/$lib $dest/ 2> /dev/null
    if [ $? -eq 0 ]; then
	return
    fi

    cp /usr/lib/$lib $dest/ 2> /dev/null
    if [ $? -eq 0 ]; then
	return
    fi

    echo "Fail to copy $lib in $dest" 1>&2
    exit 1
}

#########################################
# Commented during tests for performace
rm -Rf install_dir
make install DESTDIR=$(pwd)/install_dir
#########################################

rm -Rf SPackage
mkdir SPackage

mkdir SPackage/data
cd $(pwd)/install_dir/usr/local/share/warmux
tar c . |tar x -C ../../../../../SPackage/data
cd -

mkdir SPackage/data/locale
cd $(pwd)/install_dir/usr/local/share/locale
tar c . |tar x -C ../../../../../SPackage/data/locale
cd -

mkdir SPackage/bin/
cp $(pwd)/install_dir/usr/local/bin/warmux SPackage/bin/

mkdir SPackage/lib/

file SPackage/bin/warmux|grep 64-bit > /dev/null
if [ $? -eq 0 ]; then
    LDSO=ld-linux-x86-64.so.2
else
    LDSO=ld-linux.so.2
fi

cp /lib/$LDSO SPackage/lib/
if [ $? -ne 0 ]; then
    echo "Fail to copy $LDSO"
    exit 1
fi

for i in $(ldd SPackage/bin/warmux |awk -F '[>(]+' '{print $2}' |grep -v ')'); do
    copylib $i SPackage/lib/
done

for i in $(strings SPackage/lib/libSDL_image*.so* |grep -E 'lib(.*).so'); do
    copylib $i SPackage/lib
done

copylib libX*.so.?	SPackage/lib
copylib libpulse*.so.?  SPackage/lib
copylib alsa-lib/libasound_module*.so  SPackage/lib

#That was all dynamic loaded libraries... I hope !
strip SPackage/lib/*
strip SPackage/bin/warmux

# creating the shell script
cat > SPackage/warmux.sh << EOF
#!/bin/sh
#pulse=\`ps x|grep pulseaudio|wc -l\`
#if [ "\$pulse" -gt 1 ]; then
#    export SDL_AUDIODRIVER=pulse
#fi
LD_LIBRARY_PATH=./lib WARMUX_DATADIR=./data/ WARMUX_LOCALEDIR=./data/locale WARMUX_FONT_PATH=./data/font/Ubuntu-R.ttf ./lib/$LDSO ./bin/warmux
EOF

#You thaught what you read was the most awful thing you've ever seen?
#Here is even better

DIR="warmux-`getwarmuxversion`"
rm -Rf $DIR
rm -Rf $DIR.tar.gz
rm -Rf $DIR.sh
mv SPackage $DIR
tar czf ${DIR}.tar.gz $DIR

#Wait i'm not done yet, here is the worst part:
cat > ${DIR}.sh << EOF
#!/bin/sh

DIR=\$(mktemp -d)
mkdir -p \${DIR} 2>&1 > /dev/null
cat \$0 |tail -n +11 |tar xz -C \${DIR}
cd \${DIR}/worm*
sh ./warmux.sh
cd -
rm -Rf \${DIR}
exit 0
EOF

#If you've understood the previous part, it won't shock you too much....
#Else you'll need some drugs to survive.
cat ${DIR}.tar.gz >> ${DIR}.sh

chmod 500 ${DIR}.sh

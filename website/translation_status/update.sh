#!/bin/bash

path=`dirname $0`
cd $path
path=`pwd`

# removing old status files
rm -rf po_status

r=0

# get/update the trunk
if [ ! -d $path/trunk ]; then
    svn co svn://svn.gna.org/svn/wormux/trunk trunk
    r=$?
else
    cd $path/trunk
    svn up
    r=$?
    cd $path
fi

if [ $r -ne 0 ]; then
    echo "ERROR $r: Fail to checkout/update wormux trunk"
    exit 1
fi

cd $path/trunk
make clean; ./autogen.sh && ./configure
r=$?

if [ $r -ne 0 ]; then
    echo "ERROR $r: Fail to run configure on wormux trunk"
    exit 1
fi

# update translations status for the trunk
cd $path
make po_status BRANCH=trunk
if [ $r -ne 0 ]; then
    echo "ERROR $r: \"make po_status BRANCH=trunk\" has failed"
    exit 1
fi

make last_update BRANCH=trunk
if [ $r -ne 0 ]; then
    echo "ERROR $r: \"make last_update BRANCH=trunk\" has failed"
    exit 1
fi

# get all the branches
if [ ! -d $path/branches ]; then
    svn co svn://svn.gna.org/svn/wormux/branches branches
    r=$?
else
    cd $path/branches
    svn up
    r=$?
    cd $path
fi

if [ $r -ne 0 ]; then
    echo "ERROR: Fail to checkout/update wormux branches"
    exit 1
fi

for b in branches/*; do
    cd $b
    make clean; ./autogen.sh && ./configure
    cd $path
    make po_status BRANCH=$b
    r=$?
    if [ $r -ne 0 ]; then
	echo "ERROR $r: \"make po_status BRANCH=$b\" has failed"
	exit 1
    fi
done

mv po_status/branches/* po_status/
rm -rf po_status/branches

# send the files on ftp
lftp -f wormux.ftp


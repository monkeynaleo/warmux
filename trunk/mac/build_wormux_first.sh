export CFLAGS=-I/sw/include 
export LDFLAGS=-L/sw/lib 
export CXXFLAGS=$CFLAGS 
export CPPFLAGS=$CXXFLAGS 
export ACLOCAL_FLAGS="-I /sw/share/aclocal"
export PKG_CONFIG_PATH="/sw/lib/pkgconfig"
export MACOSX_DEPLOYMENT_TARGET=10.4

#cd wormux
cd ..
svn up
./autogen.sh
./configure
make clean
make -w

cd mac 
# Macports / Fink
# Fink : comment the second line
# Macports : comment the first line
./build_wormux_fink.sh
#./build_wormux_macports.sh

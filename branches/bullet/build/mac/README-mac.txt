******************
Use of mac scripts
******************

* The XCode package have to be installed in order to have gcc and other staff.
* Install mac deps from http://download.gna.org/wormux/mac/mac_dependencies.zip - copy frameworks to [hard disk]/Library/Frameworks/
* Install gettext (or at least libintl headers)

Then simply cd into this directory from terminal and launch "./cmake_build.sh"
For universal binaries (a universal box2D build will be necessary, the one included here is i386 only) : "./cmake_build.sh universal"
For dual-cores : "./cmake_build.sh 2" (disabled for now, didn't work for some reason and caused more trouble)

Then, to make the generated .app distributable, simply copy the frameworks from the mac deps package to /Wormux.app/Contents/Frameworks

Have fun :)


Plorf (Yoann Katchourine)
Auria (Marianne Gagnon)
lynxlynxlynx
Snaggle
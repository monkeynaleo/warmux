#
# Make .dmg file
#

APP_VERSION=0.8.3
BUNDLE_NAME=Wormux
DMG_TARGET="${BUNDLE_NAME}-${APP_VERSION}"
DMG_OUT=${BUNDLE_NAME}-${APP_VERSION}-`uname -p`
APP=./Wormux.app

echo ""
echo "Creating the distributable disk image"
echo ""

/bin/mv ${APP} ${DMG_OUT}.app
APP=${DMG_OUT}.app

/usr/bin/hdiutil create -type SPARSE -size 85m -fs HFS+ -volname "${DMG_TARGET}" -attach ${BUNDLE_NAME}-${APP_VERSION}.sparseimage
/bin/cp -R ${APP} "/Volumes/${DMG_TARGET}"
/bin/cp ../../ChangeLog "/Volumes/${DMG_TARGET}"
/bin/cp ../../BUGS "/Volumes/${DMG_TARGET}"
/bin/cp ../../COPYING "/Volumes/${DMG_TARGET}"
/bin/cp ../../README "/Volumes/${DMG_TARGET}"
/bin/cp ../../TODO "/Volumes/${DMG_TARGET}"

/usr/bin/hdiutil detach "/Volumes/${DMG_TARGET}"
/usr/bin/hdiutil convert -imagekey zlib-level=9 -format UDZO ${BUNDLE_NAME}-${APP_VERSION}.sparseimage -o ${DMG_OUT}.dmg
/bin/rm -f ${BUNDLE_NAME}-${APP_VERSION}.sparseimage
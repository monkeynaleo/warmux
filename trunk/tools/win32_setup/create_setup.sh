#!/bin/bash

## Init
PKG_CONFIG=pkg-config

# Path to which all others are relative
WORMUXDIR=..\..

# Path of MinGW installation
MINGWDIR=/mingw

# Windows registry software path
HKLM_PATH=SOFTWARE\Games\Wormux

# Clean prior stuff
rm -f Wormux-Setup*.exe

function pkg_path
{
  if [ -z "$1" ]; then return ""; fi
  var=$($PKG_CONFIG --variable=prefix $1 2>/dev/null)
  if [ -z "$var" ]; then echo "Couldn't find $1, aborting..." 1>&2; exit 1; fi
  echo "Found $1 in $var" 1>&2
  echo "$var"
}

## Make sure all is done
echo "Checking make status"
make -C "$WORMUXDIR" 2>/dev/null 1>&2 || (echo "Bad return code from make; aborting"; exit 1)

if ! pkg-config --help 2>/dev/null 1>&2 ; then
  echo "pkg-config not found, aborting..."
  exit 1
fi

# Create head
WORMUX_VERSION=`date +"%Y%m%d"`
cat > wormux.nsi <<EOF
;based on MUI Welcome/Finish Page Example Script written by Joost Verburg
!define MUI_ICON  ${WORMUXDIR}\src\wormux.ico
!define MUI_UNICON  ${WORMUXDIR}\src\uninstall.ico
!define MUI_COMPONENTSPAGE_SMALLDESC
!define MUI_LANGDLL_ALWAYSSHOW
!include "MUI.nsh"
!include "Sections.nsh"
!include "LogicLib.nsh"
!include "StrFunc.nsh"

Name "Wormux"

;General
OutFile "Wormux-Setup-${WORMUX_VERSION}.exe"
!ifndef COMPRESSION
  !define COMPRESSION lzma
!endif
SetCompressor lzma

!define MUI_LANGDLL_REGISTRY_ROOT "HKLM" 
!define MUI_LANGDLL_REGISTRY_KEY "${HKLM_PATH}" 
!define MUI_LANGDLL_REGISTRY_VALUENAME "lang"

;--------------------------------
;Configuration

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE \$(WormuxLicense)
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!define MUI_ABORTWARNING

;Languages
!insertmacro MUI_LANGUAGE "English"
LicenseLangString WormuxLicense "English" "${WORMUXDIR}\license\COPYING.en.txt"
LangString TITLE_Wormux "English" "Wormux"
LangString DESC_Wormux  "English" "Installs the game Wormux, version ${WORMUX_VERSION}"

!insertmacro MUI_LANGUAGE "French"
LicenseLangString WormuxLicense "French" "${WORMUXDIR}\license\COPYING.fr.txt"
LangString TITLE_Wormux "French" "Wormux"
LangString DESC_Wormux  "French" "Installe le jeu Wormux, en version ${WORMUX_VERSION}"

!insertmacro MUI_LANGUAGE "German"
LicenseLangString WormuxLicense "German" "${WORMUXDIR}\license\COPYING.de.txt"
LangString TITLE_Wormux "German" "Wormux"
LangString DESC_Wormux  "German" "Das Spiel Wormux, Version ${WORMUX_VERSION} anbringen"

!insertmacro MUI_LANGUAGE "Spanish"
LicenseLangString WormuxLicense "Spanish" "${WORMUXDIR}\license\COPYING.es.txt"
LangString TITLE_Wormux "Spanish" "Wormux"
LangString DESC_Wormux  "Spanish" "Instala el juego Wormux, versión ${WORMUX_VERSION}"

!insertmacro MUI_LANGUAGE "Dutch"
LicenseLangString WormuxLicense "Dutch" "${WORMUXDIR}\license\COPYING.nl.txt"
LangString TITLE_Wormux "Dutch" "Wormux"
LangString DESC_Wormux  "Dutch" "Wormux ${WORMUX_VERSION}"

!insertmacro MUI_LANGUAGE "Polish"
LicenseLangString WormuxLicense "Polish" "${WORMUXDIR}\license\COPYING.pl.txt"
LangString TITLE_Wormux "Polish" "Wormux"
LangString DESC_Wormux  "Polish" "Wormux ${WORMUX_VERSION}"

!insertmacro MUI_LANGUAGE "Russian"
LicenseLangString WormuxLicense "Russian" "${WORMUXDIR}\license\COPYING.ru.txt"
LangString TITLE_Wormux "Russian" "Wormux"
LangString DESC_Wormux  "Russian" "Wormux ${WORMUX_VERSION}"

!insertmacro MUI_LANGUAGE "Slovenian"
LicenseLangString WormuxLicense "Slovenian" "${WORMUXDIR}\license\COPYING.sk.txt"
LangString TITLE_Wormux "Slovenian" "Wormux"
LangString DESC_Wormux  "Slovenian" "Wormux ${WORMUX_VERSION}"

;Folder-selection page
InstallDir "$PROGRAMFILES\Wormux"
; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM ${HKLM_PATH} "pth"

AutoCloseWindow false

;--------------------------------
;Installer Sections

Section \$(TITLE_Wormux) Sec_Wormux
  ; Set output path to the installation directory.
  SetOutPath \$INSTDIR
  File "${WORMUXDIR}\src\wormux.ico"
  File "${WORMUXDIR}\src\uninstall.ico"
  File "${WORMUXDIR}\src\wormux.exe"
EOF

# All iconv+gettext+jpeg+zlib+mingw stuff
cp $MINGWDIR/bin/libintl-3.dll $MINGWDIR/bin/libiconv-2.dll \
   $MINGWDIR/bin/jpeg62.dll $MINGWDIR/bin/zlib1.dll .

# Glib
GLIB_PATH=$(pkg_path glib-2.0)
cp "$GLIB_PATH/bin/libgobject"*.dll "$GLIB_PATH/bin/libgthread"*.dll \
   "$GLIB_PATH/bin/libglib"*.dll "$GLIB_PATH/bin/libgmodule"*.dll .

# Other libs
cp "$(pkg_path sigc++-2.0)/bin/libsigc"*.dll .
cp "$(pkg_path libxml-2.0)/bin/libxml2"*.dll .
cp "$(pkg_path libxml++-2.6)/bin/libxml++"*.dll .
cp "$(pkg_path glibmm-2.4)/bin/libglibmm"*.dll .
cp "$(pkg_path libpng13)/bin/libpng13.dll" .

# Clean up before non-strippable files
strip ./*.dll "$WORMUXDIR"/src/*.exe

# Files that must not be stripped (all of SDL)
SDL_PATH=`sdl-config --prefix`
cp $MINGWDIR/bin/zlib1.dll "$SDL_PATH/bin/SDL_mixer.dll" \
   "$SDL_PATH/bin/SDL_ttf.dll"  "$SDL_PATH/bin/SDL_image.dll" \
   "$SDL_PATH/bin/SDL.dll" .

# Produce installer
echo "  ; Dlls and co" >> wormux.nsi
echo "  File \"*.dll\"" >> wormux.nsi

## Locale
echo -e "\n  ; Locale\n" >> wormux.nsi
for gmo in "$WORMUXDIR"/po/*.gmo; do
  lg=${gmo%%.gmo}
  lg=${lg//.*\//}
  gmo=${gmo//\//\\}
  echo "  SetOutPath \$INSTDIR\\locale\\$lg\\LC_MESSAGES" >> wormux.nsi
  echo "  File /oname=wormux.mo $gmo" >> wormux.nsi
done

## Data - I love this syntax
cat >> wormux.nsi <<EOF
  ; Data
  SetOutPath \$INSTDIR
  File /r /x .svn /x Makefile* /x Makefile.* ${WORMUXDIR}\data
EOF

## License
cat >> wormux.nsi <<EOF
  ; Licenses
  File /r /x .svn ${WORMUXDIR}\license
EOF

# End
cat >> wormux.nsi <<EOF

  ; Write the installation path into the registry
  WriteRegStr HKLM ${HKLM_PATH} "pth" "$INSTDIR"
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Wormux" "DisplayName" "Wormux (remove only)"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Wormux" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteUninstaller "uninstall.exe"
  ; Shortcuts
  SetShellVarContext all
  CreateDirectory "$SMPROGRAMS\Wormux"
  CreateShortCut  "$SMPROGRAMS\Wormux\Wormux.lnk" "$INSTDIR\Wormux.exe" "" "$INSTDIR\Wormux.exe" 0
  CreateShortCut  "$SMPROGRAMS\Wormux\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  ;Write language to the registry (for the uninstaller)
  WriteRegStr HKLM ${HKLM_PATH} "Installer Language" $LANGUAGE

  GetFullPathName /SHORT $SHORTINSTDIR $INSTDIR
SectionEnd

;--------------------------------
;Descriptions

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${Sec_Wormux} $(DESC_Wormux)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"
  ; remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Wormux"
  DeleteRegKey HKLM ${HKLM_PATH}
  DeleteRegKey HKCU ${HKLM_PATH}
  ; remove shortcuts, if any.
  SetShellVarContext all
  Delete "\$SMPROGRAMS\Wormux\*.*"
  RMDir  "\$SMPROGRAMS\Wormux"

  ; remove files
  RMDir /r "\$INSTDIR"

  ;  !insertmacro MUI_UNFINISHHEADER
SectionEnd

Function .onInit

  ;Language selection

  !insertmacro MUI_LANGDLL_DISPLAY

  SectionSetFlags \${Sec_Wormux} 17

FunctionEnd

Function un.onInit
  !insertmacro MUI_UNGETLANGUAGE
FunctionEnd
EOF

## Compile installer
makensis wormux.nsi

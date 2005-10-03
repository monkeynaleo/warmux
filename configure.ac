dnl ========================================================================
dnl =                   W O R M U X  -  confiure.ac                        =
dnl =                                                                      =
dnl = Process this file with autoconf to produce a configure script.       =
dnl ========================================================================


dnl ========================================================================
dnl === Initial configuration                                              =
dnl ========================================================================

dnl = Require at least automake 2.52
AC_PREREQ(2.52)

AC_INIT([Wormux], [0.6.0], [wormux-dev@gna.org], [wormux])
AC_CONFIG_SRCDIR([src/main.cpp])

dnl Detect the canonical host and target build environment
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

#AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([foreign 1.5])

AM_MAINTAINER_MODE
AM_GNU_GETTEXT([external])


dnl ========================================================================
dnl === Check for tools                                                    =
dnl ========================================================================

AC_PROG_MAKE_SET
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_INSTALL

dnl =========================================================================
dnl === Options for users                                                   =
dnl =========================================================================

AX_CFLAGS_WARN_ALL([CXXFLAGS])

AC_ARG_ENABLE([debug],
	      [  --enable-debug          Enable debug in Wormux],
	      [if test "x${enableval}" = "xyes" ; then
                   CXXFLAGS="$CXXFLAGS -DDEBUG -ggdb3"
	       fi])

AC_ARG_ENABLE([profile],
	      [  --enable-profile        Enable profiling],
	      [if test "x${enableval}" = "xyes" ; then
                   CXXFLAGS="$CXXFLAGS -pg"
	       fi])

AC_ARG_ENABLE([static],
              [  --enable-static         Enable static building of wormux],
	      [static=$enableval],
	      [static=no])

# Check whether user supplied the option to statically link binaries.
#if test "$static" = yes; then
# if we're using gcc, add `-static' to LDFLAGS
#       if test -n "$GCC" || test "$ac_cv_prog_gcc" = "yes"; then
#               STATIC_LD="-static"
#               LDFLAGS="$LDFLAGS -static"
#       fi
#fi

DATADIR=$PACKAGE
AC_ARG_ENABLE([datadir-name],
	      [  --datadir-name[[=dir]]    Change name of data directory. Default with this option: wormux], 
	      [case "${enableval}" in
	       yes)
			DATADIR="wormux"
			;;
		no)
			;;
		*)
			DATADIR="${enableval}"
			;;
		esac])

AC_SUBST([DATADIR])
		
AM_CONDITIONAL([STATIC], [test x$static = xyes])
AM_CONDITIONAL([GCC], [test x$GXX = xyes])
AM_CONDITIONAL([INCLUDEDINTL], [test x$nls_cv_use_gnu_gettext = xyes])


dnl ========================================================================
dnl === Checks for required libraries to be present                        =
dnl ========================================================================

SDL_VERSION=1.2.6




dnl === Check for SDL ======================================================

AM_PATH_SDL($SDL_VERSION,
            :,
	    AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!]))
CXXFLAGS="$CXXFLAGS $SDL_CFLAGS"
CFLAGS="$CFLAGS $SDL_CFLAGS"
LIBS="$LIBS $SDL_LIBS"


dnl === Check for SDL_image ================================================

AC_CHECK_LIB(SDL_image, IMG_Load,,
	AC_MSG_ERROR([SDL_image library required]))
LIBS="$LIBS -lSDL_image"


dnl === Check for SDL_mixer ================================================

AC_CHECK_LIB(SDL_mixer, Mix_OpenAudio,,
	AC_MSG_ERROR([SDL_mixer library required]))
LIBS="$LIBS -lSDL_mixer"


dnl === Check for SDL_ttf ==================================================

AC_CHECK_LIB([SDL_ttf], [TTF_OpenFont],, 
        AC_MSG_ERROR([SDL_ttf library is required]))
LIBS="$LIBS -lSDL_ttf"

dnl === Check for SDL_gfx ==================================================

AC_CHECK_LIB(SDL_gfx, zoomSurface,,
	AC_MSG_ERROR([SDL_gfx library required]))
LIBS="$LIBS -lSDL_gfx"


dnl === Check for paragui-config  ==========================================

AC_PATH_PROGS([PGUI_CONFIG], [paragui-config], [none])
if test "x$PGUI_CONFIG" = "xnone"; then	
	AC_MSG_ERROR([*** paragui-config has not been found.])
fi
AC_MSG_CHECKING(for Paragui)
if $PGUI_CONFIG --libs > /dev/null 2>&1; then
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
	AC_MSG_ERROR([*** Paragui not found!])
fi
PARAGUI_CFLAGS=`$PGUI_CONFIG --cflags`
PARAGUI_LIBS=`$PGUI_CONFIG --libs`
CFLAGS="$CFLAGS $PARAGUI_CFLAGS"
CXXFLAGS="$CXXFLAGS $PARAGUI_CFLAGS"
LIBS="$LIBS $PARAGUI_LIBS"


dnl === Check for libxml++ 1.x =============================================

dnl Check first for pkg-config  

AC_PATH_PROGS([PACKAGE_CONFIG], [pkg-config], [none])
if test "x$PACKAGE_CONFIG" = "xnone"; then
	
	AC_MSG_ERROR([*** pkg-config, needed to check for libxml++ existence
has not been found.])
fi

AC_MSG_CHECKING(for libxml++)

if $PACKAGE_CONFIG libxml++-1.0 --libs > /dev/null 2>&1; then
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
	AC_MSG_ERROR([*** libxml++ version 1.0 not found!])
fi
LIBXMLPP_CFLAGS=`$PACKAGE_CONFIG libxml++-1.0 --cflags`
LIBXMLPP_LIBS=`$PACKAGE_CONFIG libxml++-1.0 --libs`
CFLAGS="$CFLAGS $LIBXMLPP_CFLAGS"
CXXFLAGS="$CXXFLAGS $LIBXMLPP_CFLAGS"
LIBS="$LIBS $LIBXMLPP_LIBS"

dnl ========================================================================
dnl == Check for header files                                              =
dnl ========================================================================

dnl TODO: remove them when we don't need them any more ;)

AC_HEADER_DIRENT
AC_HEADER_STDC


dnl ========================================================================
dnl === Data file substitution.                                          ===
dnl ========================================================================

DATA_FILES=`cd data ; find . -name CVS -prune -o -name ".svn*" -o -name "Makefile*" -o -name ".#*" -o -type d -o -print`
DATA_FILES=`echo $DATA_FILES`

AC_SUBST([DATA_FILES])

AM_PO_SUBDIRS

AC_CONFIG_FILES([Makefile
		 data/Makefile
		 src/Makefile
		 ])

AC_CONFIG_FILES([po/Makefile], [AM_POSTPROCESS_PO_MAKEFILE])

AM_CONFIG_HEADER(src/config.h)

AC_OUTPUT

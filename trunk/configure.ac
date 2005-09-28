# Process this file with autoconf to produce a configure script.

#######################################################################
# Initial configuration                                               #
#######################################################################

AC_PREREQ([2.57])
AC_INIT([Wormux], [0.6.0], [wormux-dev@gna.org], [wormux])

AC_REVISION([$Revision: 1.20 $])

#AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([foreign 1.5])
AC_CONFIG_SRCDIR([src/main.cpp])
#AC_CONFIG_HEADER()

AM_MAINTAINER_MODE
AM_GNU_GETTEXT([external])

#######################################################################
# Configuration options                                               #
#######################################################################

AX_CFLAGS_WARN_ALL([CXXFLAGS])

AC_ARG_ENABLE([debug],
	      [  --enable-debug        Enable debug in Wormux],
	      [if test "x${enableval}" = "xyes" ; then
                   CXXFLAGS="$CXXFLAGS -DDEBUG -ggdb3"
	       fi])

AC_ARG_ENABLE([static],
              [  --enable-static       Enable static building of wormux],
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
	      [  --datadir-name[[=dir]] Change name of data directory. Default with this option: wormux], 
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

#######################################################################
# Checks for programs.                                                #
#######################################################################

AC_PROG_CXX
AC_PROG_CC
have_libx11='no'
if test "$with_x" != 'no'; then

dnl Locate X include files and libraries
   AC_PATH_XTRA
   NEW_LIBS="$X_LIBS -lX11"

   AC_CHECK_LIB(X11, XOpenDisplay, have_libx11='yes',have_libx11='no',$X_LIBS)
   if test "$have_libx11" != 'no'; then
     AC_DEFINE(HAVE_LIBX11,,Define if you have X11 libraries)
     X_LIBS="$NEW_LIBS"
     CPPFLAGS="$X_CFLAGS $CPPFLAGS"
     LIBS="$X_LIBS $LIBS"
   fi
fi

AM_CONDITIONAL([X11], [test "$have_libx11" = 'yes'])

###### Check for pkg-config  ########
AC_PATH_PROGS([PACKAGE_CONFIG], [pkg-config], [none])
if test "x$PACKAGE_CONFIG" = "xnone"; then
	
	AC_MSG_ERROR([*** pkg-config, needed to check for libxml++ existence
has not been found.])

fi


###### Check for sdl-config  ########
AC_PATH_PROGS([SDL_CONFIG], [sdl-config], [none])

if test "x$SDL_CONFIG" = "xnone"; then	

	AC_MSG_ERROR([*** sdl-config has not been found.])

fi

###### Check for paragui-config  ########
AC_PATH_PROGS([PGUI_CONFIG], [paragui-config], [none])

if test "x$PGUI_CONFIG" = "xnone"; then	

	AC_MSG_ERROR([*** paragui-config has not been found.])

fi

AC_MSG_CHECKING(for libxml++)

if $PACKAGE_CONFIG libxml++-1.0 --libs > /dev/null 2>&1; then
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
	AC_MSG_ERROR([*** libxml++ version 1.0 not found!])
fi


AC_MSG_CHECKING(for SDL)

if $SDL_CONFIG --libs > /dev/null 2>&1; then
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
	AC_MSG_ERROR([*** SDL not found!])
fi

AC_MSG_CHECKING(for Paragui)

if $PGUI_CONFIG --libs > /dev/null 2>&1; then
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
	AC_MSG_ERROR([*** Paragui not found!])
fi

#######################################################################
# Checks for types.                                                   #
#######################################################################

# No need to chech anymore    
#AC_LANG([C++])
#AC_MSG_CHECKING([if char is signed])

#AC_COMPILE_IFELSE(
#	[[static int test_array [1 - 2 * !(((char) -1) < 0)];]],
#	[AC_MSG_RESULT(yes)],
#	[AC_MSG_RESULT(no)] 
#	[AC_MSG_ERROR([*** char is unsigned. Wormux cannot work with unsigned 
#chars. Try with another compiler (specify it using CXX environment variable.)
#If you use GCC, you can force signed chars with CXXFLAGS=-fsigned-char])])

#######################################################################
# Checks for libraries.                                               #
#######################################################################

LIBXMLPP_LIBS=`$PACKAGE_CONFIG libxml++-1.0 --libs`
LIBXMLPP_LIBS="$LIBXMLPP_LIBS"
SDL_LIBS=`$SDL_CONFIG --libs`
SDL_LIBS="$SDL_LIBS -lSDL_mixer -lSDL_image -lSDL_ttf -lSDL_gfx"
PARAGUI_LIBS=`$PGUI_CONFIG --libs`

#### Paragui 1.0 ####
#g++ library flags:
#PARAGUI_LIBS="-L/usr/lib/ -lparagui-1.0"

AC_SUBST([LIBXMLPP_LIBS])
AC_SUBST([PARAGUI_LIBS])
AC_SUBST([SDL_LIBS])

#######################################################################
# Checks for header files.                                            #
#######################################################################

AC_HEADER_DIRENT
AC_HEADER_STDC

OLD_CPPFLAGS=$CPPFLAGS
OLD_CXXFLAGS=$CXXFLAGS

LIBXMLPP_CFLAGS=`$PACKAGE_CONFIG libxml++-1.0 --cflags`
LIBXMLPP_CFLAGS="$LIBXMLPP_CFLAGS"
SDL_CFLAGS=`$SDL_CONFIG --cflags`
PARAGUI_CFLAGS=`$PGUI_CONFIG --cflags`
CPPFLAGS="$CPPFLAGS $LIBXMLPP_CFLAGS"
CXXFLAGS="$CXXFLAGS $LIBXMLPP_CFLAGS"

#### Paragui 1.0 ####
#g++ incldue flags
#PARAGUI_CFLAGS="-I/usr/include/paragui-1.0"


AC_SUBST(SDL_CFLAGS)
AC_SUBST(LIBXMLPP_CFLAGS)
AC_SUBST(PARAGUI_CFLAGS)

#AC_CHECK_HEADER([ClanLib/core.h],
#		[],
#		[AC_MSG_ERROR([*** ClanLib include files not found!
#You should install ClanLib development package.])])

CPPFLAGS=$OLD_CPPFLAGS
CXXFLAGS=$OLD_CXXFLAGS

#AC_CHECK_HEADERS([stdlib.h unistd.h])


#######################################################################
# Checks for typedefs, structures, and compiler characteristics.      #
#######################################################################

#AC_HEADER_STDBOOL
#AC_C_CONST
#AC_C_INLINE
#AC_TYPE_MODE_T
#AC_TYPE_SIZE_T
#AC_STRUCT_TM

# Checks for library functions.
#AC_FUNC_ERROR_AT_LINE
#AC_FUNC_STAT
#AC_FUNC_STRFTIME
#AC_CHECK_FUNCS([floor socket strtoul])

#######################################################################
# Data file substitution.                                             #
#######################################################################

DATA_FILES=`cd data ; find . -name CVS -prune -o -name ".svn*" -o -name "Makefile*" -o -name ".#*" -o -type d -o -print`
DATA_FILES=`echo $DATA_FILES`

AC_SUBST([DATA_FILES])

AM_PO_SUBDIRS

AC_CONFIG_FILES([Makefile
		 data/Makefile
		 src/Makefile
		 ])

AC_CONFIG_FILES([po/Makefile], [AM_POSTPROCESS_PO_MAKEFILE])

AC_OUTPUT

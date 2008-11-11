/* src/msvc/config.h copied from a MINGW-generated src/config.h  */

/* Define to 1 if translation of program messages to the user's native
   language is requested. */
#define ENABLE_NLS 1

/* GNU dcgettext() function is already present or preinstalled. */
#define HAVE_DCGETTEXT 1

/* GNU gettext() function is already present or preinstalled. */
#define HAVE_GETTEXT 1

/* iconv() function. */
#include <iconv.h>
#define HAVE_ICONV 1

/* Define to 1 if you have a functional curl library. */
#define HAVE_LIBCURL 1

/* `SDL_gfx' library (-lSDL_gfx). */
#define HAVE_LIBSDL_GFX 1

/* `SDL_image' library (-lSDL_image). */
#define HAVE_LIBSDL_IMAGE 1

/* `SDL_mixer' library (-lSDL_mixer). */
#define HAVE_LIBSDL_MIXER 1

/* `SDL_net' library (-lSDL_net). */
#define HAVE_LIBSDL_NET 1

/* `SDL_ttf' library (-lSDL_ttf). */
#define HAVE_LIBSDL_TTF 1

/* <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* <SDL_rotozoom.h> header file. */
#define HAVE_SDL_ROTOZOOM_H 1

/* <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* <string.h> header file. */
#define HAVE_STRING_H 1

/* <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Manual addition */
#define CURL_STATICLIB 1

/* Defined if libcurl supports AsynchDNS */
/* #undef LIBCURL_FEATURE_ASYNCHDNS */

/* Defined if libcurl supports IDN */
/* #undef LIBCURL_FEATURE_IDN */

/* Defined if libcurl supports IPv6 */
/* #undef LIBCURL_FEATURE_IPV6 */

/* Defined if libcurl supports KRB4 */
/* #undef LIBCURL_FEATURE_KRB4 */

/* Defined if libcurl supports libz */
#define LIBCURL_FEATURE_LIBZ 1

/* Defined if libcurl supports NTLM */
/* #undef LIBCURL_FEATURE_NTLM */

/* Defined if libcurl supports SSL */
/* #undef LIBCURL_FEATURE_SSL */

/* Defined if libcurl supports SSPI */
/* #undef LIBCURL_FEATURE_SSPI */

/* Defined if libcurl supports DICT */
#define LIBCURL_PROTOCOL_DICT 1

/* Defined if libcurl supports FILE */
#define LIBCURL_PROTOCOL_FILE 1

/* Defined if libcurl supports FTP */
#define LIBCURL_PROTOCOL_FTP 1

/* Defined if libcurl supports FTPS */
/* #undef LIBCURL_PROTOCOL_FTPS */

/* Defined if libcurl supports HTTP */
#define LIBCURL_PROTOCOL_HTTP 1

/* Defined if libcurl supports HTTPS */
/* #undef LIBCURL_PROTOCOL_HTTPS */

/* Defined if libcurl supports LDAP */
#define LIBCURL_PROTOCOL_LDAP 1

/* Defined if libcurl supports TELNET */
#define LIBCURL_PROTOCOL_TELNET 1

/* Defined if libcurl supports TFTP */
#define LIBCURL_PROTOCOL_TFTP 1

/* Name of package */
#define PACKAGE "wormux"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "wormux-dev@gna.org"

/* Define to the full name of this package. */
#define PACKAGE_NAME "Wormux"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "Wormux 0.8.1svn"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "wormux"

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.8.1svn"

/* ANSI C header files. */
#define STDC_HEADERS 1

/* Define to use FriBidi */
#define USE_FRIBIDI 1

/* Version number of package */
#define VERSION "0.8.1svn"

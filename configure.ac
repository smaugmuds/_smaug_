#                     R E A L M S    O F    D E S P A I R  !
#   ___________________________________________________________________________
#  //            /                                                            \\
# [|_____________\   ********   *        *   ********   *        *   *******   |]
# [|   \\._.//   /  **********  **      **  **********  **      **  *********  |]
# [|   (0...0)   \  **********  ***    ***  **********  ***    ***  *********  |]
# [|    ).:.(    /  ***         ****  ****  ***    ***  ***    ***  ***        |]
# [|    {o o}    \  *********   **********  **********  ***    ***  *** ****   |]
# [|   / ' ' \   /   *********  *** ** ***  **********  ***    ***  ***  ****  |]
# [|-'- /   \ -`-\         ***  ***    ***  ***    ***  ***    ***  ***   ***  |]
# [|   .VxvxV.   /   *********  ***    ***  ***    ***  **********  *********  |]
# [|_____________\  **********  **      **  **      **  **********  *********  |]
# [|             /  *********   *        *  *        *   ********    *******   |]
#  \\____________\____________________________________________________________//
#     |                                                                     |
#     |    --{ [S]imulated [M]edieval [A]dventure Multi[U]ser [G]ame }--    |
#     |_____________________________________________________________________|
#     |                                                                     |
#     |                         -*- configure.ac -*-                        |
#     |                                                                     |
#     |   Process this file with autoconf to produce a configure script     |
#     |                                                                     |
#     |_____________________________________________________________________|
#    //                                                                     \\
#   [|  SMAUG 1.4 © 1994-1998 Thoric/Altrag/Blodkai/Narn/Haus/Scryn/Rennard  |]
#   [|  Swordbearer/Gorog/Grishnakh/Nivek/Tricops/Fireblade/Edmond/Conran    |]
#   [|                                                                       |]
#   [|  Merc 2.1 Diku Mud improvments © 1992-1993 Michael Chastain, Michael  |]
#   [|  Quan, and Mitchell Tse. Original Diku Mud © 1990-1991 by Sebastian   |]
#   [|  Hammer, Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, Katja    |]
#   [|  Nyboe. Win32 port Nick Gammon.                                       |]
#   [|                                                                       |]
#   [|  SMAUG 2.0 © 2014-2015 Antonio Cao (@burzumishi)                      |]
#    \\_____________________________________________________________________//


AC_PREREQ([2.69])
LT_PREREQ([2.4])  # require libtool

AC_SUBST([PACKAGE], ["smaug"])

AC_SUBST([SMAUG_MAJOR_VERSION], [2])
AC_SUBST([SMAUG_MINOR_VERSION], [0])
AC_SUBST([SMAUG_UPDATE_VERSION], [0])

AC_SUBST([VERSION], ["${SMAUG_MAJOR_VERSION}.${SMAUG_MINOR_VERSION}.${SMAUG_UPDATE_VERSION}"])

AC_INIT([smaug], [2.0.0],
	[<@burzumishi>],
	[smaug])

AC_COPYRIGHT(["2014-2015 (c) Antonio Cao (@burzumishi)"])

LT_INIT
LT_LANG([C])

AM_PROG_LIBTOOL
AM_SANITY_CHECK
AM_INIT_AUTOMAKE

AC_CONFIG_HEADERS([src/smaug.h])
AC_CONFIG_MACRO_DIR([m4])

AC_CANONICAL_HOST
AC_CANONICAL_BUILD

AC_DEFINE_UNQUOTED([PREFIX], ["/opt/smaug/"], [Define to the root directory.])
AC_DEFINE_UNQUOTED([RUNDIR], ["${prefix}/com/smaug/"], [Define modifiable architecture-independent data.])
AC_DEFINE_UNQUOTED([LOGDIR], ["${prefix}/var/log/smaug/"], [Define to log directory.])
AC_DEFINE_UNQUOTED([BINDIR], ["${prefix}/bin/"], [Define HotBoot Executable Directory.])
AC_DEFINE_UNQUOTED([LOCKDIR], ["${prefix}/var/lock/smaug/"], [Define lock file directory.])

AC_SUBST([DESCRIPTION], ["--{smaug}-- MUD Server"])

AC_LANG([C])
AC_LANG_PUSH([C])

AC_PROG_CC([cc])
AC_PROG_CXX([g++])
AC_PROG_LIBTOOL
AC_PROG_LEX
AC_PROG_AWK
AC_PROG_GREP
AC_PROG_SED
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_MKDIR_P

AC_FUNC_MALLOC

AC_SUBST(LIBTOOL_DEPS)

# Checks for libraries.
AC_CHECK_LIB([crypt], [encrypt])
AM_CONDITIONAL([HAVE_CRYPT], [test -n "${crypt}"])

AC_CHECK_LIB([dl], [dlopen])
AM_CONDITIONAL([HAVE_DL], [test -n "${dl}"])

AC_CHECK_LIB([z], [zlibVersion])
AM_CONDITIONAL([HAVE_Z], [test -n "${z}"])

AC_CHECK_LIB([m], [isnan])
AM_CONDITIONAL([HAVE_M], [test -n "${m}"])

# AC_CHECK_LIB([socket], [shutdown])
# AM_CONDITIONAL([HAVE_SOCKET], [test -n "${socket}"])

AC_CHECK_LIB([ssl], [main])
AM_CONDITIONAL([HAVE_SSL], [test -n "${ssl}"])

AC_CHECK_LIB([pthread], [main])
AM_CONDITIONAL([HAVE_PTHREAD], [test -n "${pthread}"])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h malloc.h math.h etdb.h netinet/in.h sys/param.h sys/socket.h sys/time.h unistd.h pthread.h openssl/ssl.h limits.h stddef.h zlib.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UID_T
AC_TYPE_INT64_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_STRUCT_TIMEZONE

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MKTIME
AC_FUNC_REALLOC

AC_CHECK_FUNCS([alarm dup2 gethostbyaddr gethostbyname gettimeofday inet_ntoa isascii memmove memset pow putenv select socket sqrt strcasecmp strchr strerror strpbrk strrchr strstr tzset strtol getcwd memchr mkdir rmdir strcspn strdup strspn])

AC_PATH_PROG([BASH], [bash])
AM_CONDITIONAL([HAVE_BASH], [test -n "${BASH}"])

AC_PATH_PROG([ENV], [env])
AM_CONDITIONAL([HAVE_ENV], [test -n "${ENV}"])

AC_PATH_PROG([GNUMAKE], [make])
AM_CONDITIONAL([HAVE_GNUMAKE], [test -n "${GNUMAKE}"])

AC_PATH_PROG([PERL], [perl])
AM_CONDITIONAL([HAVE_PERL], [test -n "${PERL}"])

AC_ARG_ENABLE(profiling,
              AC_HELP_STRING([--enable-profiling],
                             [enable profiling support @<:@default=no@:>@]),
              [], [enable_profiling=no])
if test "$enable_profiling" = "yes"; then
    PROFILING_CFLAGS="$CFLAGS -pg -g3"
else
    PROFILING_CFLAGS="-fomit-frame-pointer"
fi

AC_CONFIG_FILES([Makefile
		 src/Makefile
		 bin/Makefile
		 etc/Makefile
		 etc/init.d/Makefile])

LT_OUTPUT
AC_OUTPUT


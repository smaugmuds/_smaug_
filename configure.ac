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
#     |_____________________________________________________________________|
#    //                                                                     \\
#   [|  SMAUG 2.0 © 2014-2015 Antonio Cao (@burzumishi)                      |]
#   [|                                                                       |]
#   [|  AFKMud Copyright 1997-2007 by Roger Libiez (Samson),                 |]
#   [|  Levi Beckerson (Whir), Michael Ward (Tarl), Erik Wolfe (Dwip),       |]
#   [|  Cameron Carroll (Cam), Cyberfox, Karangi, Rathian, Raine,            |]
#   [|  Xorith, and Adjani.                                                  |]
#   [|  All Rights Reserved. External contributions from Remcon, Quixadhal,  |]
#   [|  Zarius and many others.                                              |]
#   [|                                                                       |]
#   [|  SMAUG 1.4 © 1994-1998 Thoric/Altrag/Blodkai/Narn/Haus/Scryn/Rennard  |]
#   [|  Swordbearer/Gorog/Grishnakh/Nivek/Tricops/Fireblade/Edmond/Conran    |]
#   [|                                                                       |]
#   [|  Merc 2.1 Diku Mud improvments © 1992-1993 Michael Chastain, Michael  |]
#   [|  Quan, and Mitchell Tse. Original Diku Mud © 1990-1991 by Sebastian   |]
#   [|  Hammer, Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, Katja    |]
#   [|  Nyboe. Win32 port Nick Gammon.                                       |]
#    \\_____________________________________________________________________//

AC_PREREQ([2.69])
AC_INIT([smaug], [2.0.0],
	[<@burzumishi>],
	[smaug])

AC_COPYRIGHT(["2014-2016 (c) Antonio Cao (@burzumishi)"])

AC_CONFIG_AUX_DIR([.])

AC_REVISION([$Revision: 1.20 $])

AC_SUBST([DESCRIPTION], ["--{ SMAUG II }-- MUD Server"])

AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_HEADERS([src/smaug.h])

AM_INIT_AUTOMAKE([1.9 no-dist-gzip dist-bzip2 dist-xz tar-ustar -Wno-portability])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AC_CONFIG_MACRO_DIR([m4])

AM_MAINTAINER_MODE([disable])

AH_TOP([
#ifndef SMAUG_CONFIG_H__
#define SMAUG_CONFIG_H__
])

AH_BOTTOM([
#endif /* SMAUG_CONFIG_H__ */
])

AM_SANITY_CHECK

AC_CANONICAL_HOST
AC_CANONICAL_BUILD

AC_DEFINE_UNQUOTED([PREFIX], ["/opt/smaug/"], [Define to the root directory.])
AC_DEFINE_UNQUOTED([RUNDIR], ["${prefix}/com/smaug/"], [Define modifiable architecture-independent data.])
AC_DEFINE_UNQUOTED([LOGDIR], ["${prefix}/var/log/smaug/"], [Define to log directory.])
AC_DEFINE_UNQUOTED([BINDIR], ["${prefix}/bin/"], [Define HotBoot Executable Directory.])
AC_DEFINE_UNQUOTED([LOCKDIR], ["${prefix}/var/lock/smaug/"], [Define lock file directory.])

AC_LANG([C])
AC_LANG_PUSH([C])

AC_C_BIGENDIAN
AC_C_CONST

AC_PROG_CC
AC_PROG_LEX
AC_PROG_AWK
AC_PROG_GREP
AC_PROG_SED
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AM_PROG_CC_C_O
AM_PROG_AS

dnl make sure we keep ACLOCAL_FLAGS around for maintainer builds to work
AC_SUBST(ACLOCAL_AMFLAGS, "\${ACLOCAL_FLAGS}")

dnl Require Libtool
LT_PREREQ([2.4])
LT_INIT
LT_LANG([C])

AC_SUBST(LIBTOOL_DEPS)

dnl Checks for libraries.
AC_FUNC_MALLOC
AM_WITH_DMALLOC

dnl Check for libraries

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
AC_CHECK_HEADERS([arpa/inet.h arpa/telnet.h ctype.h direct.h dirent.h dlfcn.h endian.h errno.h fcntl.h io.h libintl.h limits.h locale.h malloc.h netdb.h netinet/in.h netinet/in_systm.h netinet/ip.h process.h re_comp.h regex.h signal.h stdarg.h stdio.h stdlib.h string.h strings.h sys/cdefs.h sys/endian.h sys/param.h sys/socket.h sys/stat.h sys/time.h sys/types.h sys/wait.h time.h types.h unistd.h windows.h winsock.h])

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

AC_C_CONST
AC_C_INLINE

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MKTIME
AC_FUNC_REALLOC

AC_CHECK_FUNCS([bzero gethostname re_comp setlocale textdomain bindtextdomain alarm dup2 gethostbyaddr gethostbyname gettimeofday inet_ntoa isascii memmove memset pow putenv select socket sqrt strcasecmp strchr strerror strpbrk strrchr strstr tzset strtol getcwd memchr mkdir rmdir strcspn strdup strspn])

dnl Language Support
IT_PROG_INTLTOOL([0.50.1])

dnl Check for gettext
GETTEXT_PACKAGE=smaug-2.0
AM_GNU_GETTEXT([external])
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [The gettext translation domain])
AC_SUBST(GETTEXT_PACKAGE)

dnl Dependency Checks
PKG_PROG_PKG_CONFIG()

# Check for doxygen (optional) #
AC_CHECK_PROGS([DOXYGEN], [doxygen])
AM_CONDITIONAL([HAVE_DOXYGEN], [test -n "${DOXYGEN}"])

# Check for bash (required) #
AC_PATH_PROG([BASH], [bash])
AM_CONDITIONAL([HAVE_BASH], [test -n "${BASH}"])

AC_CHECK_PROG(BASH, [bash], [yes], [no])
if test "$BASH" = "no"; then
  AC_MSG_ERROR([Unable to find the bash command.])
fi

# Check for env (required) #
AC_PATH_PROG([ENV], [env])
AM_CONDITIONAL([HAVE_ENV], [test -n "${ENV}"])

AC_CHECK_PROG(ENV, [env], [yes], [no])
if test "$ENV" = "no"; then
  AC_MSG_ERROR([Unable to find the env command.])
fi

# Check for make (required) #
AC_PATH_PROG([GNUMAKE], [make])
AM_CONDITIONAL([HAVE_GNUMAKE], [test -n "${GNUMAKE}"])

AC_CHECK_PROG(GNUMAKE, [make], [yes], [no])
if test "$GNUMAKE" = "no"; then
  AC_MSG_ERROR([Unable to find the make command.])
fi

# Check for perl (required) #
AC_PATH_PROG([PERL], [perl])
AM_CONDITIONAL([HAVE_PERL], [test -n "${PERL}"])

AC_CHECK_PROG(PERL, [perl], [yes], [no])
if test "$PERL" = "no"; then
  AC_MSG_ERROR([Unable to find the perl command.])
fi

## CPU architecture specific assembly

build_cpu_mmx="no"
build_cpu_sse3="no"
build_cpu_altivec="no"
build_cpu_neon="no"

want_neon="yes"
AC_ARG_ENABLE([neon],
   [AS_HELP_STRING([--disable-neon],[disable neon support @<:@default=enable@:>@])],
   [
    if test "x${enableval}" = "xyes"; then
       want_neon="yes"
    else
       want_neon="no"
    fi
   ])

SSE3_CFLAGS=""
ALTIVEC_CFLAGS=""
NEON_CFLAGS=""

case $host_cpu in
  i*86|x86_64|amd64)
    AC_DEFINE([BUILD_MMX], [1], [Build MMX Code])
    build_cpu_mmx="yes"
    AC_CHECK_HEADER([immintrin.h],
       [
        AC_DEFINE(BUILD_SSE3, 1, [Build SSE3 Code])
        build_cpu_sse3="yes"
       ],
       [build_cpu_sse3="no"])
    AC_MSG_CHECKING([whether to build SSE3 code])
    AC_MSG_RESULT([${build_cpu_sse3}])

    if test "x$build_cpu_sse3" = "xyes" ; then
       SSE3_CFLAGS="-msse3"
    fi
    ;;
  *power* | *ppc*)
    build_cpu_altivec="yes"
    AC_CHECK_HEADER([altivec.h],
       [
        AC_DEFINE([BUILD_ALTIVEC], [1], [Build Altivec Code])
        AC_DEFINE([HAVE_ALTIVEC_H], [1], [Have altivec.h header file])
        build_cpu_altivec="yes"
       ],
       [
        save_CFLAGS=$CFLAGS
        save_CPPFLAGS=$CPPFLAGS
        CFLAGS=$CFLAGS" -maltivec"
        CPPFLAGS=$CPPFLAGS" -maltivec"
        unset ac_cv_header_altivec_h
        AC_CHECK_HEADER([altivec.h],
          [
            AC_DEFINE([BUILD_ALTIVEC], [1], [Build Altivec Code])
            AC_DEFINE([HAVE_ALTIVEC_H], [1], [Have altivec.h header file])
            build_cpu_altivec="yes"
          ],
          [build_cpu_altivec="no"]
        )
        CFLAGS=$save_CFLAGS
        CPPFLAGS=$save_CPPFLAGS
       ]
    )
    if test "x${build_cpu_altivec}" = "xyes"; then
       AC_MSG_CHECKING([whether to use altivec compiler flag])
       if test "x$GCC" = "xyes"; then
          if echo "int main(){return 0;}" | ${CPP} -faltivec - > /dev/null 2>&1; then
             altivec_cflags="-faltivec"
             AC_DEFINE([BUILD_ALTIVEC], [1], [Build Altivec Code])
          elif echo "int main(){return 0;}" | ${CPP} -maltivec - > /dev/null 2>&1; then
             altivec_cflags="-maltivec"
             AC_DEFINE([BUILD_ALTIVEC], [1], [Build Altivec Code])
          fi
       fi
       AC_MSG_RESULT([${altivec_cflags}])
       CFLAGS="$CFLAGS ${altivec_cflags}"
       ALTIVEC_CFLAGS="-maltivec"
    fi
    ;;
  arm*)
    if test "x${want_neon}" = "xyes"; then
       build_cpu_neon="yes"
       AC_MSG_CHECKING([whether to use NEON instructions])
       CFLAGS_save="${CFLAGS}"
       CFLAGS="${CFLAGS} -mfpu=neon -ftree-vectorize"
       AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <arm_neon.h>]], [[asm volatile ("vqadd.u8 d0, d1, d0\n")]])],[
           AC_MSG_RESULT([yes])
           AC_DEFINE([BUILD_NEON], [1], [Build NEON Code])
           build_cpu_neon="yes"
           NEON_CFLAGS="-mfpu=neon"
       	 ],[
	   AC_MSG_RESULT([no])
           build_cpu_neon="no"
       	 ])
       CFLAGS="${CFLAGS_save}"
    fi
    ;;
esac

AC_SUBST([ALTIVEC_CFLAGS])
AC_SUBST([SSE3_CFLAGS])
AC_SUBST([NEON_CFLAGS])

# sockets

case "$host_os" in
	*mingw*)
	  AC_MSG_NOTICE([OS MinGW detected])
      AC_DEFINE_UNQUOTED(MINGW,1,[This is a MinGW system])
      AC_DEFINE_UNQUOTED(WIN32,1,[This is a Windows system])
      AC_DEFINE_UNQUOTED(WINDOWS,1,[This is a Windows system])
      LIBS="$LIBS -lws2_32"
      AC_CHECK_HEADERS([winsock2.h ws2tcpip.h],, AC_MSG_ERROR([[Winsock2 headers are required for W32]]))
      AC_CACHE_CHECK([for MS lib utility], [ac_cv_use_ms_lib_tool],
       [[mslibcheck=`lib 2>&1`
        if [[ $mslibcheck = "Microsoft (R) Library Manager"* ]]; then
          ac_cv_use_ms_lib_tool="yes"
        else
          ac_cv_use_ms_lib_tool="no"
        fi
         ]])
      if test "x$ac_cv_use_ms_lib_tool" = "xyes"; then
        AC_SUBST([MS_LIB_TOOL], [[lib]])
      fi
      AC_SUBST([lt_cv_objdir])
      os_is_windows="yes"
      os_is_native_w32="yes"
      have_socket="no"
	  enable_crypt="no"
      ;;
	freebsd*)
	  AC_MSG_NOTICE([OS FreeBSD detected])
      AC_DEFINE_UNQUOTED(SOMEBSD,1,[This is a BSD system])
      AC_DEFINE_UNQUOTED(FREEBSD,1,[This is a FreeBSD system])
      ;;
	openbsd*)
	  AC_MSG_NOTICE([OS OpenBSD detected])
      AC_DEFINE_UNQUOTED(SOMEBSD,1,[This is a BSD system])
      AC_DEFINE_UNQUOTED(OPENBSD,1,[This is an OpenBSD system])
      ;;
	netbsd*)
	  AC_MSG_NOTICE([OS NetBSD detected])
      AC_DEFINE_UNQUOTED(SOMEBSD,1,[This is a BSD system])
      AC_DEFINE_UNQUOTED(NETBSD,1,[This is a NetBSD system])
      ;;

	*solaris*)
	  AC_MSG_NOTICE([OS Solaris detected])
      AC_DEFINE_UNQUOTED(SOLARIS,1,[This is a Solaris system])
      AC_DEFINE_UNQUOTED(_REENTRANT,1,[Need with solaris or errno doesnt work])
      AC_SEARCH_LIBS(gethostbyname, nsl)
      AC_SEARCH_LIBS(socket, socket)
      AC_CHECK_LIB([socket], [connect],
         [
          have_socket="yes"
          requirement_elm_libs="-lsocket ${requirement_elm_libs}"
         ],
         [have_socket="no"])
      ;;
    *darwin* | *rhapsody* | *macosx*)
	  AC_MSG_NOTICE([OS MacOSX detected])
      AC_DEFINE_UNQUOTED(OSX,1,[This is an OS X system])
      CFLAGS="-no-cpp-precomp -fno-common $CFLAGS"
      have_socket="yes"
      ;;
	*arm-linux*)
	  AC_MSG_NOTICE([OS ARM GNU/Linux detected])
      AC_DEFINE_UNQUOTED(LINUX,1,[This is a Linux kernel])
      AC_DEFINE_UNQUOTED(HAVE_LISTEN_SHUTDOWN,1,[can use shutdown on listen sockets])
      CFLAGS="-fPIC -pipe $CFLAGS"
      ;;
	*linux*)
	  AC_MSG_NOTICE([OS GNU/Linux detected])
      AC_DEFINE_UNQUOTED(LINUX,1,[This is a Linux kernel])
      AC_DEFINE_UNQUOTED(HAVE_LISTEN_SHUTDOWN,1,[can use shutdown on listen sockets])
      ;;
	*cygwin*)
	  AC_MSG_NOTICE([OS Cygwin detected])
      AC_DEFINE_UNQUOTED(__CYGWIN__,1,[This is a Cygwin system])
      os_is_windows="yes"
      have_socket="no"
	  enable_crypt="no"
      ;;
    *openedition*)
	  AC_MSG_NOTICE([OS Open Edition detected])
      AC_DEFINE_UNQUOTED(OS390,1,[This is a OS/390 system])
      ;;
   *)
      AC_MSG_WARN([Unrecognised OS $host_os])
      AC_DEFINE_UNQUOTED(OTHEROS,1,[Some strange OS])
      have_socket="yes"
      ;;
esac

AM_CONDITIONAL([BUILD_RUN], [test "x$have_socket" = "xyes"])

### SMAUG BASIC FEATURES ###

# Planes Support (-DPLANES) #
AC_ARG_ENABLE(planes,
              [AS_HELP_STRING([--enable-planes],
              				[enable Planes support @<:@default=yes@:>@])],,
              				[enable_planes=yes])
if test "$enable_planes" = "yes"; then
		AC_DEFINE([PLANES], [], [Planes Support])
fi

# Require Who Command Argument (-DREQWHOARG) #
AC_ARG_ENABLE(whoarg,
              [AS_HELP_STRING([--enable-whoarg],
                            [require Who command arguments @<:@default=no@:>@])],,
              				[enable_whoarg=no])
if test "$enable_whoarg" = "yes"; then
		AC_DEFINE([REQWHOARG], [], [Who Arguments Support])
fi

# Honour Code Support (-DHONOUR_CODE) #
AC_ARG_ENABLE(honour,
              [AS_HELP_STRING([--enable-honour],
                            [enable Honour Code support @<:@default=yes@:>@])],,
              				[enable_honour=yes])
if test "$enable_honour" = "yes"; then
		AC_DEFINE([HONOUR_CODE], [], [Honour Code Support])
fi

# Housing Support (-DHOUSING_CODE) #
AC_ARG_ENABLE(housing,
              [AS_HELP_STRING([--enable-housing],
                            [enable Housing support @<:@default=yes@:>@])],,
              				[enable_housing=yes])
if test "$enable_housing" = "yes"; then
		AC_DEFINE([HOUSING_CODE], [], [Housing Support])
fi

# House mobs Support (-DHOUSE_MOBS) #
AC_ARG_ENABLE(housemobs,
              [AS_HELP_STRING([--enable-housemobs],
                            [enable House Mobs support @<:@default=yes@:>@])],,
              				[enable_housemobs=yes])
if test "$enable_housemobs" = "yes"; then
		AC_DEFINE([HOUSE_MOBS], [], [House Mobs Support])
fi

# Enable Extended Rooms (-DEXTENDED_ROOMS) #
AC_ARG_ENABLE(extrooms,
              [AS_HELP_STRING([--enable-extrooms],
                            [enable Extended Rooms Support @<:@default=yes@:>@])],,
              				[enable_extrooms=yes])
if test "$enable_extrooms" = "yes"; then
		AC_DEFINE([EXTENDED_ROOMS], [], [Extended Rooms Support])
fi

# act_comm.c SCRAMBLE Support (testing) (-DSCRAMBLE) #
AC_ARG_ENABLE(scramble,
              [AS_HELP_STRING([--enable-scramble],
                            [enable Act Scramble Support (testing) @<:@default=no@:>@])],,
              				[enable_scramble=no])
if test "$enable_scramble" = "yes"; then
		AC_DEFINE([SCRAMBLE], [], [ActComm Scramble Support])
fi

# Liquids & Mixtures System Support (-DLIQUIDSYSTEM) #
# Replaces: "do_drink", "do_fill", "do_empty" functions from "misc.c".
AC_ARG_ENABLE(liquids,
              [AS_HELP_STRING([--enable-liquids],
                            [enable Liquids & Mixtures System @<:@default=yes@:>@])],,
              				[enable_liquids=yes])
if test "$enable_liquids" = "yes"; then
		AC_DEFINE([LIQUIDSYSTEM], [], [Liquids & Mixtures System])
fi

# TIME Format Support (-DTIMEFORMAT) #
# Enable if you are getting a line like: interp.c:757: warning: int format, time_t arg (arg 7)
AC_ARG_ENABLE(time,
              [AS_HELP_STRING([--enable-time],
                            [enable Time Format if you are getting warning int format @<:@default=no@:>@])],,
              				[enable_time=no])
if test "$enable_time" = "yes"; then
		AC_DEFINE([TIMEFORMAT], [], [Time Format warnings])
fi

# REGEX Support (-DREGEX) #
# Enable if you are getting implicit decleration of re_exec
# AC_ARG_ENABLE(regex,
#               [AS_HELP_STRING([--enable-regex],
#                             [enable if you are getting implicit decleration of re_exec @<:@default=yes@:>@])],,
#               				[enable_regex=yes])
# if test "$enable_regex" = "yes"; then
# 		# Manage undefined re_exec errors
# # 		AC_DEFINE([REGEX], [], [Regex Support])
# 		LDFLAGS="$LDFLAGS -lgnuregex"
# fi

# Enable Password Encription Support (-DCRYPT) #
AC_ARG_ENABLE(crypt,
              [AS_HELP_STRING([--enable-crypt],
                            [enable Password Encryption support @<:@default=yes@:>@])],,
              				[enable_crypt=yes])
if test "$enable_crypt" = "yes"; then
		AC_DEFINE([CRYPT], [], [Password Encryption Support])
		LDFLAGS="$LDFLAGS -lcrypt"
else
		AC_DEFINE([NOCRYPT], [], [Disable Password Encryption Support])
fi

# REQUEST Support (-DREQUESTS) #
AC_ARG_ENABLE(request,
              [AS_HELP_STRING([--enable-request],
                            [enable Request Support @<:@default=no@:>@])],,
              				[enable_request=no])
if test "$enable_request" = "yes"; then
		AC_DEFINE([REQUESTS], [], [Requests Support])
fi

# Profiling Support #
AC_ARG_ENABLE(profiling,
              [AS_HELP_STRING([--enable-profiling],
                            [enable profiling support @<:@default=no@:>@])],,
              				[enable_profiling=no])
if test "$enable_profiling" = "yes"; then
		CFLAGS="$CFLAGS -pg -g3"
else
		CFLAGS="$CFLAGS -fomit-frame-pointer"
fi

### SMAUG BUILD FLAGS ###

# Performance CFLAGS #
AC_ARG_ENABLE(performance,
              [AS_HELP_STRING([--enable-performance],
                            [enable Performance Flags @<:@default=yes@:>@])],,
              				[enable_performance=yes])
if test "$enable_performance" = "yes"; then
		LDFLAGS="$LDFLAGS -finline-functions -funroll-loops -fdefer-pop -fstrength-reduce"
fi

# Developer CFLAGS #
AC_ARG_ENABLE(devel,
              [AS_HELP_STRING([--enable-devel],
                            [enable Developer Build Flags @<:@default=no@:>@])],,
              				[enable_devel=no])
if test "$enable_devel" = "yes"; then
		CFLAGS="$CFLAGS -Wall -Wshadow -Wformat-security -Wpointer-arith -Wcast-align -Wredundant-decls -Wuninitialized"
else
		CFLAGS="$CFLAGS -Wno-all -Wno-cast-align -Wno-unused-result -Wno-write-strings -Wno-unused-but-set-variable -Wno-pointer-to-int-cast -Wno-address -Wno-int-to-pointer-cast -Wno-uninitialized"
fi

### SMAUG 2.0 EXTRA FEATURES ###

# SMAUG 2.0 Suport (-DSMAUG -DSMAUG2) #
AC_ARG_ENABLE(smaug2,
              [AS_HELP_STRING([--enable-smaug2],
                            [enable SMAUG 2.0 support @<:@default=yes@:>@])],,
              				[enable_smaug2=yes])
if test "$enable_smaug2" = "yes"; then
		AC_DEFINE([SMAUG2], [], [SMAUG Support])
fi
AM_CONDITIONAL([SMAUG2], [test "$enable_smaug2" = "yes"])

# Enable OasisOLC Support (-DENABLE_OLC2) #
AC_ARG_ENABLE(olc2,
              [AS_HELP_STRING([--enable-olc2],
                            [enable OasisOLC Support @<:@default=yes@:>@])],,
              				[enable_olc2=yes])
if test "$enable_olc2" = "yes"; then
		AC_DEFINE([ENABLE_OLC2], [], [OasisOLC Support])
fi
AM_CONDITIONAL([ENABLE_OLC2], [test "$enable_olc2" = "yes"])

# Enable OasisOLC Extras Support (-DENABLE_OLC2_EXTRAS) #
AC_ARG_ENABLE(olc2extras,
              [AS_HELP_STRING([--enable-olc2extras],
                            [enable OasisOLC Extras Support @<:@default=yes@:>@])],,
              				[enable_olc2extras=yes])
if test "$enable_olc2extras" = "yes"; then
		AC_DEFINE([ENABLE_OLC2_EXTRAS], [], [OasisOLC Extras Support])
fi
AM_CONDITIONAL([ENABLE_OLC2_EXTRAS], [test "$enable_olc2extras" = "yes"])

# Enable Overland Support (-DOVERLANDCODE) #
AC_ARG_ENABLE(overland,
              [AS_HELP_STRING([--enable-overland],
                            [enable Overland Support @<:@default=no@:>@])],,
              				[enable_overland=no])
if test "$enable_overland" = "yes"; then
		AC_DEFINE([OVERLANDCODE], [], [Overland Support])
fi
AM_CONDITIONAL([OVERLANDCODE], [test "$enable_overland" = "yes"])

# Enable BuildWalk Support (-DENABLE_BUILDWALK) #
AC_ARG_ENABLE(buildwalk,
              [AS_HELP_STRING([--enable-buildwalk],
                            [enable BuildWalk Support @<:@default=yes@:>@])],,
              				[enable_buildwalk=yes])
if test "$enable_buildwalk" = "yes"; then
		AC_DEFINE([ENABLE_BUILDWALK], [], [BuildWalk Support])
fi
AM_CONDITIONAL([ENABLE_BUILDWALK], [test "$enable_buildwalk" = "yes"])

# Enable Billion MAX_VNUM Support (-DBILLION_VNUM) #
AC_ARG_ENABLE(bvnum,
              [AS_HELP_STRING([--enable-bvnum],
                            [enable Billion MAX_VNUM Support @<:@default=yes@:>@])],,
              				[enable_bvnum=yes])
if test "$enable_bvnum" = "yes"; then
		AC_DEFINE([BILLION_VNUM], [], [Billion MAX_VNUM Support])
fi
AM_CONDITIONAL([BILLION_VNUM], [test "$enable_bvnum" = "yes"])

# New Score Table (-DNEWSCORE) #
AC_ARG_ENABLE(newscore,
              [AS_HELP_STRING([--enable-newscore],
                             [enable New Score Tables @<:@default=yes@:>@])],,
              				 [enable_newscore=yes])
if test "$enable_newscore" = "yes"; then
		AC_DEFINE([NEWSCORE], [], [New Score Tables])
fi
AM_CONDITIONAL([NEWSCORE], [test "$enable_newscore" = "yes"])

# Player Show Stance (-DPLRSHOWSTANCE) #
AC_ARG_ENABLE(showstance,
              [AS_HELP_STRING([--enable-showstance],
                             [enable Show Stances in Player Score @<:@default=yes@:>@])],,
              				 [enable_showstance=yes])
if test "$enable_showstance" = "yes"; then
		AC_DEFINE([PLRSHOWSTANCE], [], [Show Stances Score])
fi
AM_CONDITIONAL([PLRSHOWSTANCE], [test "$enable_showstance" = "yes"])

# Enable Gold, Silver & Copper Support (-DENABLE_GOLD_SILVER_COPPER) #
AC_ARG_ENABLE(gsc,
              [AS_HELP_STRING([--enable-gsc],
                             [enable Gold, Silver & Copper Support @<:@default=yes@:>@])],,
              				 [enable_gsc=yes])
if test "$enable_gsc" = "yes"; then
		AC_DEFINE([ENABLE_GOLD_SILVER_COPPER], [], [Gold, Silver & Copper Support])
fi
AM_CONDITIONAL([ENABLE_GOLD_SILVER_COPPER], [test "$enable_gsc" = "yes"])

# Enable Banking Support (-DBANK_INSTALLED) #
AC_ARG_ENABLE(bank,
              [AS_HELP_STRING([--enable-bank],
                             [enable Banking Support @<:@default=yes@:>@])],,
              				 [enable_bank=yes])
if test "$enable_bank" = "yes"; then
		AC_DEFINE([BANK_INSTALLED], [], [Banking Support])
fi
AM_CONDITIONAL([BANK_INSTALLED], [test "$enable_bank" = "yes"])

# Enable Gold Race Rate Support (-DGOLD_MULT) #
AC_ARG_ENABLE(gmult,
              [AS_HELP_STRING([--enable-gmult],
                             [enable Gold Race Rate Support @<:@default=yes@:>@])],,
              				 [enable_gmult=yes])
if test "$enable_gmult" = "yes"; then
		AC_DEFINE([GOLD_MULT], [], [Gold Race Rate Support])
fi
AM_CONDITIONAL([GOLD_MULT], [test "$enable_gmult" = "yes"])

# Enable Marriage Support (-DMARRIAGE) #
AC_ARG_ENABLE(marry,
              [AS_HELP_STRING([--enable-marry],
                             [enable Marriage Support @<:@default=yes@:>@])],,
              				 [enable_marry=yes])
if test "$enable_marry" = "yes"; then
		AC_DEFINE([MARRIAGE], [], [Marriage Support])
fi
AM_CONDITIONAL([MARRIAGE], [test "$enable_marry" = "yes"])

# Enable Morgue Support (-DENABLE_MORGUE) #
AC_ARG_ENABLE(morgue,
              [AS_HELP_STRING([--enable-morgue],
                             [enable Morgue Support @<:@default=yes@:>@])],,
          					 [enable_morgue=yes])
if test "$enable_morgue" = "yes"; then
		AC_DEFINE([ENABLE_MORGUE], [], [Morgue Support])
fi

# Enable Undertaker Support (-DENABLE_UNDERTAKER) #
AC_ARG_ENABLE(undertaker,
              [AS_HELP_STRING([--enable-undertaker],
                             [enable Undertaker Support @<:@default=yes@:>@])],,
              				 [enable_undertaker=yes])
if test "$enable_undertaker" = "yes"; then
		AC_DEFINE([ENABLE_UNDERTAKER], [], [Undertaker Support])
fi
AM_CONDITIONAL([ENABLE_UNDERTAKER], [test "$enable_undertaker" = "yes"])

# Enable Dice Game Support (-DENABLE_DICE) #
AC_ARG_ENABLE(dice,
              [AS_HELP_STRING([--enable-dice],
                             [enable Dice Game Support @<:@default=yes@:>@])],,
              				 [enable_dice=yes])
if test "$enable_dice" = "yes"; then
		AC_DEFINE([ENABLE_DICE], [], [Dice Game Support])
fi
AM_CONDITIONAL([ENABLE_DICE], [test "$enable_dice" = "yes"])

# Player Bleeding (-DBLEEDING) #
AC_ARG_ENABLE(bleeding,
              [AS_HELP_STRING([--enable-bleeding],
                             [enable Player Bleeding @<:@default=yes@:>@])],,
              				 [enable_bleeding=yes])
if test "$enable_bleeding" = "yes"; then
		AC_DEFINE([BLEEDING], [], [Player Bleeding])
fi

# Enable Hotboot Support (-DENABLE_HOTBOOT) #
AC_ARG_ENABLE(hotboot,
              [AS_HELP_STRING([--enable-hotboot],
                             [enable Hotboot Support @<:@default=yes@:>@])],,
             				 [enable_hotboot=yes])
if test "$enable_hotboot" = "yes"; then
		AC_DEFINE([ENABLE_HOTBOOT], [], [Hotboot Support])
fi
AM_CONDITIONAL([ENABLE_HOTBOOT], [test "$enable_hotboot" = "yes"])

# Enable Command Alias Support (-DENABLE_ALIAS) #
AC_ARG_ENABLE(alias,
              [AS_HELP_STRING([--enable-alias],
                             [enable Command Alias Support @<:@default=yes@:>@])],,
              				 [enable_alias=yes])
if test "$enable_alias" = "yes"; then
		AC_DEFINE([ENABLE_ALIAS], [], [Command Alias Support])
fi
AM_CONDITIONAL([ENABLE_ALIAS], [test "$enable_alias" = "yes"])

# Enable Weapon Proficency (-DENABLE_WEAPONPROF) #
AC_ARG_ENABLE(weaponprof,
              [AS_HELP_STRING([--enable-weaponprof],
                             [enable Weapon Proficency @<:@default=yes@:>@])],,
              				 [enable_weaponprof=yes])
if test "$enable_weaponprof" = "yes"; then
		AC_DEFINE([ENABLE_WEAPONPROF], [], [Weapon Proficency Support])
fi
AM_CONDITIONAL([ENABLE_WEAPONPROF], [test "$enable_weaponprof" = "yes"])

# Enable Archery (-DENABLE_ARCHERY) #
AC_ARG_ENABLE(archery,
              [AS_HELP_STRING([--enable-archery],
                             [enable Archery @<:@default=yes@:>@])],,
             				 [enable_archery=yes])
if test "$enable_archery" = "yes"; then
		AC_DEFINE([ENABLE_ARCHERY], [], [Archery Support])
fi
AM_CONDITIONAL([ENABLE_ARCHERY], [test "$enable_archery" = "yes"])

# Enable Arena Challenge (-DENABLE_ARENA) #
AC_ARG_ENABLE(arena,
              [AS_HELP_STRING([--enable-arena],
                             [enable Arena Challenge @<:@default=yes@:>@])],,
             				 [enable_arena=yes])
if test "$enable_arena" = "yes"; then
		AC_DEFINE([ENABLE_ARENA], [], [Arena Challenge Support])
fi
AM_CONDITIONAL([ENABLE_ARENA], [test "$enable_arena" = "yes"])

# Enable Color Support (-DENABLE_COLOR) #
AC_ARG_ENABLE(color,
              [AS_HELP_STRING([--enable-color],
                             [enable Color Support @<:@default=yes@:>@])],,
              				 [enable_color=yes])
if test "$enable_color" = "yes"; then
		AC_DEFINE([ENABLE_COLOR], [], [Color Support])
fi
AM_CONDITIONAL([ENABLE_COLOR], [test "$enable_color" = "yes"])

# Enable Mapper Support (-DENABLE_MAPPER) #
AC_ARG_ENABLE(mapper,
              [AS_HELP_STRING([--enable-mapper],
                             [enable Mapper Support @<:@default=yes@:>@])],,
              				 [enable_mapper=yes])
if test "$enable_mapper" = "yes"; then
		AC_DEFINE([ENABLE_MAPPER], [], [Mapper Support])
fi
AM_CONDITIONAL([ENABLE_MAPPER], [test "$enable_mapper" = "yes"])

# Enable New Weather System (-DENABLE_WEATHER) #
AC_ARG_ENABLE(weather,
              [AS_HELP_STRING([--enable-weather],
                             [enable New Weather Support @<:@default=yes@:>@])],,
              				 [enable_weather=yes])
if test "$enable_weather" = "yes"; then
		AC_DEFINE([ENABLE_WEATHER], [], [New Weather Support])
fi
AM_CONDITIONAL([ENABLE_WEATHER], [test "$enable_weather" = "yes"])

# Enable Timezone (-DENABLE_TIMEZONE) #
AC_ARG_ENABLE(timezone,
              [AS_HELP_STRING([--enable-timezone],
                             [enable Timezone @<:@default=yes@:>@])],,
             				 [enable_timezone=yes])
if test "$enable_timezone" = "yes"; then
		AC_DEFINE([ENABLE_TIMEZONE], [], [Timezone Support])
fi
AM_CONDITIONAL([ENABLE_TIMEZONE], [test "$enable_timezone" = "yes"])

# Enable Holidays (-DENABLE_HOLIDAYS) #
AC_ARG_ENABLE(holidays,
              [AS_HELP_STRING([--enable-holidays],
                             [enable Holidays @<:@default=yes@:>@])],,
              				 [enable_holidays=yes])
if test "$enable_holidays" = "yes"; then
		AC_DEFINE([ENABLE_HOLIDAYS], [], [Holidays Support])
fi
AM_CONDITIONAL([ENABLE_HOLIDAYS], [test "$enable_holidays" = "yes"])

# Enable Quest Support (-DENABLE_QUEST) #
AC_ARG_ENABLE(quest,
              [AS_HELP_STRING([--enable-quest],
                             [enable Quest Support @<:@default=yes@:>@])],,
              				 [enable_quest=yes])
if test "$enable_quest" = "yes"; then
		AC_DEFINE([ENABLE_QUEST], [], [Quest Support])
fi
AM_CONDITIONAL([ENABLE_QUEST], [test "$enable_quest" = "yes"])

# Enable MSSP Support (-DENABLE_MSSP) #
AC_ARG_ENABLE(mssp,
              [AS_HELP_STRING([--enable-mssp],
                             [enable MSSP Support @<:@default=yes@:>@])],,
              				 [enable_mssp=yes])
if test "$enable_mssp" = "yes"; then
		AC_DEFINE([ENABLE_MSSP], [], [MSSP Support])
fi
AM_CONDITIONAL([ENABLE_MSSP], [test "$enable_mssp" = "yes"])

# Enable Web Server (-DHTTPD)
AC_ARG_ENABLE(httpd,
              [AS_HELP_STRING([--enable-httpd],
                             [enable Mongoose Web Server (Listen on port [:8080]) @<:@default=yes@:>@])],,
              				 [enable_httpd=yes])
if test "$enable_httpd" = "yes"; then
		AC_DEFINE([HTTPD], [], [Mongoose Web Server])
		AC_DEFINE([WEB_PORT], [8080], [Mongoose Web Server Port])
fi
AM_CONDITIONAL([HTTPD], [test "$enable_httpd" = "yes"])

# Enable DNS Resolver Support (-DENABLE_DNS_RESOLV) #
AC_ARG_ENABLE(dns,
              [AS_HELP_STRING([--enable-dns],
                             [enable DNS Resolver @<:@default=yes@:>@])],,
              				 [enable_dns=yes])
if test "$enable_dns" = "yes"; then
		AC_DEFINE([ENABLE_DNS_RESOLV], [], [DNS Resolver])
fi
AM_CONDITIONAL([ENABLE_DNS_RESOLV], [test "$enable_dns" = "yes"])

# Create build files #

AC_CONFIG_FILES([
bin/Makefile
etc/Makefile
etc/init.d/Makefile
src/Makefile
po/Makefile.in
smaug-2.0.pc
Makefile
Doxyfile
])
AC_OUTPUT

dnl Check gettext --from-code support
if echo foo | xgettext --from-code=UTF-8 -LC -o - - 2>/dev/null; then
:;
else
echo "Warning: Your version of gettext does not support --from-code."
echo "         This will cause translation of some strings in non US-English"
echo "         locales to fail. For full support please upgrade to GNU"
echo "         gettext 0.12 or later."
echo ""
fi

# Show configure build flags & options #

AC_MSG_RESULT
AC_MSG_RESULT([
   Configured --{$PACKAGE}-- build options:

   prefix:                    ${prefix}
   exec_prefix:               ${exec_prefix}
   libdir:                    ${libdir}
   bindir:                    ${bindir}
   sbindir:                   ${sbindir}
   sysconfdir:                ${sysconfdir}
   localstatedir:             ${localstatedir}
   datadir:                   ${datadir}
   source code location:      ${srcdir}
   compiler:                  ${CC}
   cflags:                    ${CFLAGS} $CFLAGS

   Configured --{$PACKAGE}-- build flags:
   
    * Maintainer mode:        <${USE_MAINTAINER_MODE}>
    * Time Format fix:        <$enable_time>
    * Profiling flags:        <$enable_profiling>
    * Developer flags:        <$enable_devel>
    * Performance flags:      <$enable_performance>

   Configured --{$PACKAGE}-- MUD Server features:

    * SMAUG 2.0:              <$enable_smaug2>
    * Oasis OLC & Extras:     <$enable_olc2 & $enable_olc2extras>
    * Overland:               <$enable_overland>
    * Buildwalk:              <$enable_buildwalk>
    * Billion MAX_VNUM:       <$enable_bvnum>
    * Password Encryption:    <$enable_crypt>
    * Request Support:        <$enable_request>
    * Room Planes:            <$enable_planes>
    * Who Arguments:          <$enable_whoarg>
    * MUD HotBoot:            <$enable_hotboot>
    * Command Aliases:        <$enable_alias>
    * New Score Tables:       <$enable_newscore>
    * Show Stance Info:       <$enable_showstance>
    * Player Bleeding:        <$enable_bleeding>
    * New Colors:             <$enable_color>
    * New MUD Mapper:         <$enable_mapper>
    * MSSP Support:           <$enable_mssp>
    * New Weather & Timezone: <$enable_weather & $enable_timezone>
    * Holidays:               <$enable_holidays>
    * Honour Code:            <$enable_honour>
    * Player Houses & Mobs:   <$enable_housing & $enable_housemobs>
    * Extended Rooms:         <$enable_extrooms>
    * Liquids System:         <$enable_liquids>
    * Act Comm Scramble:      <$enable_scramble>
    * Arena Challenge:        <$enable_arena>
    * Morgue & Undertaker:    <$enable_morgue & $enable_undertaker>
    * Gold/Silver/Copper:     <$enable_gsc>
    * Bank (/G/S/C):          <$enable_bank>
    * Gold Race Multirate:    <$enable_gmult>
    * Weapon Proficency:      <$enable_weaponprof>
    * Archery:                <$enable_archery>
    * Quest:                  <$enable_quest>
    * Player Marry:           <$enable_marry>
    * Dice Game:              <$enable_dice>
    * DNS Resolver:           <$enable_dns>
    * Web Server:             <$enable_httpd>
])
AC_MSG_RESULT([Well done! "configure" complete, now type "make"!])
AC_MSG_RESULT

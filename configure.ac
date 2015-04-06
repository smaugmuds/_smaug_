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

AC_CONFIG_SRCDIR([src/smaug.c])
AC_CONFIG_HEADERS([src/smaug.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE
AM_PROG_LIBTOOL
AM_SANITY_CHECK

AC_CANONICAL_HOST
AC_CANONICAL_BUILD

AC_DEFINE_UNQUOTED([PREFIX], ["/opt/smaug/"], [Define to the root directory.])
AC_DEFINE_UNQUOTED([RUNDIR], ["${prefix}/com/smaug/"], [Define modifiable architecture-independent data.])
AC_DEFINE_UNQUOTED([LOGDIR], ["${prefix}/var/log/smaug/"], [Define to log directory.])
AC_DEFINE_UNQUOTED([BINDIR], ["${prefix}/bin/"], [Define HotBoot Executable Directory.])
AC_DEFINE_UNQUOTED([LOCKDIR], ["${prefix}/var/lock/smaug/"], [Define lock file directory.])

AC_SUBST([DESCRIPTION], ["--{SMAUG}-- MUD Server"])

AC_LANG([C])
AC_LANG_PUSH([C])

AC_PROG_CC([cc])
AC_PROG_CXX([g++])

AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL
AM_PROG_CC_C_O
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
# Check for gettext #
AM_GNU_GETTEXT([external])

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

AC_CHECK_FUNCS([bzero gethostname re_comp setlocale alarm dup2 gethostbyaddr gethostbyname gettimeofday inet_ntoa isascii memmove memset pow putenv select socket sqrt strcasecmp strchr strerror strpbrk strrchr strstr tzset strtol getcwd memchr mkdir rmdir strcspn strdup strspn])

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

### SMAUG BASIC FEATURES ###

# Planes Support (-DPLANES) #
AC_ARG_ENABLE(planes,
              AC_HELP_STRING([--enable-planes],
                             [enable Planes support @<:@default=yes@:>@]),
              [], [enable_planes=yes])
if test "$enable_planes" = "yes"; then
		AC_DEFINE([PLANES], [], [Planes Support])
fi

# Require Who Command Argument (-DREQWHOARG) #
AC_ARG_ENABLE(whoarg,
              AC_HELP_STRING([--enable-whoarg],
                             [require Who command arguments @<:@default=no@:>@]),
              [], [enable_whoarg=no])
if test "$enable_whoarg" = "yes"; then
		AC_DEFINE([REQWHOARG], [], [Who Arguments Support])
fi

# Honour Code Support (-DHONOUR_CODE) #
AC_ARG_ENABLE(honour,
              AC_HELP_STRING([--enable-honour],
                             [enable Honour Code support @<:@default=yes@:>@]),
              [], [enable_honour=yes])
if test "$enable_honour" = "yes"; then
		AC_DEFINE([HONOUR_CODE], [], [Honour Code Support])
fi

# Housing Support (-DHOUSING_CODE) #
AC_ARG_ENABLE(housing,
              AC_HELP_STRING([--enable-housing],
                             [enable Housing support @<:@default=yes@:>@]),
              [], [enable_housing=yes])
if test "$enable_housing" = "yes"; then
		AC_DEFINE([HOUSING_CODE], [], [Housing Support])
fi

# House mobs Support (-DHOUSE_MOBS) #
AC_ARG_ENABLE(housemobs,
              AC_HELP_STRING([--enable-housemobs],
                             [enable House Mobs support @<:@default=yes@:>@]),
              [], [enable_housemobs=yes])
if test "$enable_housemobs" = "yes"; then
		AC_DEFINE([HOUSE_MOBS], [], [House Mobs Support])
fi

# Enable Extended Rooms (-DEXTENDED_ROOMS) #
AC_ARG_ENABLE(extrooms,
              AC_HELP_STRING([--enable-extrooms],
                             [enable Extended Rooms Support @<:@default=yes@:>@]),
              [], [enable_extrooms=yes])
if test "$enable_extrooms" = "yes"; then
		AC_DEFINE([EXTENDED_ROOMS], [], [Extended Rooms Support])
fi

# act_comm.c SCRAMBLE Support (testing) (-DSCRAMBLE) #
AC_ARG_ENABLE(scramble,
              AC_HELP_STRING([--enable-scramble],
                             [enable Act Scramble Support (testing) @<:@default=no@:>@]),
              [], [enable_scramble=no])
if test "$enable_scramble" = "yes"; then
		AC_DEFINE([SCRAMBLE], [], [ActComm Scramble Support])
fi

# Liquids & Mixtures System Support (-DLIQUIDSYSTEM) #
# Replaces: "do_drink", "do_fill", "do_empty" functions from "misc.c".
AC_ARG_ENABLE(liquids,
              AC_HELP_STRING([--enable-liquids],
                             [enable Liquids & Mixtures System @<:@default=yes@:>@]),
              [], [enable_liquids=yes])
if test "$enable_liquids" = "yes"; then
		AC_DEFINE([LIQUIDSYSTEM], [], [Liquids & Mixtures System])
fi

# TIME Format Support (-DTIMEFORMAT) #
# Enable if you are getting a line like: interp.c:757: warning: int format, time_t arg (arg 7)
AC_ARG_ENABLE(time,
              AC_HELP_STRING([--enable-time],
                             [enable Time Format if you are getting warning int format @<:@default=no@:>@]),
              [], [enable_time=no])
if test "$enable_time" = "yes"; then
		AC_DEFINE([TIMEFORMAT], [], [Time Format warnings])
fi

# REGEX Support (-DREGEX) #
# Enable if you are getting implicit decleration of re_exec
AC_ARG_ENABLE(regex,
              AC_HELP_STRING([--enable-regex],
                             [enable if you are getting implicit decleration of re_exec @<:@default=yes@:>@]),
              [], [enable_regex=yes])
if test "$enable_regex" = "yes"; then
		# Manage undefined re_exec errors
		AC_DEFINE([REGEX], [], [Regex Support])
		# LDFLAGS="$LDFLAGS -lgnuregex"
fi

# Password Encription Support (-DNOCRYPT) #
AC_ARG_ENABLE(crypt,
              AC_HELP_STRING([--enable-crypt],
                             [enable Password Encryption support @<:@default=yes@:>@]),
              [], [enable_crypt=yes])
if test "$enable_crypt" = "yes"; then
		AC_DEFINE([CRYPT], [], [Password Encryption Support])
		# LDFLAGS="$LDFLAGS -lcrypt"
else
		AC_DEFINE([NOCRYPT], [], [Disable Password Encryption Support])
fi

# REQUEST Support (-DREQUESTS) #
AC_ARG_ENABLE(request,
              AC_HELP_STRING([--enable-request],
                             [enable Request Support @<:@default=no@:>@]),
              [], [enable_request=no])
if test "$enable_request" = "yes"; then
		AC_DEFINE([REQUESTS], [], [Requests Support])
fi

# Profiling Support #
AC_ARG_ENABLE(profiling,
              AC_HELP_STRING([--enable-profiling],
                             [enable profiling support @<:@default=no@:>@]),
              [], [enable_profiling=no])
if test "$enable_profiling" = "yes"; then
		CFLAGS="$CFLAGS -pg -g3"
else
		CFLAGS="$CFLAGS -fomit-frame-pointer"
fi

### SMAUG BUILD FLAGS ###

# Performance CFLAGS #
AC_ARG_ENABLE(performance,
              AC_HELP_STRING([--enable-performance],
                             [enable Performance Flags @<:@default=yes@:>@]),
              [], [enable_performance=yes])
if test "$enable_performance" = "yes"; then
		LDFLAGS="$LDFLAGS -finline-functions -funroll-loops -fdefer-pop -fstrength-reduce"
fi

# Developer CFLAGS #
AC_ARG_ENABLE(devel,
              AC_HELP_STRING([--enable-devel],
                             [enable Developer Build Flags @<:@default=no@:>@]),
              [], [enable_devel=no])
if test "$enable_devel" = "yes"; then
		CFLAGS="$CFLAGS -Wall -Wshadow -Wformat-security -Wpointer-arith -Wcast-align -Wredundant-decls -Wuninitialized"
else
		CFLAGS="$CFLAGS -Wno-all -Wno-cast-align -Wno-unused-result -Wno-write-strings -Wno-unused-but-set-variable -Wno-pointer-to-int-cast -Wno-address -Wno-int-to-pointer-cast -Wno-uninitialized"
fi

### SMAUG 2.0 EXTRA FEATURES ###

# SMAUG 2.0 Suport (-DSMAUG -DSMAUG2) #
AC_ARG_ENABLE(smaug2,
              AC_HELP_STRING([--enable-smaug2],
                             [enable SMAUG 2.0 support @<:@default=yes@:>@]),
              [], [enable_smaug2=yes])
if test "$enable_smaug2" = "yes"; then
		AC_DEFINE([SMAUG2], [], [SMAUG Support])
fi

# New Score Table (-DNEWSCORE) #
AC_ARG_ENABLE(newscore,
              AC_HELP_STRING([--enable-newscore],
                             [enable New Score Tables @<:@default=yes@:>@]),
              [], [enable_newscore=yes])
if test "$enable_newscore" = "yes"; then
		AC_DEFINE([NEWSCORE], [], [New Score Tables])
fi

# Player Show Stance (-DPLRSHOWSTANCE) #
AC_ARG_ENABLE(showstance,
              AC_HELP_STRING([--enable-showstance],
                             [enable Show Stances in Player Score @<:@default=yes@:>@]),
              [], [enable_showstance=yes])
if test "$enable_showstance" = "yes"; then
		AC_DEFINE([PLRSHOWSTANCE], [], [Show Stances Score])
fi

# Enable Banking Support (-DBANK_INSTALLED) #
AC_ARG_ENABLE(bank,
              AC_HELP_STRING([--enable-bank],
                             [enable Banking Support @<:@default=yes@:>@]),
              [], [enable_bank=yes])
if test "$enable_bank" = "yes"; then
		AC_DEFINE([BANK_INSTALLED], [], [Banking Support])
fi

# Enable Marriage Support (-DMARRIAGE) #
AC_ARG_ENABLE(marry,
              AC_HELP_STRING([--enable-marry],
                             [enable Marriage Support @<:@default=yes@:>@]),
              [], [enable_marry=yes])
if test "$enable_marry" = "yes"; then
		AC_DEFINE([MARRIAGE], [], [Marriage Support])
fi

# Player Bleeding (-DBLEEDING) #
AC_ARG_ENABLE(bleeding,
              AC_HELP_STRING([--enable-bleeding],
                             [enable Player Bleeding @<:@default=yes@:>@]),
              [], [enable_bleeding=yes])
if test "$enable_bleeding" = "yes"; then
		AC_DEFINE([BLEEDING], [], [Player Bleeding])
fi

# Enable Hotboot Support (-DENABLE_HOTBOOT) #
AC_ARG_ENABLE(hotboot,
              AC_HELP_STRING([--enable-hotboot],
                             [enable Hotboot Support @<:@default=yes@:>@]),
              [], [enable_hotboot=yes])
if test "$enable_hotboot" = "yes"; then
		AC_DEFINE([ENABLE_HOTBOOT], [], [Hotboot Support])
fi
AM_CONDITIONAL([ENABLE_HOTBOOT], [test "$enable_hotboot" = "yes"])

# Enable Web Server (-DWEBSRV)
AC_ARG_ENABLE(websrv,
              AC_HELP_STRING([--enable-websrv],
                             [enable Integrated Web Server (Listen on port [:8080]) @<:@default=yes@:>@]),
              [], [enable_websrv=yes])
if test "$enable_websrv" = "yes"; then
		AC_DEFINE([WEBSRV], [], [Integrated Web Server])
		AC_DEFINE([WEB_PORT], [8080], [Integrated Web Server Port])
fi
AM_CONDITIONAL([WEBSRV], [test "$enable_websrv" = "yes"])

# Enable DNS Resolver Support (-DENABLE_DNS_RESOLV) #
AC_ARG_ENABLE(dns,
              AC_HELP_STRING([--enable-dns],
                             [enable DNS Resolver @<:@default=yes@:>@]),
              [], [enable_dns=yes])
if test "$enable_dns" = "yes"; then
		AC_DEFINE([ENABLE_DNS_RESOLV], [], [DNS Resolver])
fi
AM_CONDITIONAL([ENABLE_DNS_RESOLV], [test "$enable_dns" = "yes"])

# Create build files #

AC_CONFIG_FILES([Makefile
		 po/Makefile.in
		 src/Makefile
		 bin/Makefile
		 etc/Makefile
		 etc/init.d/Makefile
		 Doxyfile])

LT_OUTPUT
AC_OUTPUT

# Show configure build flags & options #

AC_MSG_RESULT
AC_MSG_RESULT([Configured --{SMAUG}-- build flags:

 * Time Format fix:     <$enable_time>
 * RE_EXEC fix:         <$enable_regex>
 * Profiling flags:     <$enable_profiling>
 * Developer flags:     <$enable_devel>
 * Performance flags:   <$enable_performance>

Configured --{SMAUG}-- MUD Server basic features:

 * Password Encryption: <$enable_crypt>
 * Request Support:     <$enable_request>
 * Planes:              <$enable_planes>
 * Who arguments:       <$enable_whoarg>
 * Honour Code:         <$enable_honour>
 * Housing:             <$enable_housing>
 * House Mobs:          <$enable_housemobs>
 * Extended Rooms:      <$enable_extrooms>
 * Act Comm Scramble:   <$enable_scramble>
 * Liquids System:      <$enable_liquids>

Configured --{SMAUG}-- MUD Server extra features:

 * SMAUG 2.0:           <$enable_smaug2>
 * New Score Tables:    <$enable_newscore>
 * Show Stance Info:    <$enable_showstance>
 * Banking:             <$enable_bank>
 * Marry:               <$enable_marry>
 * Player Bleeding:     <$enable_bleeding>
 * DNS Resolver:        <$enable_dns>
 * Web Server:          <$enable_websrv>
 * HotBoot:             <$enable_hotboot>
])
AC_MSG_RESULT([Well done! \"configure\" complete, now type \"make\"!])
AC_MSG_RESULT

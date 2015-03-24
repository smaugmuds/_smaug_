 ############################################################################
 #                                                                          #
 #   X      X  ******* **    **  ******  **    **  ******                   #
 #    X    X  ******** ***  *** ******** **    ** ********       \\._.//    #
 #     X  X   **       ******** **    ** **    ** **             (0...0)    #
 #      XX    *******  ******** ******** **    ** **  ****        ).:.(     #
 #      XX     ******* ** ** ** ******** **    ** **  ****        {o o}     #
 #     X  X         ** **    ** **    ** **    ** **    **       / ' ' \    #
 #    X    X  ******** **    ** **    ** ******** ********    -^^.VxvxV.^^- #
 #   X      X *******  **    ** **    **  ******   ******                   #
 #                                                                          #
 # ------------------------------------------------------------------------ #
 # Ne[X]t Generation [S]imulated [M]edieval [A]dventure Multi[U]ser [G]ame  #
 # ------------------------------------------------------------------------ #
 # SMAUG © 2014-2015 Antonio Cao (@burzumishi)                             #
 # ------------------------------------------------------------------------ #
 #                                  SMAUG                                  #
 #                               configure.ac                               #
 ############################################################################
 #                                                                          #
 #                             -*- Autoconf -*-                             #
 #      Process this file with autoconf to produce a configure script.      #
 #                                                                          #
 ############################################################################

AC_PREREQ([2.69])
LT_PREREQ([2.4])  # require libtool

AC_SUBST([PACKAGE], ["smaug"])

AC_SUBST([SMAUG_MAJOR_VERSION], [1])
AC_SUBST([SMAUG_MINOR_VERSION], [8])
AC_SUBST([SMAUG_UPDATE_VERSION], [2])

AC_SUBST([VERSION], ["${SMAUG_MAJOR_VERSION}.${SMAUG_MINOR_VERSION}.${SMAUG_UPDATE_VERSION}"])

AC_INIT([smaug], [1.8.2],
	[<@burzumishi>],
	[smaug])

AC_COPYRIGHT(["2014-2015 (c) Antonio Cao (@burzumishi)"])

LT_INIT
LT_LANG([C])

# AM_INIT_AUTOMAKE([-Wno-all -Wno-address -Wno-pointer-to-int-cast -Wno-unused-but-set-variable -Wno-cast-align -Wno-unused-result -Wno-write-strings])
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

AC_SUBST([DESCRIPTION], ["SMAUG MUD Server"])

AC_LANG([C])
AC_LANG_PUSH([C])

AC_PROG_CC([cc])
AC_PROG_CXX([g++])
AC_PROG_LIBTOOL
# AC_PROG_RANLIB
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
AM_CONDITIONAL([HAVE_Z], [test -n "${z}"])

AC_CHECK_LIB([ssl], [main])
AM_CONDITIONAL([HAVE_SSL], [test -n "${ssl}"])

AC_CHECK_LIB([pthread], [main])
AM_CONDITIONAL([HAVE_PTHREAD], [test -n "${pthread}"])

# Checks for header files.
AC_CHECK_HEADERS([arpa/inet.h fcntl.h malloc.h math.h etdb.h netinet/in.h sys/param.h sys/socket.h sys/time.h unistd.h pthread.h openssl/ssl.h limits.h stddef.h])

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
    CFLAGS="$CFLAGS -pg -g3"
else
    DFB_CFLAGS_OMIT_FRAME_POINTER="-fomit-frame-pointer"
fi

AC_CONFIG_FILES([Makefile
		 src/Makefile
		 bin/Makefile
		 etc/Makefile
		 etc/init.d/Makefile])

LT_OUTPUT
AC_OUTPUT


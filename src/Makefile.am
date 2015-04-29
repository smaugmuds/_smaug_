#!@ENV@ make -f

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
#     |                         -*- Makefile -*-                            |
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

CC = gcc
CPP = cpp
CXX = g++

ARCH = `uname -m`

localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

# Uncomment the two lines below if compiling on a Solaris box

# SOLARIS_FLAG = -Dsun -DSYSV
# SOLARIS_LINK = -lnsl -lsocket

CLEANFILES = *~ *.o *.lo *.la *.a *.so smaug.h \
smaug$(EXEEXT) smaug-resolv$(EXEEXT) smaug-httpd$(EXEEXT) stamp-h1

# Build binaries #

# bin/smaug #
bin_PROGRAMS = smaug$(EXEEXT)

# bin/smaug-resolv #
if ENABLE_DNS_RESOLV
bin_PROGRAMS += smaug-resolv$(EXEEXT)
endif

# bin/smaug-httpd #
if HTTPD
bin_PROGRAMS += smaug-httpd$(EXEEXT)
endif

# Build Shared libraries #

# lib/libsmaug-*.la (static libs) #
noinst_LTLIBRARIES = \
libsmaug-act.la libsmaug-player.la libsmaug-mudprog.la \
libsmaug-info.la libsmaug-realm.la

# lib/libsmaug-httpd.la (static lib) #
if HTTPD
noinst_LTLIBRARIES += libsmaug-httpd.la
endif

# lib/libsmaug-dns.la (static lib) #
if ENABLE_DNS_RESOLV
noinst_LTLIBRARIES += libsmaug-dns.la
endif

# lib/libsmaug-act.so (shared lib) #
libsmaug_act_la_CXXFLAGS = -Wl,-rpath $(SOLARIS_FLAG)
libsmaug_act_la_CFLAGS = -Wl,-rpath $(SOLARIS_FLAG)
libsmaug_act_la_LDFLAGS = -Wl,-rpath $(SOLARIS_LINK) $(LTLIBINTL)

libsmaug_act_la_SOURCES = \
act_comm.c act_info.c act_move.c act_obj.c act_wiz.c fight.c misc.c 

# lib/libsmaug-player.so (shared lib) #
libsmaug_player_la_CXXFLAGS = -Wl,-rpath $(SOLARIS_FLAG)
libsmaug_player_la_CFLAGS = -Wl,-rpath $(SOLARIS_FLAG)
libsmaug_player_la_LDFLAGS = -Wl,-rpath $(SOLARIS_LINK) $(LTLIBINTL)

libsmaug_player_la_SOURCES = \
player.c polymorph.c update.c save.c ban.c 

# lib/libsmaug-mudprog.so (shared lib) #
libsmaug_mudprog_la_CXXFLAGS = -Wl,-rpath $(SOLARIS_FLAG)
libsmaug_mudprog_la_CFLAGS = -Wl,-rpath $(SOLARIS_FLAG)
libsmaug_mudprog_la_LDFLAGS = -Wl,-rpath $(SOLARIS_LINK) $(LTLIBINTL)

libsmaug_mudprog_la_SOURCES = \
mud_comm.c mud_prog.c

# lib/libsmaug-info.so (shared lib) #
libsmaug_info_la_CXXFLAGS = -Wl,-rpath $(SOLARIS_FLAG)
libsmaug_info_la_CFLAGS = -Wl,-rpath $(SOLARIS_FLAG)
libsmaug_info_la_LDFLAGS = -Wl,-rpath $(SOLARIS_LINK) $(LTLIBINTL)

libsmaug_info_la_SOURCES = \
boards.c comments.c hint.c news.c requests.c 

# lib/libsmaug-realm.so (shared lib) #
libsmaug_realm_la_CXXFLAGS = -Wl,-rpath $(SOLARIS_FLAG)
libsmaug_realm_la_CFLAGS = -Wl,-rpath $(SOLARIS_FLAG)
libsmaug_realm_la_LDFLAGS = -Wl,-rpath $(SOLARIS_LINK) $(LTLIBINTL)

libsmaug_realm_la_SOURCES = \
adminlist.c bank.c build.c clans.c deity.c grub.c house.c \
ibuild.c liquids.c magic.c makeobjs.c mapout.c mpxset.c planes.c \
renumber.c reset.c shops.c skills.c special.c stances.c starmap.c \
stat_obj.c tables.c track.c variables.c marry.c

if ENABLE_MAPPER
libsmaug_realm_la_SOURCES += mapper.c
endif

if ENABLE_CALENDAR
libsmaug_realm_la_SOURCES += calendar.c
endif

if ENABLE_WEATHER
libsmaug_realm_la_SOURCES += weather.c
endif

# bin/smaug #

smaug_CXXFLAGS = -Wl,-rpath $(SOLARIS_FLAG)
smaug_CFLAGS = -Wl,-rpath $(SOLARIS_FLAG)
smaug_LDFLAGS = -Wl,-rpath $(SOLARIS_LINK) $(LTLIBINTL)

smaug_SOURCES = smaug.c \
i18n.c const.c colorize.c db.c handler.c hashstr.c \
ident.c imm_host.c interp.c win32srv.c sha256.c

if ENABLE_HOTBOOT
smaug_SOURCES += hotboot.c
endif

smaug_LDADD = \
libsmaug-act.la libsmaug-player.la libsmaug-mudprog.la \
libsmaug-info.la libsmaug-realm.la

if HTTPD
# libsmaug-httpd #

libsmaug_httpd_la_CXXFLAGS = -Wl,-rpath $(SOLARIS_FLAG)
libsmaug_httpd_la_CFLAGS = -Wl,-rpath $(SOLARIS_FLAG)
libsmaug_httpd_la_LDFLAGS = -Wl,-rpath $(SOLARIS_LINK) $(LTLIBINTL)

libsmaug_httpd_la_SOURCES = mongoose.c

# bin/smaug-httpd #

smaug_httpd_CXXFLAGS = -g -O0 -Wl,-rpath $(SOLARIS_FLAG) -I../.. -pthread -DMONGOOSE_ENABLE_THREADS -DNS_ENABLE_SSL -lssl
smaug_httpd_CFLAGS = -g -O0 -Wl,-rpath $(SOLARIS_FLAG) -I../.. -pthread -DMONGOOSE_ENABLE_THREADS -DNS_ENABLE_SSL -lssl
smaug_httpd_LDFLAGS = -Wl,-rpath $(SOLARIS_LINK) $(LTLIBINTL)

smaug_httpd_SOURCES = httpd.c

smaug_httpd_LDADD = libsmaug-httpd.la
endif

if ENABLE_DNS_RESOLV
# libsmaug-dns #

libsmaug_dns_la_CXXFLAGS = -Wl,-rpath $(SOLARIS_FLAG)
libsmaug_dns_la_CFLAGS = -Wl,-rpath $(SOLARIS_FLAG)
libsmaug_dns_la_LDFLAGS = -Wl,-rpath $(SOLARIS_LINK) $(LTLIBINTL)

libsmaug_dns_la_SOURCES = dns.c

# bin/smaug-resolv #

smaug_resolv_CXXFLAGS = -Wl,-rpath $(SOLARIS_FLAG)
smaug_resolv_CFLAGS = -Wl,-rpath $(SOLARIS_FLAG)
smaug_resolv_LDFLAGS = -Wl,-rpath $(SOLARIS_LINK) $(LTLIBINTL)

smaug_resolv_SOURCES = resolv.c

smaug_resolv_LDADD = libsmaug-dns.la
endif

# Source Code Headers #

include_HEADERS = \
smaug.h mud.h bank.h bet.h hint.h i18n.h house.h liquids.h \
news.h sha256.h gettext.h 

if HTTPD
include_HEADERS += mongoose.h
endif

if ENABLE_HOTBOOT
include_HEADERS += hotboot.h
endif

if ENABLE_MAPPER
include_HEADERS += mapper.h
endif

if ENABLE_CALENDAR
include_HEADERS += calendar.h
endif

if ENABLE_WEATHER
include_HEADERS += weather.h
endif

if ENABLE_DNS_RESOLV
include_HEADERS += dns.h
endif


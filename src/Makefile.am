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

##### --{smaug}-- SERVER OPTIONS #####

# USE_SMAUG = -DSMAUG -DSMAUG2 ### NOT USED MAYBE IN SOME EXTRA SNIPPETS ###

# Comment the line if you don't want planes */
USE_PLANES = -DPLANES

# Comment the line if you don't want "who" command arguments */
# USE_WHOARG = -DREQWHOARG

# Comment the line if you don't want support for Honour Code */
USE_HONOUR_CODE = -DHONOUR_CODE

# Comment the line if you don't want support for housing */
USE_HOUSING_CODE = -DHOUSING_CODE

# Comment the line if you don't want support for house mobs */
USE_HOUSE_MOBS = -DHOUSE_MOBS

# Uncomment the line below if you've installed a banking snippet */
USE_BANK = -DBANK_INSTALLED

# Uncomment the line below if you've changed your room flags into extended bitvectors*/
USE_EXT_ROOMS = -DEXTENDED_ROOMS

##### --{smaug}-- BUILD OPTIONS #####

# Uncomment to enable profiling

PROFILING = ${PROFILE_CFLAGS}

# Uncomment the two lines below if compiling on a Solaris box

# SOLARIS_FLAG = -Dsun -DSYSV
# SOLARIS_LINK = -lnsl -lsocket

# Uncomment the line below if you are getting a line like:
# interp.c:757: warning: int format, time_t arg (arg 7)

# TIME = -DTIMEFORMAT

# Uncomment the line below if you are getting implicit decleration of re_exec

REG = -DREGEX

# Uncomment the line below if you are getting undefined re_exec errors

NEED_REG = -lgnuregex

# Uncomment the line below if you are getting undefined crypt errors
# Replaced by sha256_crypt! 

NEED_CRYPT = -lcrypt 

# Uncomment to disable password encryption

#NO_CRYPT = -DNOCRYPT

DEBUG_FLAGS = -DREQUESTS

### DEVELOPER CFLAGS (All Warnings Enabled) ###

# Uncomment the line below if you want all warnings while building.

#AM_CFLAGS = \
#-Wall -Wshadow -Wformat-security -Wpointer-arith -Wcast-align \
#-Wredundant-decls -Wuninitialized $(DEBUG_FLAGS)

# (and comment the following AM_CFLAGS)

### PACKAGER CFLAGS (No Warnings Enabled) ###

# Uncomment the line below if you dont want warnings while building.
# (and comment previous AM_CFLAGS)

AM_CFLAGS = \
-Wno-all -Wno-cast-align -Wno-unused-result -Wno-write-strings \
-Wno-unused-but-set-variable -Wno-pointer-to-int-cast \
-Wno-address -Wno-int-to-pointer-cast -Wno-uninitialized

# Uncomment the line below if you want a performance increase though beware
# your core files may not be as much of a benefit if you do.

AM_PFLAGS = -finline-functions -funroll-loops -fdefer-pop -fstrength-reduce

AM_CFLAGS += \
$(PROFILING) $(USE_SMAUG) $(USE_PLANES) $(USE_WHOARG) $(USE_HONOUR_CODE) \
$(USE_HOUSING_CODE) $(USE_HOUSE_MOBS) $(USE_BANK) $(USE_EXT_ROOMS) \
$(NO_CRYPT) $(TIME) $(REG) $(SOLARIS_FLAG)

AM_LDFLAGS = $(AM_PFLAGS) \
$(PROFILING) $(SOLARIS_LINK) $(NEED_CRYPT)

smaug_CXXFLAGS = $(AM_CFLAGS)
smaug_CFLAGS = $(AM_CFLAGS)

smaug_LDFLAGS = $(AM_LDFLAGS)

### --{smaug}-- source files ###

bin_PROGRAMS = smaug$(EXEEXT)

CLEANFILES = *~ *.o smaug.h smaug$(EXEEXT) stamp-h1

smaug_SOURCES = \
act_comm.c act_info.c act_move.c act_obj.c act_wiz.c adminlist.c update.c \
ban.c bank.c boards.c build.c clans.c colorize.c comm.c comments.c const.c \
db.c deity.c fight.c grub.c handler.c hashstr.c hint.c house.c ibuild.c \
ident.c imm_host.c interp.c liquids.c magic.c makeobjs.c mapout.c misc.c \
mpxset.c mud_comm.c mud_prog.c news.c planes.c player.c polymorph.c \
renumber.c requests.c reset.c save.c services.c sha256.c shops.c skills.c \
special.c stances.c starmap.c stat_obj.c tables.c track.c variables.c

include_HEADERS = \
smaug.h mud.h bank.h bet.h hint.h house.h liquids.h news.h sha256.h

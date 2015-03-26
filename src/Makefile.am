#!@ENV@ make -f

  #################################################################
  #                                              |                #
  #  ******* **    **  ******  **    **  ******  |                #
  # ******** ***  *** ******** **    ** ******** |    \\._.//     #
  # **       ******** **    ** **    ** **       |    (0...0)     #
  # *******  ******** ******** **    ** **  **** |     ).:.(      #
  #  ******* ** ** ** ******** **    ** **  **** |     {o o}      #
  #       ** **    ** **    ** **    ** **    ** |    / ' ' \     #
  # ******** **    ** **    ** ******** ******** | -^^.VxvxV.^^-  #
  # *******  **    ** **    **  ******   ******  |                #
  #                                              |                #
  # ------------------------------------------------------------- #
  # [S]imulated [M]edieval [A]dventure Multi[U]ser [G]ame         #
  # ------------------------------------------------------------- #
  # SMAUG 1.4 © 1994, 1995, 1996, 1998  by Derek Snider           #
  # ------------------------------------------------------------- #
  # SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,         #
  # Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,         #
  # Tricops, Fireblade, Edmond, Conran                            #
  # ------------------------------------------------------------- #
  # Merc 2.1 Diku Mud improvments copyright © 1992, 1993 by       #
  # Michael Chastain, Michael Quan, and Mitchell Tse.             #
  # Original Diku Mud copyright © 1990, 1991 by Sebastian Hammer, #
  # Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja #
  # Nyboe. Win32 port by Nick Gammon                              #
  # ------------------------------------------------------------- #
  # --{smaug}-- 1.8.x © 2014-2015 by Antonio Cao @(burzumishi)    #
  # ------------------------------------------------------------- #
  #                           Makefile                            #
  #################################################################

CC = gcc
CPP = cpp
CXX = g++

ARCH = `uname -m`

##### --{smaug}-- SERVER OPTIONS #####

USE_SMAUG = -DSMAUG -DSMAUG18

USE_PLANES = -DPLANES

USE_WHOARG = -DREQWHOARG

USE_HONOUR_CODE = -DHONOUR_CODE

##### --{smaug}-- BUILD OPTIONS #####

### DEVELOPER CFLAGS (All Warnings Enabled) ###

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

NOCRYPT = -DNOCRYPT

DEBUG_FLAGS = -DREQUESTS

# Uncomment the line below if you dont want warnings while building.

AM_CFLAGS = \
-Wall -Wshadow -Wformat-security -Wpointer-arith -Wcast-align \
-Wredundant-decls -Wuninitialized $(DEBUG_FLAGS)

# Uncomment the line below if you dont want warnings while building.
# (and comment upper AM_CFLAGS)

#AM_CFLAGS = \
#-Wno-all -Wno-cast-align -Wno-unused-result -Wno-write-strings \
#-Wno-unused-but-set-variable -Wno-pointer-to-int-cast \
#-Wno-address -Wno-int-to-pointer-cast -Wno-uninitialized

# Uncomment the line below if you want a performance increase though beware
# your core files may not be as much of a benefit if you do.

AM_PFLAGS = -finline-functions -funroll-loops -fdefer-pop -fstrength-reduce

AM_CFLAGS += \
-O -g3 $(PROFILING) $(USE_SMAUG) $(USE_PLANES) $(USE_WHOARG) $(USE_HONOUR_CODE) \
$(NOCRYPT) $(TIME) $(REG) $(SOLARIS_FLAG)

AM_LDFLAGS = $(AM_PFLAGS) $(PROFILING) $(SOLARIS_LINK) $(NEED_CRYPT)

smaug_CXXFLAGS = $(AM_CFLAGS)
smaug_CFLAGS = $(AM_CFLAGS)

smaug_LDFLAGS = $(AM_LDFLAGS)

### --{smaug}-- source files ###

bin_PROGRAMS = smaug$(EXEEXT)

CLEANFILES = *~ *.o smaug.h smaug$(EXEEXT) stamp-h1

smaug_SOURCES = \
act_comm.c act_info.c act_move.c act_obj.c act_wiz.c adminlist.c \
ban.c boards.c build.c clans.c colorize.c comm.c comments.c const.c \
db.c deity.c fight.c grub.c handler.c hashstr.c hint.c house.c ibuild.c \
ident.c imm_host.c interp.c liquids.c magic.c makeobjs.c mapout.c misc.c \
mpxset.c mud_comm.c mud_prog.c news.c planes.c player.c polymorph.c \
renumber.c requests.c reset.c save.c services.c sha256.c shops.c skills.c \
special.c stances.c starmap.c stat_obj.c tables.c track.c update.c variables.c 

include_HEADERS = \
mud.h smaug.h bet.h hint.h news.h sha256.h house.h liquids.h       

.c.o-local:
	@echo -n "  $(CC) -> Building Object: $@ ..."; \
	$(CC) -c $(AM_CFLAGS) $(AM_LDFLAGS) $< -o $@; \
	echo " done";

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

##### --{smaug}-- BUILD OPTIONS #####

USE_SMAUG = -DSMAUG -DSMAUG18

USE_PLANES = -DPLANES

### MUD Interconector (IMC) ###

USE_IMC = -DUSE_IMC

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

NOCRYPT =

DEBUG_FLAGS = -DREQUESTS

# Uncomment the line below if you dont want warnings while building.

#AM_CFLAGS = \
#-Wall -Wshadow -Wformat-security -Wpointer-arith -Wcast-align \
#-Wredundant-decls -Wuninitialized $(DEBUG_FLAGS)

# Uncomment the line below if you dont want warnings while building.
# (and comment upper AM_CFLAGS)

AM_CFLAGS = \
-Wno-all -Wno-cast-align -Wno-unused-result -Wno-write-strings \
-Wno-unused-but-set-variable -Wno-pointer-to-int-cast \
-Wno-address -Wno-int-to-pointer-cast -Wno-uninitialized

# Uncomment the line below if you want a performance increase though beware
# your core files may not be as much of a benefit if you do.

AM_PFLAGS = -finline-functions -funroll-loops -fdefer-pop -fstrength-reduce

AM_CFLAGS += \
-O -g3 -export-dinamic $(PROFILING) $(USE_SMAUG) $(USE_PLANES) \
$(NOCRYPT) $(TIME) $(REG) $(USE_IMC) $(SOLARIS_FLAG)

AM_LDFLAGS = $(AM_PFLAGS) $(PROFILING) $(SOLARIS_LINK) $(NEED_CRYPT)

smaug_CXXFLAGS = $(AM_CFLAGS)
smaug_CFLAGS = $(AM_CFLAGS)

smaug_LDFLAGS = $(AM_LDFLAGS)

### --{smaug}-- source files ###

bin_PROGRAMS = smaug$(EXEEXT)

CLEANFILES = *~ *.o smaug.h smaug$(EXEEXT) stamp-h1

smaug_SOURCES = \
act_comm.c   comm.c      ibuild.c         imc-mercbase.c  mud_comm.c   sha256.c   \
act_info.c   comments.c  ice.c            imc-util.c      mud_prog.c   shops.c    \
act_move.c   const.c     icec.c           imc-version.c   news.c       skills.c   \
act_obj.c    db.c        icec-mercbase.c  imm_host.c      planes.c     special.c  \
act_wiz.c    deity.c     iced.c           interp.c        player.c     stances.c  \
adminlist.c  fight.c     ident.c          liquids.c       polymorph.c  starmap.c  \
ban.c        grub.c      imc.c            magic.c         renumber.c   stat_obj.c \
boards.c     handler.c   imc-config.c     makeobjs.c      requests.c   tables.c   \
build.c      hashstr.c   imc-events.c     mapout.c        reset.c      track.c    \
clans.c      hint.c      imc-interp.c     misc.c          save.c       update.c   \
colorize.c   house.c     imc-mail.c       mpxset.c        services.c   variables.c

include_HEADERS =  mud.h smaug.h  \
bet.h    icec.h          ice.h    imc-mercbase.h  icec-mercbase.h \
hint.h   imc-config.h    news.h   imc-mercdefs.h  sha256.h \
house.h  iced.h          imc.h    liquids.h       

smaug_%.o: %.c
	@echo -n "  $(CC) -> Building Object: $@ ..."; \
	$(CC) -c $(AM_CFLAGS) $(AM_LDFLAGS) $< -o $@; \
	echo " done";


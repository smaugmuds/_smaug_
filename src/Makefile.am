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
 #                                 Makefile                                 #
 ############################################################################

CC = gcc
CPP = cpp
CXX = g++

ARCH = `uname -m`

#PROF = -p

#TIME = -DTIMEFORMAT

#Uncomment the line below if you are getting implicit decleration of re_exec
REG = -DREGEX

#Uncomment the line below if you are getting undefined re_exec errors
NEED_REG = -lgnuregex

#Uncomment the line below if you are getting undefined crypt errors
NEED_CRYPT = -lcrypt

USE_IMC = -DUSE_IMC

## DEVELOPER CFLAGS (All Warnings) ##

DEBUG_FLAGS = -DREQUESTS

AM_CFLAGS = -Wall -Wuninitialized $(DEBUG_FLAGS)

#AM_CFLAGS = \
#-Wno-all -Wno-cast-align -Wno-unused-result -Wno-write-strings \
#-Wno-unused-but-set-variable -Wno-pointer-to-int-cast \
#-Wno-address -Wno-int-to-pointer-cast -Wno-uninitialized

# Uncomment the line below if you want a performance increase though beware
# your core files may not be as much of a benefit if you do.
AM_PFLAGS = -finline-functions -funroll-loops -fdefer-pop -fstrength-reduce

AM_CFLAGS += \
-O -g3 $(PROF) $(NOCRYPT) -DSMAUG $(TIME) $(REG) $(USE_IMC)

AM_LDFLAGS = $(AM_PFLAGS) $(PROF) $(SOLARIS_LINK) $(NEED_CRYPT)

bin_PROGRAMS = smaug$(EXEEXT)

CLEANFILES = *~ *.o smaug.h smaug$(EXEEXT) stamp-h1

smaug_SOURCES = \
ice.c icec.c icec-mercbase.c iced.c imc.c imc-config.c imc-events.c \
imc-interp.c imc-mail.c imc-mercbase.c imc-util.c imc-version.c \
act_comm.c   build.c     deity.c    ident.c     mpxset.c     requests.c  starmap.c \
act_info.c   fight.c     imm_host.c mud_comm.c  reset.c      stat_obj.c sha256.c \
act_move.c   clans.c     grub.c     interp.c    mud_prog.c   save.c      tables.c \
act_obj.c    colorize.c  handler.c  news.c      services.c   track.c     liquids.c \
act_wiz.c    comm.c      hashstr.c  magic.c     planes.c     shops.c     update.c \
adminlist.c  comments.c  hint.c     makeobjs.c  player.c     skills.c    variables.c \
ban.c        const.c     house.c    mapout.c    polymorph.c  special.c \
boards.c     db.c        ibuild.c   misc.c      renumber.c   stances.c

include_HEADERS = smaug.h mud.h bet.h house.h liquids.h hint.h news.h sha256.h \
icec.h icec-mercbase.h iced.h ice.h imc-config.h imc.h imc-mercbase.h imc-mercdefs.h

smaug_CXXFLAGS = $(AM_CFLAGS)
smaug_CFLAGS = $(AM_CFLAGS)

smaug_LDFLAGS = $(AM_LDFLAGS)

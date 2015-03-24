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

#Uncomment the line below if you are getting implicit decleration of re_exec
REG = -DREGEX

#Uncomment the line below if you are getting undefined re_exec errors
NEED_REG = -lgnuregex

#Uncomment the line below if you are getting undefined crypt errors
NEED_CRYPT = -lcrypt

DBUGFLG = -DREQUESTS

USE_IMC = -DUSE_IMC

# Uncomment the line below if you want a performance increase though beware
# your core files may not be as much of a benefit if you do.
OPT_FLAG = -finline-functions -funroll-loops -fdefer-pop -fstrength-reduce

CFLAGS += \
$(OPT_FLAG) -O -g3 -Wall -Wuninitialized \
$(PROF) $(NOCRYPT) $(DBUGFLG) -DSMAUG \
$(SOLARIS_FLAG) $(TIME) $(REG) $(USE_IMC)

LDFLAGS += $(OPT_FLAG) $(PROF) $(SOLARIS_LINK) $(NEED_CRYPT)

bin_PROGRAMS = smaug$(EXEEXT)

CLEANFILES = *~ *.o smaug.h smaug$(EXEEXT) stamp-h1

smaug_SOURCES = \
act_comm.c   build.c     deity.c    ident.c     mpxset.c     requests.c  starmap.c \
act_info.c   chess.c     fight.c    imm_host.c  mud_comm.c   reset.c     stat_obj.c \
act_move.c   clans.c     grub.c     interp.c    mud_prog.c   save.c      tables.c \
act_obj.c    colorize.c  handler.c  liquids.c   news.c       services.c  track.c \
act_wiz.c    comm.c      hashstr.c  magic.c     planes.c     shops.c     update.c \
adminlist.c  comments.c  hint.c     makeobjs.c  player.c     skills.c    variables.c \
ban.c        const.c     house.c    mapout.c    polymorph.c  special.c \
boards.c     db.c        ibuild.c   misc.c      renumber.c   stances.c


include_HEADERS = \
bet.h house.h liquids.h smaug.h \
chess.h mud.h hint.h news.h

CFLAGS_COMMON = \
-Wno-all -Wno-cast-align -Wno-unused-result -Wno-write-strings \
-Wunused-but-set-variable

smaug_CXXFLAGS = $(CFLAGS) $(CFLAGS_COMMON)
smaug_CFLAGS = $(CFLAGS) $(CFLAGS_COMMON)


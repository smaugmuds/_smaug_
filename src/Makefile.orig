CC      = gcc
#PROF    = -p
NOCRYPT =

# Uncomment the two lines below if compiling on a Solaris box
#SOLARIS_FLAG = -Dsun -DSYSV
#SOLARIS_LINK = -lnsl -lsocket

#Uncomment the line below if you are getting a line like:
#interp.c:757: warning: int format, time_t arg (arg 7)
#TIME = -DTIMEFORMAT

#Uncomment the line below if you are getting implicit decleration of re_exec
REG = -DREGEX

#Uncomment the line below if you are getting undefined re_exec errors
NEED_REG = -lgnuregex

#Uncomment the line below if you are getting undefined crypt errors
NEED_CRYPT = -lcrypt

DBUGFLG = -DREQUESTS

#Uncomment the line below if you want a performance increase though beware
#your core files may not be as much of a benefit if you do.
#OPT_FLAG = -finline-functions -funroll-loops -fdefer-pop -fstrength-reduce

C_FLAGS = $(OPT_FLAG) -O -g3 -Wall -Wuninitialized $(PROF) $(NOCRYPT) $(DBUGFLG) -DSMAUG $(SOLARIS_FLAG) $(TIME) $(REG)
L_FLAGS = $(OPT_FLAG) $(PROF) $(SOLARIS_LINK) $(NEED_CRYPT)

#Uncomment the next three comments below if you want to use IMC
USE_IMC    = -DUSE_IMC

IMC_OFILES = imc.o imc-mail.o imc-interp.o imc-util.o imc-config.o \
        imc-events.o imc-version.o imc-mercbase.o ice.o icec.o icec-mercbase.o

IMC_CFILES = imc.c imc-mail.c imc-interp.c imc-util.c imc-config.c \
        imc-events.c imc-version.c imc-mercbase.c ice.c icec.c icec-mercbase.c

O_FILES = act_comm.o act_info.o act_move.o act_obj.o act_wiz.o boards.o \
          build.o clans.o comm.o comments.o const.o db.o deity.o fight.o \
          handler.o hashstr.o ibuild.o ident.o interp.o magic.o makeobjs.o \
          mapout.o misc.o mpxset.o mud_comm.o mud_prog.o player.o polymorph.o \
          requests.o reset.o save.o shops.o skills.o special.o tables.o \
          track.o update.o grub.o stat_obj.o ban.o services.o planes.o \
          imm_host.o $(IMC_OFILES) colorize.o stances.o adminlist.o \
	      hint.o starmap.o house.o variables.o news.o

C_FILES = act_comm.c act_info.c act_move.c act_obj.c act_wiz.c boards.c \
          build.c clans.c comm.c comments.c const.c db.c deity.c fight.c \
          handler.c hashstr.c ibuild.c ident.c interp.c magic.c makeobjs.c \
          mapout.c misc.c mpxset.c mud_comm.c mud_prog.c player.c polymorph.c \
          requests.c reset.c save.c shops.c skills.c special.c tables.c \
          track.c update.c grub.c stat_obj.c ban.c services.c planes.c \
          imm_host.c $(IMC_CFILES) colorize.c stances.c adminlist.c \
	       hint.c starmap.c house.c variables.c news.c


H_FILES = mud.h bet.h imc-config.h imc-mercbase.h imc-mercdefs.h imc.h \
          ice.h icec.h icec-mercbase.h hint.h house.h news.h

all:
	make smaug

smaug: $(O_FILES)
	rm -f smaug
	$(CC) $(L_FLAGS) $(USE_IMC) -o smaug $(O_FILES)
	chmod g+w smaug
	chmod a+x smaug
	chmod g+w $(O_FILES)

.c.o: mud.h
	$(CC) -c $(C_FLAGS) $(USE_IMC) $<

clean:
	rm -f *.o smaug *~

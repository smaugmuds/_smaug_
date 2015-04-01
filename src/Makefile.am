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

# Uncomment the two lines below if compiling on a Solaris box

# SOLARIS_FLAG = -Dsun -DSYSV
# SOLARIS_LINK = -lnsl -lsocket

smaug_CXXFLAGS = $(SOLARIS_FLAG)
smaug_CFLAGS = $(SOLARIS_FLAG)

smaug_LDFLAGS = $(SOLARIS_LINK)

bin_PROGRAMS = smaug$(EXEEXT)

CLEANFILES = *~ *.o smaug.h smaug$(EXEEXT) stamp-h1

### --{smaug}-- source files ###

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

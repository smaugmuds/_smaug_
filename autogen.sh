#!/bin/bash

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
  # ------------------------------------------------------------- #
  #                         -*- Autogen -*-                      #
  #################################################################

libtoolize
aclocal
autoconf
autoheader

ln -sf doc/LICENSE._smaug_ LICENSE
ln -sf doc/readme.txt INSTALL
ln -sf doc/license.txt COPYING
ln -sf README.md README
ln -sf doc/readme.txt AUTHORS
ln -sf doc/smaug.txt NEWS
ln -sf doc/readme.txt ChangeLog

automake -a

test -x configure || exit

./configure --prefix=/opt/smaug

exit


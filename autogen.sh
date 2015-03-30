#!/bin/env bash

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
#     |                         -*- Autogen -*-                             |
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

libtoolize
aclocal
autoconf
autoheader

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


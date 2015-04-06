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

echo
echo "-- --{SMAUG 2.0}-- -* Autogen *- by (@burzumishi) --"
echo

echo "Running libtoolize ..."
libtoolize

if test -f "acinclude.m4"; then rm -rf acinclude.m4; fi;

for file in libtool ltoptions ltsugar ltversion lt~obsolete; do
	echo "Appending m4/$file.m4 to acinclude.m4 ..."
	cat m4/$file.m4 >> acinclude.m4
done

# 
# if test ! -f "acinclude.m4" && test -f "/usr/share/libtool/libltdl/aclocal.m4"; then
# 	echo "Copying /usr/share/aclocal/aclocal.m4 -> m4/acinclude.m4 ..."
# 	cp -v /usr/share/libtool/libltdl/aclocal.m4 acinclude.m4
# fi;

for file in argz bison-i18n codeset expat fcntl-o gettext glib glib-gettext \
		iconv intldir intl intltool intmax inttypes_h inttypes-pri ltdl \
		lcmessage lib-ld lib-link lib-prefix libtool lock longlong ltoptions \
		ltsugar ltversion nls pkg po printf-posix progtest size_max stdint_h \
		threadlib uintmax_t visibility wchar_t wint_t ; do
			if test ! -f "m4/$file.m4" && test -f "/usr/share/aclocal/$file.m4"; then
				echo "Copying /usr/share/aclocal/$file.m4 -> m4/$file.m4 ..."
				cp -v /usr/share/aclocal/$file.m4 m4/$file.m4
			fi;
done;

echo "Backup configure.ac Makefile.am src/Makefile.am ...";
cp -v configure.ac configure.ac.autogen-bak
cp -v Makefile.am Makefile.am.autogen-bak
cp -v src/Makefile.am src/Makefile.am.autogen-bak

if test ! -f "po/Makefile.in.in" && test ! -f "po/smaug.pot"; then
	echo "Running gettextize --copy ..."
	gettextize --copy
fi

for file in m4/Makefile.am; do
	if test ! -f "$file"; then
		echo "Creating $file ..."
		touch $file;
	fi;
done;

echo "Restore configure.ac Makefile.am src/Makefile.am ...";
cp -v configure.ac.autogen-bak configure.ac
cp -v Makefile.am.autogen-bak Makefile.am
cp -v src/Makefile.am.autogen-bak src/Makefile.am

echo "Running aclocal ...";
aclocal -I m4

echo "Running autoheader ...";
autoheader

echo "Running autoconf ...";
autoconf

for doc in INSTALL README AUTHORS NEWS ChangeLog; do
	if test ! -f "$doc"; then
		echo "Linking README.md -> $doc ..."
		ln -svf README.md $doc
	fi
done

if test ! -f "COPYING"; then
	echo "Linking LICENSE -> COPYING ..."
	ln -svf LICENSE COPYING
fi;

echo "Running automake ...";
automake --add-missing
automake m4/Makefile

if test ! -f "src/gettext.h"; then
	echo "Copying gettext.h -> src/ ..."
	cp -v /usr/share/gettext/gettext.h src/
fi;

test -x configure || exit

echo "Launching: ./configure --prefix=/opt/smaug --disable-static --enable-shared --enable-nls ...";
./configure --prefix=/opt/smaug --disable-static --enable-shared --enable-nls

exit


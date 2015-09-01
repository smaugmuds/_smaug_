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
#   [|  SMAUG 2.0 © 2014-2015 Antonio Cao (@burzumishi)                      |]
#   [|                                                                       |]
#   [|  AFKMud Copyright 1997-2007 by Roger Libiez (Samson),                 |]
#   [|  Levi Beckerson (Whir), Michael Ward (Tarl), Erik Wolfe (Dwip),       |]
#   [|  Cameron Carroll (Cam), Cyberfox, Karangi, Rathian, Raine,            |]
#   [|  Xorith, and Adjani.                                                  |]
#   [|  All Rights Reserved. External contributions from Remcon, Quixadhal,  |]
#   [|  Zarius and many others.                                              |]
#   [|                                                                       |]
#   [|  SMAUG 1.4 © 1994-1998 Thoric/Altrag/Blodkai/Narn/Haus/Scryn/Rennard  |]
#   [|  Swordbearer/Gorog/Grishnakh/Nivek/Tricops/Fireblade/Edmond/Conran    |]
#   [|                                                                       |]
#   [|  Merc 2.1 Diku Mud improvments © 1992-1993 Michael Chastain, Michael  |]
#   [|  Quan, and Mitchell Tse. Original Diku Mud © 1990-1991 by Sebastian   |]
#   [|  Hammer, Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, Katja    |]
#   [|  Nyboe. Win32 port Nick Gammon.                                       |]
#    \\_____________________________________________________________________//

CC = gcc
CPP = cpp
CXX = g++

USER = smaug
GROUP = smaug

ARCH = `getconf LONG_BIT`

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = etc bin src po

EXTRA_DIST = m4/ChangeLog  config.rpath m4/ChangeLog autogen.sh tools \
	doc etc src man db README.md LICENSE m4 libtool bin autom4te.cache \
	missing compile config.guess config.sub depcomp aclocal.m4 \
	Doxyfile.in po ABOUT-NLS *.m4 TODO www

do_subst = sed -e 's,[@]sharedstatedir[@],$(sharedstatedir),g' \
		-e 's,[@]localstatedir[@],$(localstatedir),g' \
		-e 's,[@]sysconfdir[@],$(sysconfdir),g' \
		-e 's,[@]includedir[@],$(includedir),g' \
		-e 's,[@]datarootdir[@],$(datarootdir),g' \
		-e 's,[@]localedir[@],$(localedir),g' \
		-e 's,[@]prefix[@],$(prefix),g' \
		-e 's,[@]bindir[@],$(bindir),g' \
		-e 's,[@]sbindir[@],$(sbindir),g' \
		-e 's,[@]libdir[@],$(libdir),g' \
		-e 's,[@]mandir[@],$(mandir),g' \
		-e 's,[@]PERL[@],$(PERL),g' \
		-e 's,[@]ENV[@],$(ENV),g' \
		-e 's,[@]BASH[@],$(BASH),g' \
		-e 's,[@]USER[@],$(USER),g' \
		-e 's,[@]GROUP[@],$(GROUP),g' \
		-e 's,[@]CC[@],$(CC),g' \
		-e 's,[@]CXX[@],$(CXX),g' \
		-e 's,[@]PACKAGE[@],$(PACKAGE),g' \
		-e 's,[@]VERSION[@],$(VERSION),g' \
		-e 's,[@]DESCRIPTION[@],$(DESCRIPTION),g'

docdir = $(datarootdir)/doc/@PACKAGE@
doc_DATA = README AUTHORS INSTALL NEWS LICENSE ChangeLog \
	doc/*.txt ABOUT-NLS

localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

man_MANS = man/smaug.1 man/smaugd.1

CLEANDEVELFILES = compile configure depcomp src/smaug.h src/*.la ar-lib \
	autom4te.cache config.guess config.sub install-sh missing src/.libs \
	config.status ltmain.sh depcomp README COPYING NEWS po/*.gmo m4 *.m4 \
	src/smaug src/smaug-resolv src/smaug-httpd Doxyfile po/POTFILES po/stamp-po \
	AUTHORS ChangeLog INSTALL config.lt libtool src/.deps src/*.o src/*.lo \
	`find $(srcdir) -iname "Makefile.in"` \
	`find $(srcdir) -iname "*.autogen-bak"` \
	`find $(srcdir) -iname "*~"`

CLEANBUILDFILES = config.log etc/smaugd.conf etc/init.d/smaugd bin/smaug-monitor \
	`find $(srcdir) -iname "stamp-h1"` \
	`find $(srcdir) -iname "stamp-po"` \
	`find $(srcdir) -iname "Makefile"`

CLEANFILES = $(CLEANBUILDFILES) \
	`find $(srcdir) -iname "*~"`

if HAVE_DOXYGEN
all-doc:
	@$(DOXYGEN) Doxyfile;

clean-doc:
	@rm -rf $(top_builddir)/doc/latex $(top_builddir)/doc/html;

distclean-doc:
	@rm -rf $(top_builddir)/doc/latex $(top_builddir)/doc/html;

install-doc: all-doc
	@$(mkdir_p) $(DESTDIR)$(docdir); \
	cp -rvf doc/html $(DESTDIR)$(docdir);

uninstall-doc:
	@rm -rvf $(DESTDIR)$(docdir)/html; \
	rmdir $(DESTDIR)$(docdir);
else
all-doc:
	@echo "*** WARNING! Doxygen is not available ***"; \
	echo "*** Documentation will not be built ***";

clean-doc:
	@echo "*** WARNING! Doxygen is not available ***"; \
	echo "*** Documentation will not be cleaned ***";

distclean-doc:
	@echo "*** WARNING! Doxygen is not available ***"; \
	echo "*** Documentation will not be cleaned ***";

install-doc:
	@echo "*** WARNING! Doxygen is not available ***"; \
	echo "*** Documentation will not be installed ***";

uninstall-doc:
	@echo "*** WARNING! Doxygen is not available ***"; \
	echo "*** Documentation will not be uninstalled ***";
endif

all-local:
	@echo "=== all-local done ==="

clean-local:
	@echo "=== clean-local done ==="

distclean-local: clean-local
	@echo "=== distclean-local done ==="

devdistclean: distclean-local distclean-doc
	@rm -rfv $(CLEANDEVELFILES) $(CLEANBUILDFILES); \
	echo "=== distclean-local done ==="

install-data-hook:
	@echo "Creating <@PACKAGE@> directory to store log files ..."; \
	mkdir -pv $(DESTDIR)$(localstatedir)/log/@PACKAGE@; \
	echo "Creating <@PACKAGE@> directories to store lock and run files ..."; \
	mkdir -pv $(DESTDIR)$(localstatedir)/lock/@PACKAGE@; \
	mkdir -pv $(DESTDIR)$(localstatedir)/run/; \
	echo "Installing <@PACKAGE@> Database files ..."; \
	mkdir -pv $(DESTDIR)$(sharedstatedir)/@PACKAGE@; \
	cp -rvf db/* $(DESTDIR)$(sharedstatedir)/@PACKAGE@; \
	echo "Installing <@PACKAGE@> Website files ..."; \
	mkdir -pv $(DESTDIR)$(localstatedir)/www/@PACKAGE@; \
	cp -rvf www/* $(DESTDIR)$(localstatedir)/www/@PACKAGE@; \
	echo "Creating user <$(USER)> and group <$(GROUP)> ..."; \
	groupadd $(GROUP) &>/dev/null; \
	useradd $(USER) -g $(GROUP) &>/dev/null; \
	echo "Setting up <$(prefix)> permissions to <$(USER):$(GROUP)> ..."; \
	chown -R $(USER):$(GROUP) $(DESTDIR)$(localstatedir)/lock/@PACKAGE@; \
	chown -R $(USER):$(GROUP) $(DESTDIR)$(localstatedir)/log/@PACKAGE@; \
	chown -R $(USER):$(GROUP) $(DESTDIR)$(localstatedir)/run/@PACKAGE@; \
	chown -R $(USER):$(GROUP) $(DESTDIR)$(localstatedir)/www/@PACKAGE@; \
	chown -R $(USER):$(GROUP) $(DESTDIR)$(sharedstatedir)/@PACKAGE@; \
	echo ""; \
	echo "<@PACKAGE@> has ben installed to <$(prefix)> directory!"; \
	echo "You can execute <$(sysconfdir)/init.d/smaugd start> now!"; \
	echo "";

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

USER = smaug
GROUP = smaug

ARCH = `getconf LONG_BIT`

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src etc bin

EXTRA_DIST = autogen.sh data tools doc etc src man db \
	README.md LICENSE m4 libtool bin autom4te.cache \
	missing compile config.guess config.sub depcomp aclocal.m4

do_subst = sed -e 's,[@]sharedstatedir[@],$(sharedstatedir),g' \
		-e 's,[@]localstatedir[@],$(localstatedir),g' \
		-e 's,[@]sysconfdir[@],$(sysconfdir),g' \
		-e 's,[@]includedir[@],$(includedir),g' \
		-e 's,[@]datarootdir[@],$(datarootdir),g' \
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
	doc/*.md doc/*.txt doc/*.help 

man_MANS = man/smaug.1 man/smaugd.1

CLEANDEVELFILES = compile configure depcomp aclocal.m4 src/smaug.h \
	autom4te.cache config.guess config.sub install-sh missing \
	config.status ltmain.sh depcomp m4 README COPYING NEWS src/smaug \
	AUTHORS ChangeLog LICENSE INSTALL config.lt libtool src/.deps src/*.o

CLEANBUILDFILES = config.log \
	`find $(srcdir) -iname "stamp-h1"` \
	`find $(srcdir) -iname "Makefile.in"` \
	`find $(srcdir) -iname "Makefile"`

CLEANFILES = $(CLEANBUILDFILES) \
	`find $(srcdir) -iname "*~"`

all-local:
	@echo "=== all-local done ==="

clean-local:
	@echo "=== clean-local done ==="

distclean-local: clean-local
	@echo "=== distclean-local done ==="

devdistclean-local: distclean-local
	@rm -rfv $(CLEANDEVELFILES); \
	echo "=== distclean-local done ==="

install-data-hook:
	@echo "Installing <@PACKAGE@> data files ..."; \
	mkdir -pv $(sharedstatedir)/@PACKAGE@; \
	cp -rvf data/* $(sharedstatedir)/@PACKAGE@; \
	echo "Creating <@PACKAGE@> directory to store log files ..."; \
	mkdir -pv $(localstatedir)/log/@PACKAGE@; \
	echo "Creating <@PACKAGE@> directories to store lock and run files ..."; \
	mkdir -pv $(localstatedir)/lock/@PACKAGE@; \
	mkdir -pv $(localstatedir)/run/; \
	echo "Creating user <$(USER)> and group <$(GROUP)> ..."; \
	groupadd $(GROUP); \
	useradd $(USER) -g $(GROUP); \
	echo "Setting up <$(prefix)> permissions to <$(USER):$(GROUP)> ..."; \
	chown -R $(USER):$(GROUP) $(prefix); \
	echo ""; \
	echo "<@PACKAGE@> has ben installed to <$(prefix)> directory!"; \
	echo "You can execute <$(sysconfdir)/init.d/smaugd start> now!"; \
	echo "";

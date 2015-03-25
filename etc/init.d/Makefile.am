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

SVdir = $(sysconfdir)/init.d
SV_SCRIPTS = smaugd

CLEANFILES = $(SV_SCRIPTS) *~

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

smaugd: smaugd.in Makefile
	$(do_subst) < $(srcdir)/smaugd.in > smaugd; \
	chmod +x smaugd;

all-local: smaugd

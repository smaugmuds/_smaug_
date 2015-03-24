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

USER = smaug
GROUP = smaug

bin_SCRIPTS = smaug-monitor

CLEANFILES = *~ */*~ $(bin_SCRIPTS)

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

smaug-monitor: smaug-monitor.in Makefile
	$(do_subst) < $(srcdir)/smaug-monitor.in > smaug-monitor; \
	chmod +x smaug-monitor;

all-local: smaug-monitor


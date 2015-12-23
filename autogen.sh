#!/bin/sh

srcdir=`dirname $0`

test -z "$srcdir" && srcdir=.

__intltoolize() {
	if test -d "po"; then
		echo "Running: intltoolize -f -c ..." \
		&& intltoolize --force --copy \
		&& cp /usr/share/aclocal/intltool.m4 m4/ || exit 1
	fi
}

__autogen_verbose() {
	test -d "m4" || mkdir m4

	echo "Running: libtoolize -f -i -r -c ..." \
	&& libtoolize --force --verbose --install --recursive --copy || exit 1

	__intltoolize

	echo "Running: aclocal -I m4 ..." \
	&& aclocal --force --verbose -W all -I m4 \
	&& echo "Running: automake -g -a -c ..." \
	&& automake --gnu --add-missing --copy --verbose -W all  \
	&& echo "Running: autoconf -f ..." \
	&& autoconf --force --verbose -W all || exit 1
}

__autogen() {
	test -d "m4" || mkdir m4

	echo "Running: libtoolize -f -i -r -c ..." \
	&& libtoolize --force --install --recursive --copy || exit 1

	__intltoolize

	echo "Running: aclocal -I m4 ..." \
	&& aclocal --force -W all -I m4 \
	&& echo "Running: automake -g -a -c ..." \
	&& automake --gnu --add-missing --copy -W all  \
	&& echo "Running: autoconf -f ..." \
	&& autoconf --force -W all || exit 1
}

# Options
case "$1" in
        -h|--help) echo "$0 [--help] [--version] [--verbose]"; exit;
        ;;
        -v|--verbose) __autogen_verbose || exit 1;
        ;;
        -V|--version) echo "$0 3.0.0"; exit;
        ;;
	-*) echo "$0 [--help] [--version] [--verbose]"; exit 1;
	;;
        *) __autogen || exit 1;
        ;;
esac

exit

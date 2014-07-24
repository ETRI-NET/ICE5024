#! /bin/sh
aclocal -I .
autoheader -I .
automake --add-missing --copy --foreign
autoconf -f
autoreconf --force --install

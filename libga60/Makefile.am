# most of this file was taken from libgfortran

## Process this file with automake to produce Makefile.in

ACLOCAL_AMFLAGS = -I ../config

## May be used by toolexeclibdir.
gcc_version := $(shell cat $(top_srcdir)/../gcc/BASE-VER)

toolexeclib_LTLIBRARIES = libga60.la
libga60_la_LDFLAGS = -version-info `grep -v '^\#' $(srcdir)/libtool-version` -lm $(extra_ldflags_libga60)

AM_CPPFLAGS = -I$(srcdir)/$(MULTISRCTOP)../gcc \
	      -I$(srcdir)/$(MULTISRCTOP)../gcc/config \
	      -I$(MULTIBUILDTOP)../../$(host_subdir)/gcc -D_GNU_SOURCE

libga60_la_SOURCES = abs.c apow.c entier.c libc-bind.c sign.c

# No install-html support yet.
.PHONY: install-html
install-html:


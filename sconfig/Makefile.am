#***************************************************************************
#*   Copyright (C) 2013 by Volokh                                          *
#*   <volokh84@gmail.com>                                                  *
#*                                                                         *
#*   This program is under GPL 2 licence you may redistribute it and/or    *
#*   modify                                                                *
#*   Ligovskii Pr 203-207, StPetersburg, HomeWork, Russia                  *
#***************************************************************************

GDEBUG=-ggdb -g3 -ggdb3
AM_CFLAGS=-W -Wall -fstack-protector -pipe -fPIC ${GDEBUG}

ACLOCAL_AMFLAGS=-I m4

lib_LTLIBRARIES=libsconfig.la

libsconfig_la_SOURCES=sconfig-lib.c
#libsconfig_la_LIBADD=
#libsconfig_la_CFLAGS=${AM_CFLAGS}

bin_PROGRAMS=sconfig
sconfig_SHORTNAME=sc
sconfig_SOURCES=sconfig-main.cpp
#sconfig_CFLAGS=${AM_CFLAGS}
sconfig_LDADD=libsconfig.la

#includedir=/usr/include/linux/cronyx
includedir=/usr/include/cronyx
include_HEADERS=sconfig.h cserial.h

#$(ETC)/cronyx.conf:
#		@echo "INSTALL $@"
#		@install -m 0644 cronyx.conf $(ETC)/cronyx.conf
sysconfigdir=/etc/
dist_sysconfig_DATA=cronyx.conf

#servicedir=/lib/systemd/user
#dist_service_DATA=rubicon.ss7server.service
#$(MANPATH_RU)/man8/sconfig.8.gz: $(MAKEFILE_LIST) sconfig.ru
#		@echo "MAN sconfig.ru"
#		@mkdir -p $(MANPATH_RU)/man8
#		@cat sconfig.ru $(MAN_ICONV) | gzip > $@

#$(MANPATH_EN)/man8/sconfig.8.gz: $(MAKEFILE_LIST) sconfig.en
#		@echo "MAN sconfig.en"
#		@mkdir -p $(MANPATH_EN)/man8
#		@gzip < sconfig.en > $@
#MAN_PAGES=
#ifneq ($(MANPATH_RU),)
#	MAN_PAGES+=$(MANPATH_RU)/man8/sconfig.8.gz
#endif

#ifneq ($(MANPATH_EN),)
#	MAN_PAGES+=$(MANPATH_EN)/man8/sconfig.8.gz
#endif

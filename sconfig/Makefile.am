#***************************************************************************
#*   Copyright (C) 2013 by Volokh                                          *
#*   <volokh84@gmail.com>                                                  *
#*                                                                         *
#*   This program is under GPL 2 licence you may redistribute it and/or    *
#*   modify                                                                *
#*   Ligovskii Pr 203-207, StPetersburg, HomeWork, Russia                  *
#***************************************************************************

GDEBUG=-ggdb -g3 -ggdb3
AM_CXXFLAGS=-W -Wall -fstack-protector -pipe -fPIC ${GDEBUG}

ACLOCAL_AMFLAGS=-I m4

lib_LTLIBRARIES=libsconfig.la

libsconfig_la_SOURCES=sconfig-lib.c
#libServerAPI_la_LIBADD=-lHash -llgost -lpqxx
#libServerAPI_la_CXXFLAGS=${AM_CXXFLAGS} -I/usr/include/isom -I/usr/include/Metronom \
#  -I/usr/include/RubiconSS7

bin_PROGRAMS=sconfig
sconfig_SHORTNAME=sc
sconfig_SOURCES=sconfig-main.cpp
#RubiconSS7Server_CXXFLAGS=${AM_CXXFLAGS} -I/usr/include/isom -I/usr/include/Metronom \
  #-I/usr/include/RubiconSS7
sconfig_LDADD=libsconfig.la

#includedir=/usr/include/RubiconSS7
#include_HEADERS=*.tcpp ServerApi.h ServerApp.h Permission.h PermissionIface.h

#sysconfigdir=/etc/RubiconSS7
#dist_sysconfig_DATA=RubiconSS7Server.cfg
#servicedir=/lib/systemd/user
#dist_service_DATA=rubicon.ss7server.service

#***************************************************************************
#*   Copyright (C) 2013 by Volokh                                          *
#*   <volokh84@gmail.com>                                                  *
#*                                                                         *
#*   This program is under GPL 2 licence you may redistribute it and/or    *
#*   modify                                                                *
#*   Ligovskii Pr 203-207, StPetersburg, HomeWork, Russia                  *
#***************************************************************************

AC_INIT([sconfig],[__GIT_VERSION__],[volokh84@gmail.com])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_PROG_CXX
AM_PROG_AR
LT_INIT
AM_PROG_CC_C_O
AC_CONFIG_MACRO_DIR([m4])
#AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
  Makefile
])

AC_ARG_WITH(rcdir, [  --with-rcdir=x          Specify path for cronyx.sh rc-script],
		[ RCDIR=$withval ],[ RCDIR=auto ] )
AC_ARG_WITH(rctype, [  --with-rctype=x         Specify your rc.d or init.d sybsystem type]
		[          (update-rc | rc-update | chkconfig | rc.local | default: auto)],
		[ RCTYPE=$withval ],[ RCTYPE=auto ] )
AC_ARG_WITH(rclocal, [  --with-rclocal=x        Specify your rc.local init script (not for RedHat)],
		[ RCLOCAL=$withval ],[ RCLOCAL=auto ] )
AC_ARG_WITH(libmod, [  --with-libmod=x         Specify target path for modules],
		[ LIBMOD=$withval ],[ LIBMOD="auto" ] )
AC_ARG_WITH(manpath, [  --with-manpath=x        Specify path for Manual Pages location],
		[ MANPATH=$withval ],[ MANPATH="auto" ] )
AC_ARG_WITH(manpath_ru, [  --with-manpath-ru=x        Specify path for Russian manual pages location],
		[ MANPATH_RU=$withval ],[ MANPATH_RU="auto" ] )
AC_ARG_WITH(manpath_en, [  --with-manpath-en=x        Specify path for English manual pages location],
		[ MANPATH_EN=$withval ],[ MANPATH_EN="auto" ] )
AC_ARG_WITH(cronyx_major, [  --with-cronyx-major=#   Specify base devnode-major numbers]
		[          for use by drivers (default: 222)],
		[ CRONYX_MAJOR=$withval ],[ CRONYX_MAJOR="222" ] )
AC_ARG_WITH(man_encoding, [  --with-man-encoding=#   Specify charset/encoding for russian man page(s)]
		[          (utf8 | koi8-r | windows-1251 | iso-8859-5 | dos-866 | default: auto).]
		[          The `iconv' utility should be available.],
		[ MAN_ENCODING=$withval ],[ MAN_ENCODING="auto" ] )

# Check for rc.d path
AC_MSG_CHECKING(rc'path)
if test "$RCDIR" != "auto"; then
  if test ! -d "$RCDIR"; then
    AC_MSG_ERROR(The '$RCDIR' doen't exists or isn't a directory. Check --with-rcdir=.. option.)
  fi
  AC_MSG_RESULT($RCDIR)
else
  case `gcc -v 2>&1 | tr [[:upper:]] [[:lower:]] | tr [[:space:]] " "` in
      *gentoo*)
	RCDIR=/etc/init.d;;
      *suse*)
	RCDIR=/etc/init.d;;
      *ubuntu*)
	RCDIR=/etc/init.d;;
      *debian*)
	RCDIR=/etc/init.d;;
      *slackware*)
	RCDIR=/etc/rc.d;;
      *asplinux*)
	RCDIR=/etc/rc.d/init.d;;
      *mandrake*)
	RCDIR=/etc/rc.d/init.d;;
      *redhat*)
        RCDIR=/etc/rc.d/init.d;;
      *)
	if test -d "/etc/rc.d/init.d"; then
	  RCDIR=/etc/rc.d/init.d
	elif test -d "/etc/init.d"; then
	  RCDIR=/etc/init.d
	else
	  RCDIR=/etc/rc.d
	fi;;
  esac
  if test ! -d "$RCDIR"; then
    AC_MSG_ERROR(The '$RCDIR' doen't exists or isn't a directory.)
  fi
  AC_MSG_RESULT($RCDIR ${ac_msg_auto})
fi

# Check for rc type
AC_MSG_CHECKING(rc'type)
if test "$RCTYPE" != "auto"; then
  AC_MSG_RESULT($RCTYPE)
else
  case `gcc -v 2>&1 | tr [[:upper:]] [[:lower:]] | tr [[:space:]] " "` in
      *gentoo*)
  RCTYPE=rc-update;;
      *suse*)
  RCTYPE=chkconfig;;
      *slackware*)
  RCTYPE=rc.local;;
      *ubuntu*)
  RCTYPE=update-rc;;
      *debian*)
  RCTYPE=update-rc;;
      *asplinux*)
  RCTYPE=chkconfig;;
      *mandrake*)
  RCTYPE=chkconfig;;
      *redhat*)
  RCTYPE=chkconfig;;
      *)
  RCTYPE=chkconfig;;
  esac
  AC_MSG_RESULT($RCTYPE ${ac_msg_auto})
fi

# Check for rc.local
if test "$RCTYPE" = "rc.local"; then
  AC_MSG_CHECKING(rc.local)
  if test "$RCLOCAL" != "auto"; then
    if test ! -x "$RCLOCAL"; then
      AC_MSG_ERROR(The '$RCLOCAL' isn't exist or isn't executable. Check --with-rclocal=.. option.)
    fi
    AC_MSG_RESULT($RCLOCAL)
  else
    RCLOCAL=${RCDIR}/rc.local
    if test ! -x "$RCLOCAL"; then
      AC_MSG_ERROR(The '$RCLOCAL' doesn't exist or isn't executable.)
    fi
    AC_MSG_RESULT($RCLOCAL ${ac_msg_auto})
  fi
else
  RCLOCAL="$RCDIR/cronyx"
fi

#AC_MANPATH()

AC_SUBST(KSRC)
AC_SUBST(KERNEL_NAME)
AC_SUBST(KERNEL_VERSION)
AC_SUBST(KERNEL_VERSION_CODE)
AC_SUBST(MODEXT)
AC_SUBST(RCDIR)
AC_SUBST(RCTYPE)
AC_SUBST(RCLOCAL)
AC_SUBST(LIBMOD)
AC_SUBST(WITH_DAHDI)
AC_SUBST(WITH_ZAPTEL)
AC_SUBST(MANPATH)
AC_SUBST(MANPATH_EN)
AC_SUBST(MANPATH_RU)
AC_SUBST(CRONYX_MAJOR)
AC_SUBST(MAN_ENCODING)

ac_msg_done="done :)"
AC_MSG_NOTICE($ac_msg_done)
AC_OUTPUT(cserial.h cronyx.sh cronyx.conf)

# SPDX-License-Identifier: LGPL-2.1-or-later
# Copyright © 2009-2018 ANSSI. All Rights Reserved.
dnl Process this file with autoconf to produce a configure script.
AC_INIT(src/cryptclt.cpp)

dnl Checks for programs.
AC_PROG_CXX

[
if test "x$GCC" = "xyes"; then
  CFLAGS="$CFLAGS -Wall -Wextra \
	-Wstrict-prototypes -Wmissing-prototypes \
	-Wcast-qual -Wcast-align -Wpointer-arith \
	-Wnested-externs"
fi
]

dnl Check for options
AC_ARG_ENABLE(red-client,
	AC_HELP_STRING([--enable-red-client],       
			[build red client (default=no)]),
		build_red=yes, build_red=no)
AC_MSG_CHECKING(if red client should be built)
if test "x$build_red" = xyes; then
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi
AC_SUBST(build_red)

AC_ARG_ENABLE(black-client,
	AC_HELP_STRING([--enable-black-client],       
			[build black client (default=no)]),
		build_black=yes, build_black=no)
AC_MSG_CHECKING(if black client should be built)
if test "x$build_black" = xyes; then
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi
AC_SUBST(build_black)

AC_ARG_ENABLE(diode,
	AC_HELP_STRING([--enable-diode],       
			[build cryptd diode support (default=no)]),
		enable_diode=yes, enable_diode=no)
AC_MSG_CHECKING(if diode support should be built)
if test "x$enable_diode" = xyes; then
	AC_MSG_RESULT(yes)
else
	AC_MSG_RESULT(no)
fi
AC_SUBST(enable_diode)

AC_ARG_WITH(red-socket, 
	AC_HELP_STRING([--with-red-socket=path], 
		[Path where to find the red socket (default: /var/run/cryptd)]),
		[red_socket_path=$withval],
		[red_socket_path=/var/run/cryptd])
if test "x$build_red" = xyes; then
   AC_SUBST(red_socket_path)
   AC_MSG_CHECKING(path to find red socket)
   AC_MSG_RESULT($red_socket_path)
fi

AC_ARG_WITH(black-socket, 
	AC_HELP_STRING([--with-black-socket=path], 
		[Path where to find the black socket (default: /var/run/cryptd)]),
		[black_socket_path=$withval],
		[black_socket_path=/var/run/cryptd])
if test "x$build_black" = xyes; then
   AC_SUBST(black_socket_path)
   AC_MSG_CHECKING(path to find black socket)
   AC_MSG_RESULT($black_socket_path)
fi

AC_ARG_WITH(ccsd-includes, 
	AC_HELP_STRING([--with-ccsd-includes=path], 
		[Path where to find the ccsd-includes (default: /usr/include/ccsd)]),
		[ccsd_includes=$withval],
		[ccsd_includes=/usr/include/ccsd])
if test "x$build_black" = xyes; then
   AC_SUBST(ccsd_includes)
   AC_MSG_CHECKING(path to find ccsd includes)
   AC_MSG_RESULT($ccsd_includes)
fi

dnl Check for dependencies
if test -z "$PKG_CONFIG"; then
  PKG_CONFIG=pkg-config
fi
AC_SUBST(PKG_CONFIG)

AC_MSG_CHECKING(that pkg-config runs)
if "$PKG_CONFIG" --version > /dev/null 2>&1 ; then
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
  AC_MSG_ERROR([*** $PKG_CONFIG missing ***

\'$PKG_CONFIG --version\' failed to run - make sure it is inside one of the
directories in your PATH environment variable!

pkg-config comes with the developement packages for GTK+-2.2, available at:

    http://www.gtk.org])
fi


AC_DEFUN(BUILD_DEPENDS, [
AC_MSG_CHECKING(that $1 (version >= $2) is installed)
if "$PKG_CONFIG" --atleast-version=$2 $1 ; then
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
  if "$PKG_CONFIG" --exists $1 ; then
    ERRSTR=`"$PKG_CONFIG"  --modversion $1`
    AC_MSG_ERROR(Current version is only $ERRSTR)
  else
    AC_MSG_ERROR(Package is not installed)
  fi
fi
])

dnl if test "x$build_red" = xyes; then
dnl    BUILD_DEPENDS(libcryptdcr, 1.1.1)
dnl fi
dnl if test "x$build_black" = xyes; then
dnl    BUILD_DEPENDS(libcryptdcb, 1.1.1)
dnl fi

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(cryptd/cryptd_red.h)
AC_CHECK_HEADERS(cryptd/cryptd_black.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST

dnl Checks for QT lib
PKG_CHECK_MODULES(QT4_CORE, QtCore,,
    AC_MSG_ERROR([[
***
*** Qt4 Core is required.
***]])
)

PKG_CHECK_MODULES(QT4_GUI, QtGui,,
    AC_MSG_ERROR([[
***
*** Qt4 Gui is required.
***]])
)

AC_OUTPUT(Makefile src/Makefile)

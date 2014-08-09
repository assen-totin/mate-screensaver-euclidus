AC_PREREQ([2.63])
AC_INIT([mate-screensaver-euclidus], [1.0], [assen.totin@gmail.com])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([-Wall])

# prefix
if test "x$prefix" = "xNONE" ; then
	prefix="/usr/local"
fi
if test "x$exec_prefix" = "xNONE" ; then
	exec_prefix=$prefix
fi
PREFIX=$prefix
AC_SUBST([PREFIX])
BINDIR=$exec_prefix/bin
AC_SUBST([BINDIR])
SBINDIR=$exec_prefix/sbin
AC_SUBST([SBINDIR])
DATAROOTDIR=$prefix/share
AC_SUBST([DATAROOTDIR])
LIBEXECDIR=$exec_prefix/libexec
if test "$libexecdir" != '${exec_prefix}/libexec'; then
        LIBEXECDIR=$libexecdir
fi
AC_SUBST([LIBEXECDIR])
LIBDIR=${exec_prefix}/lib
if test -d /usr/lib64 ; then
        LIBDIR=${exec_prefix}/lib64
fi
if test "$libdir" != '${exec_prefix}/lib'; then
        if test "$libdir" != '${exec_prefix}/lib64'; then
                LIBDIR=$libdir
        fi
fi
AC_DEFINE_UNQUOTED([LIBDIR], ["$LIBDIR"], ["Directory where application-specific libraries are installed"])
PIXMAPDIR=$prefix/share/pixmaps
AC_SUBST([PIXMAPDIR])
AC_DEFINE_UNQUOTED([IMG_PATH], ["$PIXMAPDIR"], [Define icons location])
ICONSDIR=$prefix/share/icons
AC_SUBST([ICONSDIR])
LOCALEDIR=$prefix/share/locale
AC_SUBST([LOCALEDIR])
AC_DEFINE_UNQUOTED([LOCALEDIR],["$LOCALEDIR"],[Locale files])
TOPLEVELDIR=`pwd`
AC_SUBST([TOPLEVELDIR])

# gettext
AM_GNU_GETTEXT_VERSION(0.18)
AM_GNU_GETTEXT([external])
if test $USE_NLS != "yes" ; then
        echo "*** gettext not found. You're losing internationalization."
fi
AC_DEFINE([GETTEXT_PACKAGE], ["gtk20"], ["GTK gettext"])
AC_DEFINE_UNQUOTED([MATELOCALEDIR], ["$LOCALEDIR"], ["Locale Dir MATE"])

# Checks for programs
AC_PROG_CC
MAKEFILE_CC=`which gcc`
AC_SUBST([MAKEFILE_CC])

# Check for packages
PKG_CHECK_MODULES([GTK], [gtk+-2.0])

PKG_CHECK_MODULES([GLIB], [glib-2.0])

PKG_CHECK_MODULES([GTK_GL_X11], [gtkglext-x11-1.0])

PKG_CHECK_MODULES([GL], [gl])

# Checks for libraries.
AC_SEARCH_LIBS([cos], [m], [], [
  AC_MSG_ERROR([unable to find the cos() function])
])

# Includes and libraries
APPLET_FLAGS=`$PKG_CONFIG --cflags-only-I glib-2.0 gtk+-2.0 gtkglext-x11-1.0 gl`
APPLET_LIBS=`$PKG_CONFIG --libs glib-2.0 gtk+-2.0 gtkglext-x11-1.0 gl`
AC_SUBST([APPLET_FLAGS])
AC_SUBST([APPLET_LIBS])

# MATE or GNOME-2?
HAVE_MATE="0"
HAVE_GNOME_2="0"
LIBEXEC_SUBDIR=""
if [[ -x  $LIBEXECDIR/mate-screensaver ]] ; then
	HAVE_MATE="1";
	LIBEXEC_SUBDIR="mate-screensaver";
	AC_DEFINE([HAVE_MATE], [1], [Define if we have MATE Desktop Environment])
elif [[ -x  $LIBEXECDIR/gnome-screensaver ]] ; then
	HAVE_GNOME_2="0";
	LIBEXEC_SUBDIR="gnome-screensaver";
	AC_DEFINE([HAVE_GNOME_2], [1], [Define if we have GNOME-2 Desktop Environment])
else
	AC_MSG_ERROR(["*** screensaver libexec subdirectory not found."])
fi
AC_SUBST([LIBEXEC_SUBDIR])

# Makefiles
AC_CONFIG_FILES([Makefile src/Makefile images/Makefile po/Makefile.in po/Makefile])

# Service files
AC_CONFIG_FILES([mate-screensaver-euclidus.spec])

# Icons
AC_DEFINE_UNQUOTED([APPLET_ICON_PATH], ["$PIXMAPDIR"], [Define icons location])

# Grand finale
AC_OUTPUT

echo
echo "Configuration complete:"
echo
echo "Now, run make."
echo 

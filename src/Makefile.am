#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Library General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
#  USA.
#
#  MATE Livescore applet written by Assen Totin <assen.totin@gmail.com>
#

APPLET_FLAGS=@APPLET_FLAGS@
APPLET_LIBS=@APPLET_LIBS@
LIBEXEC_SUBDIR=@LIBEXEC_SUBDIR@

AM_CPPFLAGS = @GTK_CFLAGS@ @APPLET_FLAGS@
LDADD = @GTK_LIBS@ @APPLET_LIBS@ -lm

scrdir = $(libexecdir)/$(LIBEXEC_SUBDIR)
scr_PROGRAMS = mate-screensaver-euclidus

mate_screensaver_euclidus_SOURCES = euclidus.h euclidus.c gs-theme-window.h gs-theme-window.c
if MAKEFILE_GTK3
LDADD += -lGLU
mate_screensaver_euclidus_SOURCES += gtk_opengl.c gtk_opengl.h
endif

desktopdir = $(datadir)/applications
desktop_DATA = mate-screensaver-euclidus.desktop



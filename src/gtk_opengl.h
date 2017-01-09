/*******************************************************************************
 * Copyright (C) 2010, 2014 Carlos Pereira, Javier Cabezas
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <GL/gl.h>
#include <GL/glx.h>

int gtk_opengl_query (void);

GLXContext gtk_opengl_create (GtkWidget *area, int *attributes,
GLXContext context, int direct);

void gtk_opengl_remove (GtkWidget *area, GLXContext context);

int gtk_opengl_current (GtkWidget *area, GLXContext context);

void gtk_opengl_swap (GtkWidget *area);

void gtk_opengl_wait_gl (void);

void gtk_opengl_wait_x (void);

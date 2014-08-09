/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 *  USA.
 *
 *  MATE Euclidus screensaver written by Assen Totin <assen.totin@gmail.com>
 *  
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <GL/gl.h> 
#include <GL/glu.h>
#include "../config.h"

#define FRAMES_PER_SECOND 60	// FPS
#define FRAMES_PER_STAGE 60
#define LENGTH_HOUR 0.25		// Of the half-height of the srceen
#define LENGTH_MINUTE 0.4		// NB: to fit in the screen keep LENGTH_SECOND + LENGTH_MINUTE < 1. 
#define LENGTH_SECOND 0.55
#define LINE_THICK 3
#define LINE_THIN 2
#define IMAGE_ODD "als-logo.png"
#define IMAGE_EVEN "als-logo-inverse.png"

enum {
	COLOUR_BLACK = 0,
	COLOUR_WHITE,
	COLOUR_RED,
	COLOUR_GRAY
};

enum {
	STAGE_NONE = 0,
	STAGE_HOUR,
	STAGE_MINUTE,
	STAGE_SECOND,
	STAGE_GRAY1,
	STAGE_GRAY2,
	STAGE_IMAGE,
	STAGE_READY
};

typedef struct {
	guchar *data;
	unsigned int width;
	unsigned int height;
	unsigned int bytes_per_pixel;
	unsigned int size;
	char path[1024];
	GLuint texture;
} imageData; 

typedef struct {
	imageData *image_odd;
	imageData *image_even;
	unsigned int stage;
	unsigned int stage_frames;
	int img_frames;
	float coord_img[8];
	float mul_h; 
	float mul_m;
	float mul_s;
	float mul_g1;
	float mul_g2;
	float mul_img;
	int tm_min;
} ssData; 

static gboolean deleteEvent(GtkWidget *widget, GdkEvent *event, gpointer data);
	
static gboolean exposeCb(GtkWidget *drawingArea, GdkEventExpose *event, gpointer userData);
	
static gboolean idleCb(gpointer userData); 

static gboolean configureCb(GtkWidget *drawingArea, GdkEventConfigure *event, gpointer user_data); 
	
static void startEventLoop(); 
	
int initWindow(int argc, char *argv[]); 

void calculate_coordinates(float angle, float length, float *res);

void calculate_coordinates_img(imageData *image, int width, int height, float *res);

void draw_line(int width, int colour, float *coord_from, float *coord_to, float mul);

void load_image(imageData *image);


/*
 * This file is intended to give a broad overview with tons of comments.
 * This program is based off of a gtk/gl example called teapot.c, checked in by tristan
 * Find it here: https://svn.sat.qc.ca/trac/miville/browser/inhouse/prototypes/openGL/teapot.c?rev=1285	
 * 
 * Please make use of this code! --Ankillito
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <string.h>
#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <GL/gl.h> 
#include <GL/glu.h>
#include "gs-theme-window.h"
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

static gboolean deleteEvent(GtkWidget *widget, GdkEvent *event, gpointer data); //gtk event when window is closed - probably won't edit
	
static gboolean exposeCb(GtkWidget *drawingArea, GdkEventExpose *event, gpointer userData);
/*
 * This is the important function.  This draws stuff when the window is resized, moved, or made visable (ie, brought out
 *  from underneath antother window). This won't really happen as a screenaver other than once when the program starts
 *  up.  However, we use the idle function to call this function again, so this basically is the animation function.
 *  Put your graphical code here!
 */
	
static gboolean idleCb(gpointer userData); //When idle, ask that the drawing area is redrawn via exbose call back - probably won't edit

static gboolean configureCb(GtkWidget *drawingArea, GdkEventConfigure *event, gpointer user_data); //Set up openGL - set your enables and stuff here
	
int initWindow(int argc, char *argv[]); //Set up the window, callbacks, and initialize gtkglext  - probably won't edit

//And the main function

void calculate_coordinates(float angle, float length, float *res);

void calculate_coordinates_img(imageData *image, int width, int height, float *res);

void draw_line(int width, int colour, float *coord_from, float *coord_to, float mul);

void load_image(imageData *image);


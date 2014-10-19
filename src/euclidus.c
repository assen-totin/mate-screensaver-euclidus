/*
 *  See the header file for more abstract comments and credits.
 */
#include "euclidus.h" 

/**
 * Main command-line function.
 * Helps avoid spawning a gui for command-line-only stuff.
 * @param argc int Number of entries in agrv.
 * @param argv *char[] Array of pointers to command-line arguments, including command name.
 * @returns int Exit code.
 */
int main(int argc, char *argv[]) {
	if(initWindow(argc, argv))
		gtk_main();

        // Gettext *really* wants to have at least one invocation, so keep it happy
        char a[1024];
        snprintf(&a[0], 1023, "%s\n", _("Dummy string"));

	return 0;
}


/**
 * Main GUI function.
 * @param argc int Number of entries in agrv.
 * @param argv *char[] Array of pointers to command-line arguments, including command name.
 * @returns int Exit code.
 */
int initWindow(int argc, char *argv[]) {
	int i, width, height;
	gboolean fullscreen = FALSE;
	GtkWidget *window;
	GtkWidget *drawingArea;
	GdkScreen* screen = NULL;
	GdkGLConfig *glConfig;
	ssData *ss_data = malloc(sizeof(ssData));

	// Init GTK
	gtk_init(&argc, &argv);

	// Check for any options that would not run the screensaver
	for (i=1; i<argc; i++) { 
		if ((strcmp("-h", argv[i]) == 0) || (strcmp("--help", argv[i]) == 0)) {
			printf("Usage: myprogram [OPTION...]\nOptions:\n  -root  Fullscreen\n  --help Display this\n");
			return 0;
		}
    	else if ((strcmp("-root",argv[i]) == 0)  || (strcmp("--root",argv[i]) == 0))
			fullscreen = TRUE;
	}

	// Init screensaver window: gnome-screensaver expects gs_theme_window, so don't use the usual 'window = gtk_window_new(GTK_WINDOW_TOPLEVEL);')
	window = gs_theme_window_new();

	// Set the non-full screen to something proportional to the screen size.  (1/2)
	screen = gtk_window_get_screen(GTK_WINDOW(window)); //Notice that window is a pointer to a GtkWidget, so it must get cast when passed in
	width = gdk_screen_get_width(screen);
	height = gdk_screen_get_height(screen);
	gtk_window_set_default_size(GTK_WINDOW(window), width/2, height/2);

	if(fullscreen) 
		gtk_window_fullscreen(GTK_WINDOW(window));

	// Init drawing area, add it to the window
	drawingArea = gtk_drawing_area_new();
	gtk_container_add(GTK_CONTAINER(window), drawingArea);

	// Register window callbacks
	g_signal_connect_swapped(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(G_OBJECT(window), "delete-event", G_CALLBACK(deleteEvent), NULL);
	gtk_widget_set_events(drawingArea, GDK_EXPOSURE_MASK);

	// Init OpenGL within the contex of gdk
	gtk_gl_init(&argc, &argv);
	glConfig = gdk_gl_config_new_by_mode(GDK_GL_MODE_RGBA | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE);

	if (!glConfig)
		g_assert_not_reached();

	if (!gtk_widget_set_gl_capability(drawingArea, glConfig, NULL, TRUE, GDK_GL_RGBA_TYPE))
		g_assert_not_reached();

	// Set up callback data
	ss_data->image_odd = (imageData *) malloc(sizeof(imageData));
	sprintf(&ss_data->image_odd->path[0], "%s/%s", IMG_PATH, IMAGE_ODD);
	load_image(ss_data->image_odd);

	ss_data->image_even = (imageData *) malloc(sizeof(imageData));
	sprintf(&ss_data->image_even->path[0], "%s/%s", IMG_PATH, IMAGE_EVEN);
	load_image(ss_data->image_even);

	// Calculate image coordinates
	calculate_coordinates_img(ss_data->image_even, width, height, ss_data->coord_img);

	// Init counters
	ss_data->stage = 0;
	ss_data->stage_frames = 0;

	// Connect callback for window configuration - will be called only once
	g_signal_connect(drawingArea, "configure-event", G_CALLBACK(configureCb), ss_data);
	// Connect callback for window redraw - will be called every time we need to redraw due to becoming visible or being resized
	g_signal_connect(drawingArea, "expose-event", G_CALLBACK(exposeCb), ss_data);

	// Set timeout between frame rendering, call the rendering funciton at it.
	const gdouble TIMEOUT_PERIOD = 1000 / FRAMES_PER_SECOND;
	g_timeout_add(TIMEOUT_PERIOD, idleCb, drawingArea);

	// Run the show
	gtk_widget_show_all(window);
	return 1;
}


/**
 * OpenGL configuration function.
 * @param drawingArea *GtkWidget The widget to draw the OpenGL stuff to.
 * @param event *GdkEventConfigure The name of the event that triggered the function.
 * @param gpointer user_data Pointer to any user-supplied data when the callback is invoked.
 * @returns boolean TRUE on succcessful configuration, FALSE otherwise.
 */
static gboolean configureCb(GtkWidget *drawingArea, GdkEventConfigure *event, gpointer user_data) {
	GdkGLContext *glContext = gtk_widget_get_gl_context(drawingArea);
	GdkGLDrawable *glDrawable = gtk_widget_get_gl_drawable(drawingArea);
	ssData *ss_data = user_data;

	// Enter the OpenGL state
	if (!gdk_gl_drawable_gl_begin(glDrawable, glContext))
		g_assert_not_reached();

	// Specify what part of the screen to draw to - all of it (lower left corner, top right corner)
	glViewport(0,0,drawingArea->allocation.width, drawingArea->allocation.height);
  
	// Select & reset the Projection Matrix
	glMatrixMode(GL_PROJECTION);	
	glLoadIdentity();

	// Specify projection
	gluPerspective(45.0f,(GLfloat)drawingArea->allocation.width/(GLfloat)drawingArea->allocation.height, 0.1f, 100.0f);  

	// Use Modelview Matrix and reset it
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

    // Choose our GL options
	glEnable(GL_LINE_SMOOTH);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);

	// End OpenGL state
	gdk_gl_drawable_gl_end(glDrawable);

	// Create texture for ODD image
	glGenTextures(1, &ss_data->image_odd->texture);
	glBindTexture(GL_TEXTURE_2D, ss_data->image_odd->texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, 3, ss_data->image_odd->width, ss_data->image_odd->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, ss_data->image_odd->data);

	// Create texture for EVEN image
	glGenTextures(1, &ss_data->image_even->texture);
	glBindTexture(GL_TEXTURE_2D, ss_data->image_even->texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, 3, ss_data->image_even->width, ss_data->image_even->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, ss_data->image_even->data);

	return TRUE;
}


/**
 * OpenGL main rendering function.
 * @param drawingArea *GtkWidget The widget to draw the OpenGL stuff to.
 * @param event *GdkEventExpose The name of the event that triggered the function.
 * @param gpointer user_data Pointer to any user-supplied data when the callback is invoked.
 * @returns boolean TRUE on succcessful rendering, FALSE otherwise.
 */
static gboolean exposeCb(GtkWidget *drawingArea, GdkEventExpose *event, gpointer user_data) {
	struct timeval now_utc;
	struct tm now;
	div_t q_ms;
	float mul_img_in = 1, mul_img_out = 1;
	float coord_centre[2], coord_h[2], coord_m[2], coord_s[2], coord_hm[2], coord_hs[2], coord_ms[2], coord_hms[2];
	float angle_h, angle_m, angle_s;
	ssData *ss_data = user_data;

	// Process current time
	gettimeofday(&now_utc, NULL);
	q_ms = div(now_utc.tv_usec, 1000);
	localtime_r(&now_utc.tv_sec, &now);

	if (now.tm_hour > 11)
		now.tm_hour -= 12;

	// Increment the frames counter if we are still preparing the stage
	if (ss_data->stage < STAGE_READY)
		ss_data->stage_frames++;
	
	// Calculate line length multiplier for each stage
	switch (ss_data->stage) {
		case STAGE_HOUR:
			ss_data->mul_h = (float)ss_data->stage_frames / FRAMES_PER_STAGE;
			break;
		case STAGE_MINUTE:
			ss_data->mul_m = (float)ss_data->stage_frames / FRAMES_PER_STAGE;
			break;
		case STAGE_SECOND:
			ss_data->mul_s = (float)ss_data->stage_frames / FRAMES_PER_STAGE;
			break;
		case STAGE_GRAY1:
			ss_data->mul_g1 = (float)ss_data->stage_frames / FRAMES_PER_STAGE;
			break;
		case STAGE_GRAY2:
			ss_data->mul_g2 = (float)ss_data->stage_frames / FRAMES_PER_STAGE;
			break;
		case STAGE_IMAGE:
			ss_data->mul_img = (float)ss_data->stage_frames / FRAMES_PER_STAGE;
			break;
		case STAGE_READY:
			if (ss_data->tm_min != now.tm_min)
				// Force initial value to 1 and not 0 in order to have mul_img_out < 1 on the very first frame of the minute
				ss_data->img_frames = 1;

			if (ss_data->img_frames >= 0) {
				mul_img_out = (float)(FRAMES_PER_STAGE - ss_data->img_frames) / FRAMES_PER_STAGE;
				if (mul_img_out < 0) {
					mul_img_out = 1;
					mul_img_in = (float)(ss_data->img_frames - FRAMES_PER_STAGE) / FRAMES_PER_STAGE;
				}
				ss_data->img_frames++;
			}

			if ((ss_data->img_frames >= 2 * FRAMES_PER_STAGE) || (now.tm_sec > 30)) {
				mul_img_in = 1;
				ss_data->img_frames = -1;
			}

			break;
		default:
			ss_data->stage = STAGE_HOUR;
	}

	// Advance stage and reset stage counter when the stage counter reaches threshold
	if (ss_data->stage < STAGE_READY) {
		if (ss_data->stage_frames > FRAMES_PER_STAGE) {
			ss_data->stage_frames = 0;
			ss_data->stage++;
		}
	}

	// Remember current minute
	ss_data->tm_min = now.tm_min;

	// Extract the GTK GL objects from the widget
	GdkGLContext *glContext = gtk_widget_get_gl_context(drawingArea);
	GdkGLDrawable *glDrawable = gtk_widget_get_gl_drawable(drawingArea);

	// Start GL state
	if (!gdk_gl_drawable_gl_begin(glDrawable, glContext))
		g_assert_not_reached();

	// Reset to orgin; at this point, continue with regular OpenGL code
	glLoadIdentity();  
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set up positions: eye position, target position, up vector direction
	gluLookAt(0,0,-3,  0,0,0,  0,1,0);

	// CALCULATIONS
	// Because calculations need to happen in different order than rendering, do all of them first

	// Set where our centre is
	coord_centre[0] = 0;
	coord_centre[1] = 0;

	// Hour arm position
	angle_h = (float)(now.tm_hour * 60 + now.tm_min)/ (12 * 60) * 2 * M_PI;
	calculate_coordinates(angle_h, LENGTH_HOUR, &coord_h[0]);

	// Minute arm position
	if (ss_data->stage >= STAGE_MINUTE) {
		angle_m = (float)(now.tm_min * 60 +  now.tm_sec) / (60 * 60) * 2 * M_PI;
		calculate_coordinates(angle_m, LENGTH_MINUTE, &coord_m[0]);
	}

	// Second arm position
	if (ss_data->stage >= STAGE_SECOND) {
		angle_s = (float)(now.tm_sec * 1000 + q_ms.quot) / (60 * 1000) * 2 * M_PI;
		calculate_coordinates(angle_s, LENGTH_SECOND, &coord_s[0]);
	}

	// Gray lines from the end of hour, minute and second arms
	if (ss_data->stage >= STAGE_GRAY1) {
		// From hour arm - parallel to the minute arm
		// From minute arm - parallel to the hour arm
		coord_hm[0] = coord_h[0] + coord_m[0];
		coord_hm[1] = coord_h[1] + coord_m[1];

		// From hour arm - parallel to the seconds arm
		// From seconds arm - parallel to the hour arm
		coord_hs[0] = coord_h[0] + coord_s[0];
		coord_hs[1] = coord_h[1] + coord_s[1];

		// From minute arm - parallel to the seconds arm
		// From seconds arm - parallel to the minute arm
		coord_ms[0] = coord_m[0] + coord_s[0];
		coord_ms[1] = coord_m[1] + coord_s[1];
	}

	// Gray lines from the remote top corner - parallel to the three gray lines 
	if (ss_data->stage >= STAGE_GRAY2) {
		// Coordinates of the remote joining point
		coord_hms[0] = coord_h[0] + coord_m[0] + coord_s[0];
		coord_hms[1] = coord_h[1] + coord_m[1] + coord_s[1];
	}
	
	// DRAWING - bottom up

	// Image
	if (ss_data->stage >= STAGE_IMAGE) {
		glEnable(GL_TEXTURE_2D);

		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

		// Determine which textute to bind
		if (now.tm_min % 2 > 0) {
			// On odd minutes we bind the odd texture except when the even one has to fade out
			if (mul_img_out < 1)
				glBindTexture(GL_TEXTURE_2D, ss_data->image_even->texture);
			else
				glBindTexture(GL_TEXTURE_2D, ss_data->image_odd->texture);
		}
		else {
			// On even minutes we bind the even texture except when the even one has to fade out
			if (mul_img_out < 1)
				glBindTexture(GL_TEXTURE_2D, ss_data->image_odd->texture);
			else
				glBindTexture(GL_TEXTURE_2D, ss_data->image_even->texture);
		}

		glBegin(GL_QUADS);
                // Determine which multiplier to use
                if (ss_data->mul_img < 1)
                        glColor4f(1.0, 1.0, 1.0, ss_data->mul_img);
                else if (mul_img_out < 1)
                        glColor4f(1.0, 1.0, 1.0, mul_img_out);
                else if (mul_img_in < 1)
                        glColor4f(1.0, 1.0, 1.0, mul_img_in);

		glTexCoord2f(0, 0); glVertex3f(ss_data->coord_img[0], ss_data->coord_img[1], 0);
		glTexCoord2f(0, 1); glVertex3f(ss_data->coord_img[2], ss_data->coord_img[3], 0);
		glTexCoord2f(1, 1); glVertex3f(ss_data->coord_img[4], ss_data->coord_img[5], 0);
		glTexCoord2f(1, 0); glVertex3f(ss_data->coord_img[6], ss_data->coord_img[7], 0);
		glEnd();

		glDisable(GL_TEXTURE_2D);
	}

	// Gray lines from the remote top corner - parallel to the three gray lines 
	if (ss_data->stage >= STAGE_GRAY2) {
		draw_line(LINE_THIN, COLOUR_GRAY, &coord_hm[0], &coord_hms[0], ss_data->mul_g2);
		draw_line(LINE_THIN, COLOUR_GRAY, &coord_ms[0], &coord_hms[0], ss_data->mul_g2);
		draw_line(LINE_THIN, COLOUR_GRAY, &coord_hs[0], &coord_hms[0], ss_data->mul_g2);
	}

	// Gray lines from the end of hour, minute and second arms
	if (ss_data->stage >= STAGE_GRAY1) {
		// From hour arm - parallel to the minute arm
		// From hour arm - parallel to the second arm
		draw_line(LINE_THIN, COLOUR_GRAY, &coord_h[0], &coord_hm[0], ss_data->mul_g1);
		draw_line(LINE_THIN, COLOUR_GRAY, &coord_h[0], &coord_hs[0], ss_data->mul_g1);

		// From minute arm - parallel to the hour arm
		// From minute arm - parallel to the second arm
		draw_line(LINE_THIN, COLOUR_GRAY, &coord_m[0], &coord_hm[0], ss_data->mul_g1);
		draw_line(LINE_THIN, COLOUR_GRAY, &coord_m[0], &coord_ms[0], ss_data->mul_g1);

		// From seconds arm - parallel to the hour arm
		// From seconds arm - parallel to the minute arm
		draw_line(LINE_THIN, COLOUR_GRAY, &coord_s[0], &coord_hs[0], ss_data->mul_g1);
		draw_line(LINE_THIN, COLOUR_GRAY, &coord_s[0], &coord_ms[0], ss_data->mul_g1);
	}

	// Hour arm
	draw_line(LINE_THICK, COLOUR_WHITE, &coord_centre[0], &coord_h[0], ss_data->mul_h);

	// Minute arm
	if (ss_data->stage >= STAGE_MINUTE) {
		draw_line(LINE_THICK, COLOUR_WHITE, &coord_centre[0], &coord_m[0], ss_data->mul_m);
	}

	// Second arm
	if (ss_data->stage >= STAGE_SECOND) {
		draw_line(LINE_THICK, COLOUR_RED, &coord_centre[0], &coord_s[0], ss_data->mul_s);
	}

	// Swap buffer if we're using double-buffering
	if (gdk_gl_drawable_is_double_buffered(glDrawable))     
		gdk_gl_drawable_swap_buffers(glDrawable); 
	else
		glFlush();

	// End the OpenGL state
	gdk_gl_drawable_gl_end (glDrawable);

	return TRUE;
}
	

/**
 * OpenGL idle callback function.
 * @param gpointer user_data Pointer to any user-supplied data when the callback is invoked.
 * @returns boolean TRUE on succcessful rendering, FALSE otherwise.
 */	
static gboolean idleCb(gpointer user_data) {
	GtkWidget *drawingArea = GTK_WIDGET(user_data);

    // Invalidate drawing area, marking it "dirty" and to be redrawn when
    // main loop signals expose-events, which it does as needed when it returns
	gdk_window_invalidate_rect(drawingArea->window, &drawingArea->allocation, FALSE);

	return TRUE;
}


/**
 * OpenGL callback function for 'delete' event (exit).
 * @param widget *GtkWidget The widget which received the 'delete' event.
 * @param event *GdkEvent The name of the event that triggered the function.
 * @param gpointer user_data Pointer to any user-supplied data when the callback is invoked.
 * @returns boolean TRUE on succcessful rendering, FALSE otherwise.
 */	
static gboolean deleteEvent(GtkWidget *widget, GdkEvent *event, gpointer data) {
	return FALSE;
}

/**
 * Helper function to calculate coordinates of arm's end based on it's angle and length
 * @param angle float The angle of rotation, in rad, clockwise, 12h = 0.
 * @param length float_data The length of the arm, between 0 and 1.
 * @param res *float Pointer to a float[2] to hold the calculated coordinates, [x,y]; NB: +1/+1 is top left, -1/-1 is bottom right.
 */	
void calculate_coordinates(float angle, float length, float *res) {
	float tmp;
	tmp = -1 * sin(angle) * length;
	*res = tmp;
	res++;
	tmp = cos(angle) * length;
	*res = tmp;
}


/**
 * Helper function to calculate coordinates of an image
 * @param *imageData image The image structure
 * @param width int The physical width of the screen in pixels
 * @param height int The physical height of the screen in pixels
 * @param res *float Pointer to a float[8] to hold the calculated coordinates, 4 pairs of [x,y]; order is BR, TR, TL, BL; NB: +1/+1 is top left, -1/-1 is bottom right.
 */	
void calculate_coordinates_img(imageData *image, int width, int height, float *res) {
	float x = (float)image->width / height;
	float y = (float)image->height / height;

	// Bottom right, [x,y]
	*res = -1 * x;
	res++;
	*res = -1;
	res++;

	// Top right, [x,y]
	*res = -1 * x;
	res++;
	*res = -1 + y * 2;
	res++;

	// Top left, [x,y]
	*res = x;
	res++;
	*res = -1 + y * 2;
	res++;

	// Top right, [x,y]
	*res = x;
	res++;
	*res = -1;
	res++;
}


/**
 * Helper function to draw a line between two points with a specified width, colour; only specified percentage of the line will be drawn visible
 * @param width int The width of the line in pixels.
 * @param colour int The colour of the line.
 * @param *coord_from float Pointer to a float[2] which hold the coordinates of the starting point, [x,y].
 * @param *coord_to float Pointer to a float[2] which hold the coordinates of the edning point, [x,y].
 * @param mul float Multiplier which denotes what part of the lite to actually draw; must be between 0 and 1. 
 */	
void draw_line(int width, int colour, float *coord_from, float *coord_to, float mul) {
	glLineWidth(width);

	switch(colour) {
		case COLOUR_BLACK: 
			glColor3f(0, 0, 0);
			break;
		case COLOUR_WHITE: 
			glColor3f(1.0, 1.0, 1.0);
			break;
		case COLOUR_RED: 
			glColor3f(1.0, 0, 0);
			break;
		case COLOUR_GRAY:
			glColor3f(0.3, 0.3, 0.3);
			break;
	}

	float to_x = *coord_from + mul * (*coord_to - *coord_from);
	float to_y = *(coord_from + 1) + mul * (*(coord_to + 1) - *(coord_from + 1));

	glBegin(GL_LINES);
	glVertex3f(*coord_from, *(coord_from + 1), 0);
	glVertex3f(to_x, to_y, 0);
	glEnd();
}


/**
 * Helper function to load in image from file
 * @param *imageData image The image structure to fill in with the loaded image (must contain the path ot the image)
 */	
void load_image(imageData *image) {
	int i, j;
	guchar *gtk_pixels;

	GtkWidget *img = gtk_image_new_from_file(&image->path[0]);
	GdkPixbuf *pb = gtk_image_get_pixbuf(GTK_IMAGE(img));
	GdkPixbuf *pba = gdk_pixbuf_add_alpha (pb, FALSE, 0, 0, 0);
	gtk_pixels = gdk_pixbuf_get_pixels(pba);

	image->width = gdk_pixbuf_get_width(pba);
	image->height = gdk_pixbuf_get_height(pba);
	image->bytes_per_pixel = gdk_pixbuf_get_n_channels(pba);

	image->size = image->height * image->width * image->bytes_per_pixel;
	image->data = (guchar *) malloc(image->size);

	// Reverse the image from GTK+ order to OpenGL order - transpone the image
	for (i=0; i<image->size; i+=image->bytes_per_pixel) {
		for (j=0; j<image->bytes_per_pixel; j++) {
			image->data[image->size - image->bytes_per_pixel - i + j] = gtk_pixels[i + j];
		}
	}
}


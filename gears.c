/* sdlGears.c */
/*
 * 3-D gear wheels by Brian Paul. This program is in the public domain.
 */

//#define PLAY_MUSIC

#include <Protocol/GraphicsOutput.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/GL/gl.h"

#include "../include/zbuffer.h"
#define CHAD_API_IMPL
#define CHAD_MATH_IMPL
#include "../include-demo/3dMath.h"

#define SDL_GetTicks() 0
#define printf(...) DEBUG((EFI_D_ERROR, __VA_ARGS__))

typedef unsigned char uchar;

//#include <SDL/SDL.h>
int noSDL = 0;
#ifndef M_PI
#define M_PI 3.14159265
#endif
int override_drawmodes = 0;
GLubyte stipplepattern[128] = {0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
							   0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,

							   0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
							   0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,

							   0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
							   0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,

							   0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
							   0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55};

/*
 * Draw a gear wheel.  You'll probably want to call this function when
 * building a display list since we do a lot of trig here.
 *
 * Input:  inner_radius - radius of hole at center
 *         outer_radius - radius at center of teeth
 *         width - width of gear
 *         teeth - number of teeth
 *         tooth_depth - depth of tooth
 */
static void gear(GLfloat inner_radius, GLfloat outer_radius, GLfloat width, GLint teeth, GLfloat tooth_depth) {
	GLint i;
	GLfloat r0, r1, r2;
	GLfloat angle, da;
	GLfloat u, v, len;

	r0 = inner_radius;
	r1 = outer_radius - tooth_depth / 2.0;
	r2 = outer_radius + tooth_depth / 2.0;

	da = 2.0 * M_PI / teeth / 4.0;

	glNormal3f(0.0, 0.0, 1.0);

	/* draw front face */
	if (override_drawmodes == 1)
		glBegin(GL_LINES);
	else if (override_drawmodes == 2)
		glBegin(GL_POINTS);
	else {
		glBegin(GL_QUAD_STRIP);
	}
	for (i = 0; i <= teeth; i++) {
		angle = i * 2.0 * M_PI / teeth;
		glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5);
		glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5);
		glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5);
		glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5);
	}
	glEnd();

	/* draw front sides of teeth */
	if (override_drawmodes == 1)
		glBegin(GL_LINES);
	else if (override_drawmodes == 2)
		glBegin(GL_POINTS);
	else
		glBegin(GL_QUADS);
	da = 2.0 * M_PI / teeth / 4.0;
	for (i = 0; i < teeth; i++) {
		angle = i * 2.0 * M_PI / teeth;

		glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5);
		glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), width * 0.5);
		glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), width * 0.5);
		glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5);
	}
	glEnd();

	glNormal3f(0.0, 0.0, -1.0);

	/* draw back face */
	if (override_drawmodes == 1)
		glBegin(GL_LINES);
	else if (override_drawmodes == 2)
		glBegin(GL_POINTS);
	else
		glBegin(GL_QUAD_STRIP);
	for (i = 0; i <= teeth; i++) {
		angle = i * 2.0 * M_PI / teeth;
		glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5);
		glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5);
		glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width * 0.5);
		glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5);
	}
	glEnd();

	/* draw back sides of teeth */
	if (override_drawmodes == 1)
		glBegin(GL_LINES);
	else if (override_drawmodes == 2)
		glBegin(GL_POINTS);
	else
		glBegin(GL_QUADS);
	da = 2.0 * M_PI / teeth / 4.0;
	for (i = 0; i < teeth; i++) {
		angle = i * 2.0 * M_PI / teeth;

		glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width * 0.5);
		glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), -width * 0.5);
		glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), -width * 0.5);
		glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5);
	}
	glEnd();

	/* draw outward faces of teeth */
	if (override_drawmodes == 1)
		glBegin(GL_LINES);
	else if (override_drawmodes == 2)
		glBegin(GL_POINTS);
	else
		glBegin(GL_QUAD_STRIP);
	for (i = 0; i < teeth; i++) {
		angle = i * 2.0 * M_PI / teeth;

		glVertex3f(r1 * cos(angle), r1 * sin(angle), width * 0.5);
		glVertex3f(r1 * cos(angle), r1 * sin(angle), -width * 0.5);
		u = r2 * cos(angle + da) - r1 * cos(angle);
		v = r2 * sin(angle + da) - r1 * sin(angle);
		len = sqrt(u * u + v * v);
		u /= len;
		v /= len;
		glNormal3f(v, -u, 0.0);
		glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), width * 0.5);
		glVertex3f(r2 * cos(angle + da), r2 * sin(angle + da), -width * 0.5);
		glNormal3f(cos(angle), sin(angle), 0.0);
		glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), width * 0.5);
		glVertex3f(r2 * cos(angle + 2 * da), r2 * sin(angle + 2 * da), -width * 0.5);
		u = r1 * cos(angle + 3 * da) - r2 * cos(angle + 2 * da);
		v = r1 * sin(angle + 3 * da) - r2 * sin(angle + 2 * da);
		glNormal3f(v, -u, 0.0);
		glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), width * 0.5);
		glVertex3f(r1 * cos(angle + 3 * da), r1 * sin(angle + 3 * da), -width * 0.5);
		glNormal3f(cos(angle), sin(angle), 0.0);
	}

	glVertex3f(r1 * cos(0), r1 * sin(0), width * 0.5);
	glVertex3f(r1 * cos(0), r1 * sin(0), -width * 0.5);

	glEnd();

	/* draw inside radius cylinder */
	if (override_drawmodes == 1)
		glBegin(GL_LINES);
	else if (override_drawmodes == 2)
		glBegin(GL_POINTS);
	else
		glBegin(GL_QUAD_STRIP);
	for (i = 0; i <= teeth; i++) {
		angle = i * 2.0 * M_PI / teeth;
		glNormal3f(-cos(angle), -sin(angle), 0.0);
		glVertex3f(r0 * cos(angle), r0 * sin(angle), -width * 0.5);
		glVertex3f(r0 * cos(angle), r0 * sin(angle), width * 0.5);
	}
	glEnd();
}

static GLfloat view_rotx = 20.0, view_roty = 30.0;
static GLint gear1, gear2, gear3;
static GLfloat angle = 0.0;

void draw() {
	angle += 2.0;
	glPushMatrix();
	glRotatef(view_rotx, 1.0, 0.0, 0.0);
	glRotatef(view_roty, 0.0, 1.0, 0.0);
	// glRotatef( view_rotz, 0.0, 0.0, 1.0 );

	glPushMatrix();
	glTranslatef(-3.0, -2.0, 0.0);
	glRotatef(angle, 0.0, 0.0, 1.0);
	glCallList(gear1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(3.1, -2.0, 0.0);
	glRotatef(-2.0 * angle - 9.0, 0.0, 0.0, 1.0);
	glCallList(gear2);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-3.1, 4.2, 0.0);
	glRotatef(-2.0 * angle - 25.0, 0.0, 0.0, 1.0);
	glCallList(gear3);
	glPopMatrix();

	glPopMatrix();
}

void initScene() {
	// static GLfloat pos[4] = {0.408248290463863, 0.408248290463863, 0.816496580927726, 0.0 }; //Light at infinity.
	static GLfloat pos[4] = {5, 5, 10, 0.0}; // Light at infinity.
	// static GLfloat pos[4] = {5, 5, -10, 0.0}; // Light at infinity.

	static GLfloat red[4] = {1.0, 0.0, 0.0, 0.0};
	static GLfloat green[4] = {0.0, 1.0, 0.0, 0.0};
	static GLfloat blue[4] = {0.0, 0.0, 1.0, 0.0};
	static GLfloat white[4] = {1.0, 1.0, 1.0, 0.0};
	static GLfloat shininess = 5;
	glLightfv(GL_LIGHT0, GL_POSITION, pos);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
	// glLightfv( GL_LIGHT0, GL_AMBIENT, white);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white);
	glEnable(GL_CULL_FACE);

	glEnable(GL_LIGHT0);
	// glEnable(GL_DEPTH_TEST);

	glEnable(GL_POLYGON_STIPPLE);
	//	glDisable(GL_POLYGON_STIPPLE);
	glPolygonStipple(stipplepattern);
	glPointSize(10.0f);
	glTextSize(GL_TEXT_SIZE24x24);
	/* make the gears */
	gear1 = glGenLists(1);
	glNewList(gear1, GL_COMPILE);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, blue);
	glMaterialfv(GL_FRONT, GL_SPECULAR, white);
	glMaterialfv(GL_FRONT, GL_SHININESS, &shininess);
	glColor3fv(blue);
	gear(1.0, 4.0, 1.0, 20, 0.7); // The largest gear.
	glEndList();

	gear2 = glGenLists(1);
	glNewList(gear2, GL_COMPILE);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, red);
	glMaterialfv(GL_FRONT, GL_SPECULAR, white);
	glColor3fv(red);
	gear(0.5, 2.0, 2.0, 10, 0.7); // The small gear with the smaller hole, to the right.
	glEndList();

	gear3 = glGenLists(1);
	glNewList(gear3, GL_COMPILE);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, green);
	glMaterialfv(GL_FRONT, GL_SPECULAR, white);
	glColor3fv(green);
	gear(1.3, 2.0, 0.5, 10, 0.7); // The small gear above with the large hole.
	glEndList();
	// glEnable( GL_NORMALIZE );
}

int main(int argc, char** argv) {
	// initialize SDL video:
	int winSizeX = 800;
	int winSizeY = 600;
	unsigned int fps = 0;
	unsigned int flat = 0;
	unsigned int setenspec = 1;
	unsigned int dotext = 1;
	unsigned int blending = 0;
	ZBuffer* frameBuffer = NULL;
	char needsRGBAFix = 0;
	int isRunning;
	GLfloat h;
	unsigned int frames, tNow, tLastFps;
	int i;
	EFI_INPUT_KEY Key;

	EFI_STATUS Status;
	EFI_GRAPHICS_OUTPUT_PROTOCOL          *GraphicsOutput;

	Status = gBS->LocateProtocol (
								&gEfiGraphicsOutputProtocolGuid,
								NULL,
								(VOID **)&GraphicsOutput
								);
	ASSERT_EFI_ERROR(Status);
	if (EFI_ERROR(Status)) {
		return Status;
	}


	if (argc > 1) {
		char* larg = "";
		for (i = 1; i < argc; i++) {
			if (!strcmp(larg, "-w"))
				winSizeX = atoi(argv[i]);
			if (!strcmp(larg, "-h"))
				winSizeY = atoi(argv[i]);
			if (!strcmp(larg, "-fps"))
				fps = strtoull(argv[i], 0, 10);
			if (!strcmp(argv[i], "-flat"))
				flat = 1;
			if (!strcmp(argv[i], "-smooth"))
				flat = 0;
			if (!strcmp(argv[i], "-blend"))
				blending = 1;
			if (!strcmp(argv[i], "-nospecular"))
				setenspec = 0;
			if (!strcmp(argv[i], "-lines"))
				override_drawmodes = 1;
			if (!strcmp(argv[i], "-points"))
				override_drawmodes = 2;
			if (!strcmp(argv[i], "-nosdl"))
				noSDL = 1;
			if (!strcmp(argv[i], "-notext"))
				dotext = 0;
			larg = argv[i];
		}
	}

	if (TGL_FEATURE_RENDER_BITS == 32)
		frameBuffer = ZB_open(winSizeX, winSizeY, ZB_MODE_RGBA, 0);
	else
		frameBuffer = ZB_open(winSizeX, winSizeY, ZB_MODE_5R6G5B, 0);
	if (!frameBuffer) {
		printf("\nZB_open failed!");
		exit(1);
	}
	glInit(frameBuffer);

	// Print version info
	printf("\nVersion string:\n%s", glGetString(GL_VERSION));
	printf("\nVendor string:\n%s", glGetString(GL_VENDOR));
	printf("\nRenderer string:\n%s", glGetString(GL_RENDERER));
	printf("\nExtensions string:\n%s", glGetString(GL_EXTENSIONS));
	printf("\nLicense string:\n%s", glGetString(GL_LICENSE));
	// initialize GL:
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glViewport(0, 0, winSizeX, winSizeY);
	if (flat)
		glShadeModel(GL_FLAT);
	else
		glShadeModel(GL_SMOOTH);
	// TESTING BLENDING...
	// glDisable(GL_DEPTH_TEST);

	// glDisable( GL_LIGHTING );
	glEnable(GL_LIGHTING);
	// glBlendFunc(GL_ONE_MINUS_SRC_COLOR, GL_ZERO);
	glBlendEquation(GL_FUNC_ADD);
	if (blending) {
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_COLOR, GL_DST_COLOR);
		glBlendEquation(GL_FUNC_ADD);
	} else {
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);
	}
	h = (GLfloat)winSizeY / (GLfloat)winSizeX;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-1.0, 1.0, -h, h, 5.0, 60.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -45.0);

	initScene();
	if (setenspec)
		glSetEnableSpecular(GL_TRUE);
	else
		glSetEnableSpecular(GL_FALSE);
	// variables for timing:
	frames = 0;
	tNow = SDL_GetTicks();
	tLastFps = tNow;

	// main loop:
	isRunning = 1;
	// float test = 0;
	while (isRunning) {
		++frames;
		// Depending on SDL to give us ticks even without a window open...
		tNow = SDL_GetTicks();
		//	test = TEST_fastInvSqrt(tNow);
		//	printf("\n%f",test);
		// do event handling:
		Status = gST->ConIn->ReadKeyStroke(gST->ConIn, &Key);
		if (Status == EFI_SUCCESS) {
			switch (Key.ScanCode) {
			case SCAN_UP:
				view_rotx += 5.0;
				break;
			case SCAN_DOWN:
				view_rotx -= 5.0;
				break;
			case SCAN_LEFT:
				view_roty += 5.0;
				break;
			case SCAN_RIGHT:
				view_roty -= 5.0;
				break;
			case SCAN_ESC:
				isRunning = 0;
			default:
				break;
			}
		}

		// draw scene:
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		draw();
		if (dotext) {
			glDrawText((unsigned char*)"RED text", 0, 0, 0xFF0000);

			glDrawText((unsigned char*)"GREEN text", 0, 24, 0x00FF00);

			glDrawText((unsigned char*)"\xa2"
									   "BLUE text"
									   "\xa1",
					   0, 48, 0xFF);
		}
			// Quickly convert all pixels to the correct format
#if TODO
if TGL_FEATURE_RENDER_BITS == 32
		// for testing!
		if (needsRGBAFix)
			for (i = 0; i < frameBuffer->xsize * frameBuffer->ysize; i++) {
#define DATONE (frameBuffer->pbuf[i])
				DATONE = ((DATONE & 0x00FF0000)) << screen->format->Rshift | ((DATONE & 0x0000FF00) >> 8) << screen->format->Gshift |
						 ((DATONE & 0x000000FF) >> 16) << screen->format->Bshift;
			}
#endif
#if TODO
		if (!noSDL)
			ZB_copyFrameBuffer(frameBuffer, screen->pixels, screen->pitch);

		if (!noSDL)
			if (fps > 0)
				if ((1000 / fps) > (SDL_GetTicks() - tNow)) {
					SDL_Delay((1000 / fps) - (SDL_GetTicks() - tNow)); // Yay stable framerate!
				}
#endif
		GraphicsOutput->Blt(
			GraphicsOutput,
			(EFI_GRAPHICS_OUTPUT_BLT_PIXEL*)frameBuffer->pbuf,
			EfiBltBufferToVideo,
			0,
			0,
			0,
			0,
			frameBuffer->xsize,
			frameBuffer->ysize,
			0
		);

		// update fps:
		if (tNow >= tLastFps + 5000) {
			printf("%i frames in %f secs, %f frames per second.\n", frames, (float)(tNow - tLastFps) * 0.001f,
				   (float)frames * 1000.0f / (float)(tNow - tLastFps));
			tLastFps = tNow;
			frames = 0;
		}
	}
	printf("%i frames in %f secs, %f frames per second.\n", frames, (float)(tNow - tLastFps) * 0.001f, (float)frames * 1000.0f / (float)(tNow - tLastFps));
	// cleanup:
	// glDeleteList(gear1);
	// glDeleteList(gear2);
	// glDeleteList(gear3);
	ZB_close(frameBuffer);
	glClose();

	return 0;
}

EFI_STATUS
EFIAPI
EntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
 {
	char *argv[] = {"", ""};
	
	main(2, argv);
	return EFI_SUCCESS;
 }

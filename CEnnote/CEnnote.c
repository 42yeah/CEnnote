// CEnnote.c: 
// ÷‹Íª on 2018/3/16
//

#include <stdio.h>
#include <GL\glew.h>
#include <SFML\Graphics.h>
#include <SFML\Window.h>
#include <SFML\OpenGL.h>
#include "Dynamic.h"
#include "Resources.h"
#include "Paint.h"
#include <math.h>

int errCode = GL_NO_ERROR;

#define DEBUG(n) n; errCode = glGetError(); if (errCode != GL_NO_ERROR) { printf("%d: Error: %d\n", __LINE__, errCode); }

// Construct FSM
struct States {
	int mousePressed;
	float ix, iy;
	int igx, igy; // Initial Global X and Y
	int ioffx, ioffy;
	float x, y;
	float aspectRatio;
	float camOffset;
	char* fname;
	float dr;
	float dg;
	float db;
	float r;
	float g;
	float b;
	int dragMode;
} fsm;

sfWindow* ren; // RenderWindow
sfEvent e;
GLuint vao, vbo;
GLuint framevao, framevbo;
GLuint prog;
sfClock* eclock;
sfClock* ndraw;
float deltaTime;
int paintIndex;
struct Dynamic* paintSessions;
float orthoM[16] = { 0 };

void genOrthographicM(float* f, float left, float right, float down, float up);
void paintFrame(int index, float off);
void paint(int index, float off);
void updateColor();
void saveSessions();
void loadSessions();

int main()
{
	// Initialize FSM
	fsm.mousePressed = 0;
	fsm.ix = fsm.iy = fsm.igx = fsm.igy = fsm.ioffx = fsm.ioffy
		= fsm.x = fsm.y = fsm.aspectRatio = fsm.camOffset = fsm.dragMode = 0;
	fsm.fname = "ennote.data";
	fsm.r = fsm.dr = 0.925f;
	fsm.g = fsm.dg = 0.898f;
	fsm.b = fsm.db = 0.537f;

	// RenderWindow creation
	sfContextSettings settings;
	settings.antialiasingLevel = 4;
	settings.attributeFlags = sfContextCore;
	settings.depthBits = 24;
	settings.majorVersion = 3;
	settings.minorVersion = 3;
	settings.stencilBits = 8;
	
	sfVideoMode vm;
	vm.width = 500;
	vm.height = 400;

	ren = sfWindow_create(vm, "Ennote", sfNone, &settings);

	// Initialize GLEW
	glewExperimental = 1;
	if (glewInit() < 0) {
		printf("Error loading GL funcs\n");
		return -1;
	}
	glGetError(); // Clear 500

	// Preprocessing: Compile Vertex / Fragment Shader
	GLuint v, f;
	v = DEBUG(glCreateShader(GL_VERTEX_SHADER));
	const char* vsrc = enResources_loadFile("v.glsl");
	char log[512];
	DEBUG(glShaderSource(v, 1, &vsrc, NULL));
	DEBUG(glCompileShader(v));
	DEBUG(glGetShaderInfoLog(v, 512, NULL, log));
	printf("Vertex Shader: %s\n", log);
	free(vsrc);

	f = DEBUG(glCreateShader(GL_FRAGMENT_SHADER));
	const char* fsrc = enResources_loadFile("f.glsl");
	DEBUG(glShaderSource(f, 1, &fsrc, NULL));
	DEBUG(glCompileShader(f));
	DEBUG(glGetShaderInfoLog(f, 512, NULL, log));
	printf("Fragment Shader: %s\n", log);
	free(fsrc);

	prog = DEBUG(glCreateProgram());
	DEBUG(glAttachShader(prog, v));
	DEBUG(glAttachShader(prog, f));
	DEBUG(glLinkProgram(prog));
	DEBUG(glGetProgramInfoLog(prog, 512, NULL, log));
	printf("Program: %s\n", log);

	// Preprocessing: Initialize paint's vao
	DEBUG(glGenVertexArrays(1, &vao));
	DEBUG(glGenBuffers(1, &vbo));
	DEBUG(glGenVertexArrays(1, &framevao));
	DEBUG(glGenBuffers(1, &framevbo));
	DEBUG(glBindVertexArray(framevao));
	DEBUG(glBindBuffer(GL_ARRAY_BUFFER, framevbo));
	fsm.aspectRatio = (float)sfWindow_getSize(ren).x / sfWindow_getSize(ren).y;
	float frame[] = {
		-1.0f, -fsm.aspectRatio,
		1.0f, -fsm.aspectRatio,
		-1.0f, fsm.aspectRatio,
		1.0f, fsm.aspectRatio,
		-1.0f, fsm.aspectRatio,
		1.0f, -fsm.aspectRatio
	};
	DEBUG(glBufferData(GL_ARRAY_BUFFER, sizeof(frame), frame, GL_STATIC_DRAW));
	DEBUG(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, NULL));
	DEBUG(glEnableVertexAttribArray(0));

	// Preprocessing: 
	DEBUG(glClearColor(0.9f, 0.9f, 0.9f, 1.0f));
	DEBUG(glLineWidth(5.0f));
	eclock = sfClock_create();
	ndraw = sfClock_create();
	sfClock_restart(eclock);
	sfClock_restart(ndraw);
	deltaTime = 0;
	paintIndex = 0;
	paintSessions = enDynamic_create(sizeof(struct Paint));
	struct Paint* paint = enPaint_create();
	enDynamic_pushBack(paintSessions, paint);
	
	loadSessions();

	while (sfWindow_isOpen(ren)) {
		sfEvent e;

		// Preprocessing: Get mouse position and normalize it 
		sfVector2i vec = sfMouse_getPosition(ren);
		// Calculate normalized X and Y 
		float xn = (float)vec.x / sfWindow_getSize(ren).x;
		xn -= 0.5f;
		xn *= 2;
		float yn = (float)vec.y / sfWindow_getSize(ren).y;
		yn *= fsm.aspectRatio;
		yn -= 1.0f * fsm.aspectRatio / 2;
		yn *= 2;
		yn = -yn;

		fsm.x = xn;
		fsm.y = yn;

		// Preprocessing: get delta time
		deltaTime = (float)sfTime_asMilliseconds(sfClock_getElapsedTime(eclock)) / 1000;
		sfClock_restart(eclock);

		while (sfWindow_pollEvent(ren, &e)) {
			if (e.type == sfEvtClosed) {
				sfWindow_close(ren);
			}
			if (e.type == sfEvtMouseButtonPressed) {
				fsm.mousePressed = 1;
				fsm.ix = xn;
				fsm.iy = yn;
				fsm.igx = sfMouse_getPosition(NULL).x;
				fsm.igy = sfMouse_getPosition(NULL).y;
			    fsm.ioffx = sfWindow_getPosition(ren).x - fsm.igx;
				fsm.ioffy = sfWindow_getPosition(ren).y - fsm.igy;
				if (sfKeyboard_isKeyPressed(sfKeyLControl)) {
					fsm.dragMode = 1;
				}
			}
			if (e.type == sfEvtMouseButtonReleased) {
				fsm.mousePressed = 0;
				fsm.dragMode = 0;
				enPaint_addSkipPoint(enDynamic_get(paintSessions, paintIndex));
			}
			if (e.type == sfEvtKeyPressed) {
				if (sfKeyboard_isKeyPressed(sfKeyLControl) && sfKeyboard_isKeyPressed(sfKeyZ)) {
					enPaint_revert(enDynamic_get(paintSessions, paintIndex));
				}
				else if (sfKeyboard_isKeyPressed(sfKeyLControl) && sfKeyboard_isKeyPressed(sfKeyS)) {
					saveSessions();
					fsm.r = 0.596f;
					fsm.g = 0.984f;
					fsm.b = 0.596f;
				}
				else if (sfKeyboard_isKeyPressed(sfKeyRight)) {
					paintIndex++;
				}
				else if (sfKeyboard_isKeyPressed(sfKeyLeft)) {
					if (paintIndex > 0) {
						paintIndex--;
					}
				}
				else if (sfKeyboard_isKeyPressed(sfKeyR)) {
					enPaint_clear(enDynamic_get(paintSessions, paintIndex));
				}
			}
		}

		// Preprocessing: drag window around
		if (fsm.dragMode) {
			int gx = sfMouse_getPosition(NULL).x;
			int gy = sfMouse_getPosition(NULL).y;
			sfVector2i v;
			v.x = fsm.ioffx + gx;
			v.y = fsm.ioffy + gy;
			sfWindow_setPosition(ren, v);
		}

		if (fsm.mousePressed && sfTime_asMilliseconds(sfClock_getElapsedTime(ndraw)) >= 30 && !fsm.dragMode) {
			// Add verts on xn, yn
			sfClock_restart(ndraw);
			enPaint_addPoint(enDynamic_get(paintSessions, paintIndex), fsm.x, fsm.y);
		}

		// Preprocessing: update color back to dr, dg, db
		updateColor();

		DEBUG(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		// Preprocessing: Refresh VAO
		if (paintIndex >= enDynamic_getLen(paintSessions) - 1) {
			struct Paint* paint = enPaint_create();
			enDynamic_pushBack(paintSessions, paint);
		}

		if (enPaint_getSize(enDynamic_get(paintSessions, paintIndex)) > 0) {
			DEBUG(glBindVertexArray(vao));
			DEBUG(glBindBuffer(GL_ARRAY_BUFFER, vbo));
			DEBUG(glBufferData(GL_ARRAY_BUFFER, enPaint_getSize(enDynamic_get(paintSessions, paintIndex)), enPaint_getArray(enDynamic_get(paintSessions, paintIndex)), GL_STATIC_DRAW));
			DEBUG(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 2, NULL));
			DEBUG(glEnableVertexAttribArray(0));
		}

		// Preprocessing: Produce orthographic camera based on resolution
		float aspectRatio = (float)sfWindow_getSize(ren).x / sfWindow_getSize(ren).y;
		genOrthographicM(orthoM, -1.0f, 1.0f, -aspectRatio, aspectRatio);

		paintFrame(paintIndex, 0);
		void paint(int index, float off); // Redefine paint here - there's struct Paint* somewhere else
		paint(paintIndex, 0);

		sfWindow_display(ren);
	}
	
    return 0;
}

void genOrthographicM(float* f, float left, float right, float down, float up) {
	float znear = -1.0f;
	float zfar = 1.0f;

	f[0] = 2.0f / (right - left);
	f[1] = f[2] = 0;
	f[3] = -(right + left) / (right - left);

	f[4] = f[6] = 0;
	f[5] = 2.0f / (up - down);
	f[7] = -(up + down) / (up - down);

	f[8] = f[9] = 0;
	f[10] = -2 / (zfar - znear);
	f[11] = -(zfar + znear) / (zfar - znear);

	f[12] = f[13] = f[14] = 0;
	f[15] = 1.0f; // Homogeneous stuff
}

void saveSessions() {
	FILE* f = fopen(fsm.fname, "wb");
	if (f) {
		int i;
		int pcount = enDynamic_getLen(paintSessions);
		fwrite(&pcount, sizeof(int), 1, f);
		for (i = 0; i < pcount; i++) {
			enPaint_saveSession(enDynamic_get(paintSessions, i), f);
		}
		fclose(f);
	}
}

void loadSessions() {
	FILE* f = fopen(fsm.fname, "rb");
	if (f) {
		int pcount;
		fread(&pcount, sizeof(int), 1, f);
		enDynamic_clear(paintSessions);
		for (int i = 0; i < pcount; i++) {
			struct paint* p = enPaint_create();
			enPaint_loadSession(p, f);
			enDynamic_pushBack(paintSessions, p);
		}
	}
} 

void paintFrame(int index, float off) {
	DEBUG(glBindVertexArray(framevao));
	DEBUG(glUseProgram(prog));
	DEBUG(glUniformMatrix4fv(glGetUniformLocation(prog, "p"), 1, GL_FALSE, orthoM));
	DEBUG(glUniform1i(glGetUniformLocation(prog, "id"), 0));
	float r = fsm.r + (float)sin(index) * 0.1f;
	float g = fsm.g + (float)cos(index) * 0.1f;
	float b = fsm.b + (float)sin(index) * 0.1f;
	DEBUG(glUniform3f(glGetUniformLocation(prog, "frameColor"), r, g, b));
	DEBUG(glUniform2f(glGetUniformLocation(prog, "off"), off, 0));
	DEBUG(glUniform1f(glGetUniformLocation(prog, "aspectRatio"), fsm.aspectRatio));
	DEBUG(glDrawArrays(GL_TRIANGLES, 0, 6));
}

void paint(int index, float off) {
	struct Paint* paint = enDynamic_get(paintSessions, index);

	for (int i = 0; i < enPaint_getPointCount(paint) - 1; i++) {
		if (enPaint_isSkipPoint(paint, i)) {
			continue;
		}
		DEBUG(glBindVertexArray(vao));
		DEBUG(glUseProgram(prog));
		DEBUG(glUniformMatrix4fv(glGetUniformLocation(prog, "p"), 1, GL_FALSE, orthoM));
		DEBUG(glUniform1i(glGetUniformLocation(prog, "id"), 1));
		DEBUG(glUniform2f(glGetUniformLocation(prog, "off"), off, 0));
		DEBUG(glUniform1f(glGetUniformLocation(prog, "aspectRatio"), fsm.aspectRatio));
		DEBUG(glDrawArrays(GL_LINES, i, 2));
	}
} 

void updateColor() {
	float dr = (fsm.dr - fsm.r) * deltaTime * 3.0f;
	float dg = (fsm.dg - fsm.g) * deltaTime * 3.0f;
	float db = (fsm.db - fsm.b) * deltaTime * 3.0f;
	fsm.r += dr;
	fsm.g += dg;
	fsm.b += db;
}

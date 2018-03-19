#include "Paint.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <Windows.h>

struct Paint* enPaint_create() {
	struct Paint* paint = (struct Paint*) malloc(sizeof(struct Paint));
	paint->points_ = enDynamic_create(sizeof(float));
	paint->skipPoints_ = enDynamic_create(sizeof(int));
	return paint;
}

struct Pos enPaint_get(struct Paint* paint, int index) {
	if (index < 0 || index * 2 + 1 >= enDynamic_getLen(paint->points_)) {
		// We'll try to keep this stable so we'll move them out of the screen
		return enPos_create(-1, -1);
	}

	float fx, fy;
	fx = *(float*)enDynamic_get(paint->points_, index * 2);
	fy = fx = *(float*)enDynamic_get(paint->points_, index * 2 + 1);
	return enPos_create(fx, fy);
}

float* enPaint_getArray(struct Paint* paint) {
	return (float*) enDynamic_buffer(paint->points_);
}

// This one returns the REAL SIZE of the dynamic array
int enPaint_getSize(struct Paint* paint) {
	return enDynamic_getLen(paint->points_) * sizeof(float);
}

int enPaint_getPointCount(struct Paint* paint) {
	if (enDynamic_getLen(paint->points_) % 2 != 0) {
		// Err, we'll fix that...
		fprintf(stderr, "ERROR: enDynamic_getLen(paint->points_) %% 2 != 0\n");
		// MessageBox(NULL, "Ennote 发生了一个很严重的错误. 目前来说，我们只能帮你清空画板.", "ERROR", MB_ICONWARNING);
		enPaint_clear(paint);
		return -1;
	}

	return enDynamic_getLen(paint->points_) / 2;
}

void enPaint_addPoint(struct Paint* paint, float X, float Y) {
	int len;
	len = enDynamic_getLen(paint->points_);
	if (len >= 2) {
		float x1 = *(float*) enDynamic_get(paint->points_, len - 2);
		float y1 = *(float*) enDynamic_get(paint->points_, len - 1);
		if (fabs(x1 - X) <= 0.02f && fabs(y1 - Y) <= 0.02f) {
			return; // Discard 
		}
	}

	// Will this end in wild pointers? I have no idea... We'll see... 
	enDynamic_pushBack(paint->points_, &X);
	enDynamic_pushBack(paint->points_, &Y);
}

void enPaint_addSkipPoint(struct Paint* paint) {
	int* ilen = malloc(sizeof(int));
	*ilen = enPaint_getPointCount(paint) - 1;
	enDynamic_pushBack(paint->skipPoints_, ilen);
}

// Using C-styled 1 and 0 - for homework! 
int enPaint_isSkipPoint(struct Paint* paint, int p) {
	int i;
	int skipPoints_size = enDynamic_getLen(paint->skipPoints_);
	for (i = 0; i < skipPoints_size; i++) {
		if (* (int*)enDynamic_get(paint->skipPoints_, i) == p) {
			return 1;
		}
	}
	return 0;
}

void enPaint_revert(struct Paint* paint) {
	int skipPoints_size;
	int points_size;
	int p;

	skipPoints_size = enDynamic_getLen(paint->skipPoints_);
	points_size = enDynamic_getLen(paint->points_);
	// Clear everything: One skip point left
	if (skipPoints_size <= 1) {
		enDynamic_clear(paint->points_);
		enDynamic_clear(paint->skipPoints_);
		return;
	}

	p = *(int*) enDynamic_get(paint->skipPoints_, skipPoints_size - 1);
	if ((p + 1) * 2 == points_size) {
		// Erase it - by force - we'll free that later anyway 
		paint->skipPoints_->len--;
		p = *(int*) enDynamic_get(paint->skipPoints_, skipPoints_size - 2);
	}

	// Erase it again... Hope this works! 
	paint->points_->len = (p + 1) * 2;
}

void enPaint_clear(struct Paint* paint) {
	enDynamic_clear(paint->points_);
	enDynamic_clear(paint->skipPoints_);
}

void enPaint_loadSession(struct Paint* paint, FILE* stream) {
	int points;
	int skipPoints;
	fread(&points, sizeof(int), 1, stream);
	if (points > 0) {
		enDynamic_clear(paint->points_);
		for (int i = 0; i < points; i++) {
			float* f = malloc(sizeof(float));
			int len = fread(f, sizeof(float), 1, stream);
			enDynamic_pushBack(paint->points_, f);
		}
	}
	fread(&skipPoints, sizeof(int), 1, stream);
	if (skipPoints > 0) {
		enDynamic_clear(paint->skipPoints_);
		for (int i = 0; i < skipPoints; i++) {
			int* r = malloc(sizeof(int));
			fread(r, sizeof(int), 1, stream);
			enDynamic_pushBack(paint->points_, r);
		}
	}
}

void enPaint_saveSession(struct Paint* paint, FILE* stream) {
	int points = enDynamic_getLen(paint->points_);
	int skipPoints = enDynamic_getLen(paint->skipPoints_);
	fwrite(&points, sizeof(int), 1, stream);
	if (points > 0) {
		for (int i = 0; i < points; i++) {
			fwrite((float*) enDynamic_get(paint->points_, i), sizeof(float), 1, stream);
		}
	}
	fwrite(&skipPoints, sizeof(int), 1, stream);
	if (skipPoints > 0) {
		for (int i = 0; i < skipPoints; i++) {
			fwrite((float*) enDynamic_get(paint->skipPoints_, i), sizeof(int), 1, stream);
		}
	}
	fflush(stream);
}

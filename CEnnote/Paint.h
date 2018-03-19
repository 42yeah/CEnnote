#pragma once

#include <stdio.h>
#include "Dynamic.h"
#include "Pos.h"

struct Paint {
	/**
	 * AWAITS IMPLEMENTATION:
	 * 
	 */
	struct Dynamic* points_;
	struct Dynamic* skipPoints_;
};

struct Paint* enPaint_create();

struct Pos enPaint_get(struct Paint* paint, int index);

float* enPaint_getArray(struct Paint* paint);

int enPaint_getSize(struct Paint* paint);

int enPaint_getPointCount(struct Paint* paint);

void enPaint_addPoint(struct Paint* paint, float X, float Y);

void enPaint_addSkipPoint(struct Paint* paint);

int enPaint_isSkipPoint(struct Paint* paint, int p);

void enPaint_revert(struct Paint* paint);

void enPaint_clear(struct Paint* paint);

void enPaint_saveSession(struct Paint* paint, FILE* stream);

void enPaint_loadSession(struct Paint* paint, FILE* stream);

#pragma once

struct Dynamic {
	void* data;
	int size;
	int len;
};

struct Dynamic* enDynamic_create(int siz);

void enDynamic_pushBack(struct Dynamic* dyn, void* data);

void* enDynamic_get(struct Dynamic* dyn, int index);

void enDynamic_clear(struct Dynamic* dyn);

int enDynamic_getLen(struct Dynamic* dyn);

void enDynamic_printAsInt(struct Dynamic* dyn);

void* enDynamic_buffer(struct Dynamic* dyn);

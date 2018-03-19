/// Author: 201741402139 - ÷‹Íª

#include "Dynamic.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// I've been so defensive lately...

struct Dynamic* enDynamic_create(int siz) {
	struct Dynamic* dyn = malloc(sizeof(struct Dynamic));
	dyn->size = siz;
	dyn->data = malloc(0);
	dyn->len = 0;

	return dyn;
}

void enDynamic_pushBack(struct Dynamic* dyn, void* data) {
	char* nsiz;
	char* nsiznow;
	// Allocate a new buffer
	nsiz = malloc((dyn->len + 1) * dyn->size);
	// Locate the new buffer's last element 
	nsiznow = nsiz + (dyn->len) * dyn->size;
	// Copy the old buffer to the new one 
	memcpy(nsiz, dyn->data, (dyn->len) * dyn->size);
	// Copy the new one to the last element
	memcpy(nsiznow, data, dyn->size);
	dyn->len++;
	free(dyn->data);
	dyn->data = nsiz;
}

void* enDynamic_get(struct Dynamic* dyn, int index) {
	int off;
	if (index >= dyn->len || index < 0) {
		return NULL;
	}
	off = index * dyn->size;
	void* data = ((char*) (dyn->data)) + off;
	return data;
}

void enDynamic_clear(struct Dynamic* dyn) {
	free(dyn->data);
	dyn->data = malloc(0);
	dyn->len = 0;
}

int enDynamic_getLen(struct Dynamic* dyn) {
	return dyn->len;
}

// For testing purposes
void enDynamic_printAsInt(struct Dynamic* dyn) {
	int len = enDynamic_getLen(dyn);
	int i, j;

	for (i = 0; i < len; i++) {
		j = *(int*) enDynamic_get(dyn, i);
		printf("%d ", j);
	}
	printf("\n");
}

void* enDynamic_buffer(struct Dynamic* dyn) {
	return dyn->data;
}

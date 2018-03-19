#include "Resources.h"

#include <stdio.h>
#include <string.h>

char* enResources_loadFile(char* fname) {
	// Load a text-based file - shader indeed!
	FILE* file;
	char* buf;
	int rdlen;

	buf = malloc(51200);
	file = fopen(fname, "r");
	if (!file) {
		memcpy(buf, "Failed to open file", 1024);
		return buf;
	}
	// Max 51200b - 50KiB
	
	memset(buf, 0, 51200);
	rdlen = fread(buf, sizeof(char), 51200, file);
	if (rdlen <= 0) {
		// Somehow, the read failed
		memcpy(buf, "Failed to read file", 1024);
		return buf;
	}
	buf[rdlen] = 0;
	return buf;
}

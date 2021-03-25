// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#include <stdio.h>
#include <assert.h>
#include <unistd.h>
// file.c

FILE* open_file(char const* filename, char const* mode) {
	FILE* ret = fopen(filename, mode);
	assert(ret != NULL);
	return ret;
}

void close_file(FILE* file) {
	fclose(file);
}


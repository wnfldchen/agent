// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#pragma once
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include "threadpipe.h"
// readthread.h

typedef struct {
	ThreadPipe* pipe_out;
	FILE* inputFile;
	uint32_t m;
	int is_zstd;
	int is_agent;
} ReadThreadArgs;

void initializeargs_read_thread(pthread_t* thread, ReadThreadArgs* args);

void destroy_read_thread(pthread_t* thread, ReadThreadArgs* args);

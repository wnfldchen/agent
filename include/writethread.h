// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#pragma once
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include "threadpipe.h"
#include "agentformat.h"
// writethread.h

typedef struct {
	AgentHeader* agentHeader;
	ThreadPipe* pipe_in;
	FILE* outputFile;
	uint32_t m;
	int is_agent;
	int action;
} WriteThreadArgs;

void initializeargs_write_thread(pthread_t* thread, WriteThreadArgs* args);

void destroy_write_thread(pthread_t* thread, WriteThreadArgs* args);

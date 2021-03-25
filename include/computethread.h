// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#pragma once
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include "agentformat.h"
#include "threadpipe.h"
#include "phenotype.h"
// computethread.h

typedef struct {
    AgentHeader* agentHeader;
	ThreadPipe* pipe_in;
	ThreadPipe* pipe_out;
	int is_zstd;
	int is_agent;
	int action;
        char *output_dir;
        t_phenotype *phenos;
} ComputeThreadArgs;

void initializeargs_compute_thread(pthread_t* thread, ComputeThreadArgs* args);

void destroy_compute_thread(pthread_t* thread, ComputeThreadArgs* args);

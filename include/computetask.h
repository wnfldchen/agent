// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#pragma once
#include <stdint.h>
#include "bgenformat.h"
#include "agentformat.h"
// computetask.h

typedef struct {
	AgentFseHeader agentFseHeader;
	AgentProbabilityData agentProb;
	VariantIdData vid;
	ProbabilityData prob;
} ComputeTask;

void preallocate_tasks(uint32_t m);

ComputeTask* next_task();

void deallocate_tasks();

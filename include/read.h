// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#pragma once
#include <stdio.h>
#include "bgenformat.h"
#include "agentformat.h"
// read.h

// Read header block
void read_bgen_header(FILE* bgenFile, BgenHeader* header);

uint64_t read_l0(FILE* agentFile);

void read_agent_header(FILE* agentFile, AgentHeader* header);

// Read variant ID data block
void read_vid(FILE* bgenFile, VariantIdData* vid);

// Read probability data block
void read_prob(FILE *bgenFile, ProbabilityData *prob_union, int is_zstd, int is_agent);

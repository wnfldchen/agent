// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#pragma once
#include <stdio.h>
#include "agentformat.h"
// write.h

// Write header block
void write_bgen_header(FILE* bgenFile, BgenHeader bgenHeader);

void write_agent_header(FILE* agentFile, AgentHeader* agentHeader);

// Write variant ID data block
void write_vid(FILE* agentFile, AgentVariantIdData* agentVid);

void write_vid_txt(
        FILE* dosageFile,
        VariantIdData* vid);

// Write probability data block
void write_prob(FILE* agentFile, AgentFseHeader* agentFseHeader, AgentProbabilityData* agentProb, AgentHeader* agentHeader, uint32_t i);

void write_prob_txt(FILE *dosageFile, UncompressedProbabilityData *prob, int is_agent);

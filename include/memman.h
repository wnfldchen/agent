// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#pragma once
#include <stddef.h>
#include <stdio.h>
#include "bgenformat.h"
#include "agentformat.h"
// memman.h

// Create
void* create_buf(size_t size, size_t nmemb);

BgenHeader* create_bgen_header();

AgentHeader* create_agent_header(uint64_t l0);

AgentHeader* create_agent_header_bgen(BgenHeader* bgenHeader);

VariantIdData* create_vid();

ShortField* create_short_field(FILE* file);

LongField* create_long_field(FILE* file);

ProbabilityData* create_prob();

AgentProbabilityData* create_agent_prob();

AgentFseHeader* create_agent_fse_header();

// Resize

void* resize_buf(void* buf, size_t size, size_t nmemb);

// Destroy

void destroy_buf(void* buf);

void destroy_bgen_header(BgenHeader* header);

void destroy_agent_header(AgentHeader* header);

void destroy_vid(VariantIdData* vid);

void destroy_short_field(ShortField* field);

void destroy_long_field(LongField* field);

void destroy_prob(ProbabilityData* prob);

void destroy_agent_prob(AgentProbabilityData* agentProb);

void destroy_agent_fse_header(AgentFseHeader* agentFseHeader);

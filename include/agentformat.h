// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#pragma once
#include <stdint.h>
#include "bgenformat.h"
// agentformat.h

// Turn off packing for format structures
#pragma pack(push,1)

typedef LongField AgentLongField;
typedef ShortField AgentShortField;
typedef VariantIdData AgentVariantIdData;

typedef struct {
    uint64_t magic; // "agent001"
    uint64_t n;
    uint64_t m;
    uint64_t unused[4];
    uint64_t l0;
    uint64_t blocks[];
} AgentHeader;

typedef struct {
	uint32_t c;
	uint32_t d;
	uint8_t* data;
} AgentProbabilityData;

typedef struct {
	uint32_t data_decomp_sz;
	uint32_t data_comp_sz;
	//uint8_t flags; // 0 == Non-hardcall; 1 == Hardcall
} AgentFseHeader;


#pragma pack(pop)

void validate_agent_header(AgentHeader* header);
int numVariants(char *filename);
int numSamples(char *filename);

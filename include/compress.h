// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#pragma once
#include "bgenformat.h"
#include "agentformat.h"
// compress.h

void compress(
        AgentFseHeader* agentFseHeader,
        AgentProbabilityData* agentProbDataBlock,
        UncompressedProbabilityData *probDataBlock);

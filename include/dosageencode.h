#pragma once
#include "bgenformat.h"
#include "agentformat.h"
// dosageencode.h

void dosage_encode(
        AgentFseHeader* agentFseHeader,
        AgentProbabilityData* agentProbDataBlock,
        UncompressedProbabilityData *probDataBlock);

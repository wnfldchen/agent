#pragma once
#include "agentformat.h"
#include "computetask.h"
// decompress.h

void zstd_decompress(ComputeTask *task);

void agent_decompress(ComputeTask* task, AgentHeader* agentHeader);

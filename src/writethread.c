// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#define _GNU_SOURCE
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include "options.h"
#include "threadpipe.h"
#include "computetask.h"
#include "writethread.h"
#include "write.h"
#include "memman.h"
#include "bgenformat.h"
// writethread.c

void* write_thread_start_routine(void* arg) {
	WriteThreadArgs* args = arg;
	AgentHeader* agentHeader = args->agentHeader;
	ThreadPipe* pipe_in = args->pipe_in;
	FILE* outputFile = args->outputFile;
	uint32_t m = args->m;
	int is_agent = args->is_agent;
	int action = args->action;
	assert(action == ACT_BGA1 || action == ACT_DOSE || action == ACT_GWA1);
	ComputeTask** tasks = create_buf(sizeof(tasks[0]), m);

    uint32_t batch_size = 500; // REMARK: Must be the same as other batch_size variables
    size_t* variants_batch = create_buf(sizeof(variants_batch[0]), batch_size);
    ComputeTask** tasks_batch = create_buf(sizeof(tasks_batch[0]), batch_size);

	uint32_t written = 0;
	while (written < m) {
	    remove_threadpipe(pipe_in, variants_batch, (void**) tasks_batch, batch_size);
	    for (uint32_t i = 0; i < batch_size && tasks_batch[i] != NULL; i++) {
	        tasks[variants_batch[i]] = tasks_batch[i];
	    }
		for ( ;
				written < m && tasks[written] != NULL;
				written += 1) {
			ComputeTask* task = tasks[written];
			switch (action) {
                case ACT_BGA1:
                    write_vid(outputFile, &task->vid);
                    write_prob(
                            outputFile,
                            &task->agentFseHeader,
                            &task->agentProb,
                            agentHeader,
                            written);
                    break;
                case ACT_DOSE:
                    write_vid_txt(outputFile, &task->vid);
                    write_prob_txt(outputFile, &task->prob.uncompressed, is_agent);
                    break;
                case ACT_GWA1: {
                    VariantIdData *vid = &task->vid;
                    UncompressedProbabilityData *prob = &task->prob.uncompressed;
                    destroy_short_field(vid->id);
                    destroy_short_field(vid->rsid);
                    destroy_short_field(vid->chr);
                    for (uint16_t i = 0; i < vid->k; i += 1) {
                        destroy_long_field(vid->alleles[i]);
                    }
                    destroy_buf(vid->alleles);
                    destroy_buf(prob->ploidy);
                    destroy_buf(prob->data);
                    break;
                }
                default:
                    assert(0);
                    break;
			}
		}
	}

	destroy_buf(variants_batch);
	destroy_buf(tasks_batch);

	destroy_buf(tasks);
	return NULL;
}

void initializeargs_write_thread(pthread_t *thread, WriteThreadArgs *args) {
    pthread_create(
            thread,
            NULL,
            write_thread_start_routine,
            args);

#ifdef __APPLE__
#else
    pthread_setname_np(*thread, "agent.write");
#endif
}

void destroy_write_thread(pthread_t* thread, WriteThreadArgs* args) {
	pthread_join(*thread, NULL);
}

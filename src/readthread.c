// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#define _GNU_SOURCE
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include <memman.h>
#include "threadpipe.h"
#include "computetask.h"
#include "readthread.h"
#include "read.h"
// readthread.c

void* read_thread_start_routine(void* arg) {
	ReadThreadArgs* args = arg;
	ThreadPipe* pipe_out = args->pipe_out;
	FILE* inputFile = args->inputFile;
	uint32_t m = args->m;
	int is_zstd = args->is_zstd;
	int is_agent = args->is_agent;
	uint32_t batch_size = 500; // REMARK: Must be the same as other batch_size variables
    uint32_t buffer_factor = 2;
	uint32_t variant = 0;
	if (m >= batch_size) {
	    size_t* variants = create_buf(sizeof(variants[0]), batch_size);
	    ComputeTask** tasks = create_buf(sizeof(tasks[0]), batch_size);
        for (
                ;
                variant <= m - batch_size;
                variant += batch_size) {
            for (uint32_t i = 0; i < batch_size; i++) {
                variants[i] = variant + i;
                tasks[i] = next_task();
                read_vid(inputFile, &tasks[i]->vid);
                read_prob(inputFile, &tasks[i]->prob, is_zstd, is_agent);
            }
            /* REMARK
             * Since *task is a known type
             * we don't need to track sizeof(*task).
             * We repurpose the size parameter to id tasks.
             */
            wait_threadpipe(pipe_out, buffer_factor * batch_size);
            append_threadpipe(pipe_out, variants, (void**)tasks, batch_size);
        }
        destroy_buf(variants);
        destroy_buf(tasks);
    }
    for (
            ;
            variant < m;
            variant += 1) {
        ComputeTask *task = next_task();
        read_vid(inputFile, &task->vid);
        read_prob(inputFile, &task->prob, is_zstd, is_agent);
        /* REMARK
         * Since *task is a known type
         * we don't need to track sizeof(*task).
         * We repurpose the size parameter to id tasks.
         */
        push_threadpipe(pipe_out, variant, task);
    }
	close_threadpipe(pipe_out);
	return NULL;
}

void initializeargs_read_thread(pthread_t *thread, ReadThreadArgs *args) {
    preallocate_tasks(args->m);
    pthread_create(
            thread,
            NULL,
            read_thread_start_routine,
            args);

#ifdef __APPLE__
#else
    pthread_setname_np(*thread, "agent.read");
#endif
}

void destroy_read_thread(pthread_t* thread, ReadThreadArgs* args) {
	pthread_join(*thread, NULL);
	deallocate_tasks();
}

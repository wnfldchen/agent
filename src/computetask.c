// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#include <stdint.h>
#include "computetask.h"
#include "memman.h"
// computetask.c

static ComputeTask* tasks;
static uint32_t tid;

void preallocate_tasks(uint32_t m) {
	tasks = create_buf(sizeof(tasks[0]), m);
	tid = 0;
}

ComputeTask* next_task() {
	return &tasks[tid++];
}

void deallocate_tasks() {
	destroy_buf(tasks);
}

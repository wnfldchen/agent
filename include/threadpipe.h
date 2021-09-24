// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#pragma once
#include <stddef.h>
#include <pthread.h>
#include "linkedlist.h"
// threadpipe.h

enum {
	THREADPIPE_OPEN,
	THREADPIPE_CLOSED
};

enum {
	THREADPIPE_OP_SUCCESS,
	THREADPIPE_OP_CLOSED
};

typedef struct {
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	LinkedList list;
	int state;
} ThreadPipe;

void initialize_threadpipe(ThreadPipe* pipe);

void close_threadpipe(ThreadPipe* pipe);

void destroy_threadpipe(ThreadPipe* pipe);

int wait_threadpipe(ThreadPipe* pipe, size_t n);

int push_threadpipe(ThreadPipe* pipe, size_t size, void* data);

int append_threadpipe(ThreadPipe* pipe, size_t* sizes, void** datas, size_t n);

int remove_threadpipe(ThreadPipe* pipe, size_t* sizes, void** datas, size_t n);

int pop_threadpipe(ThreadPipe* pipe, size_t* size, void** data);

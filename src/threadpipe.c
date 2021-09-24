// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#include <stddef.h>
#include <pthread.h>
#include <stdint.h>
#include "linkedlist.h"
#include "threadpipe.h"
// threadpipe.c

void initialize_threadpipe(ThreadPipe* pipe) {
	initialize_linked_list(&pipe->list);
	pthread_mutex_init(&pipe->mutex, NULL);
	pthread_cond_init(&pipe->cond, NULL);
	pipe->state = THREADPIPE_OPEN;
}

void close_threadpipe(ThreadPipe* pipe) {
	pthread_mutex_lock(&pipe->mutex);
	pipe->state = THREADPIPE_CLOSED;
	pthread_cond_broadcast(&pipe->cond);
	pthread_mutex_unlock(&pipe->mutex);
}

void destroy_threadpipe(ThreadPipe* pipe) {
	pthread_mutex_destroy(&pipe->mutex);
	pthread_cond_destroy(&pipe->cond);
}

int wait_threadpipe(ThreadPipe* pipe, size_t n) {
    pthread_mutex_lock(&pipe->mutex);
    while (
            total_sizeof_linked_list(&pipe->list) > n
            && pipe->state == THREADPIPE_OPEN) {
        pthread_cond_wait(&pipe->cond, &pipe->mutex);
    }
    int state = pipe->state;
    pthread_mutex_unlock(&pipe->mutex);
    if (state == THREADPIPE_CLOSED) {
        return THREADPIPE_OP_CLOSED;
    }
    return THREADPIPE_OP_SUCCESS;
}

int push_threadpipe(ThreadPipe* pipe, size_t size, void* data) {
	Node* node = create_node(NULL, size, data);
	pthread_mutex_lock(&pipe->mutex);
	if (pipe->state == THREADPIPE_CLOSED) {
		pthread_mutex_unlock(&pipe->mutex);
		return THREADPIPE_OP_CLOSED;
	}
	pushnode_linked_list(&pipe->list, node);
	pthread_cond_signal(&pipe->cond);
	pthread_mutex_unlock(&pipe->mutex);
	return THREADPIPE_OP_SUCCESS;
}

int append_threadpipe(ThreadPipe *pipe, size_t *sizes, void **datas, size_t n) {
	pthread_mutex_lock(&pipe->mutex);
	if (pipe->state == THREADPIPE_CLOSED) {
		pthread_mutex_unlock(&pipe->mutex);
		return THREADPIPE_OP_CLOSED;
	}
	for (size_t i = 0; i < n; i++) {
		Node* node = create_node(NULL, sizes[i], datas[i]);
		pushnode_linked_list(&pipe->list, node);
	}
	pthread_cond_signal(&pipe->cond);
	pthread_mutex_unlock(&pipe->mutex);
	return THREADPIPE_OP_SUCCESS;
}

int remove_threadpipe(ThreadPipe *pipe, size_t *sizes, void **datas, size_t n) {
	Node* node;
	pthread_mutex_lock(&pipe->mutex);
	while (
			isempty_linked_list(&pipe->list)
			&& pipe->state == THREADPIPE_OPEN) {
		pthread_cond_wait(&pipe->cond, &pipe->mutex);
	}
	if (
			isempty_linked_list(&pipe->list)
			&& pipe->state == THREADPIPE_CLOSED) {
		pthread_mutex_unlock(&pipe->mutex);
		return THREADPIPE_OP_CLOSED;
	}
	size_t i = 0;
	for (
			;
			i < n && !isempty_linked_list(&pipe->list);
			i++) {
		popnode_linked_list(&pipe->list, &node);
		sizes[i] = node->size;
		datas[i] = node->data;
		destroy_node(node);
	}
	pthread_mutex_unlock(&pipe->mutex);
	for (; i < n; i++) {
		datas[i] = NULL;
	}
	return THREADPIPE_OP_SUCCESS;
}

int pop_threadpipe(ThreadPipe* pipe, size_t* size, void** data) {
	Node* node;
	pthread_mutex_lock(&pipe->mutex);
	while (
			isempty_linked_list(&pipe->list)
			&& pipe->state == THREADPIPE_OPEN) {
		pthread_cond_wait(&pipe->cond, &pipe->mutex);
	}
	if (
			isempty_linked_list(&pipe->list)
			&& pipe->state == THREADPIPE_CLOSED) {
		pthread_mutex_unlock(&pipe->mutex);
		return THREADPIPE_OP_CLOSED;
	}
	popnode_linked_list(&pipe->list, &node);
	pthread_mutex_unlock(&pipe->mutex);
	*size = node->size;
	*data = node->data;
	destroy_node(node);
	return THREADPIPE_OP_SUCCESS;
}

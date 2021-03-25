// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include "linkedlist.h"
// linkedlist.c

Node* create_node(Node* next, size_t size, void* data) {
	Node* ret = malloc(sizeof(*ret));
	assert(ret != NULL);
	*ret = (Node){
		.next = next,
		.size = size,
		.data = data
	};
	return ret;
}

void destroy_node(Node* node) {
	free(node);
}

size_t total_sizeof_linked_list(LinkedList* list) {
	return list->total_size;
}

int isempty_linked_list(LinkedList* list) {
	return list->head == NULL;
}

void initialize_linked_list(LinkedList* list) {
	*list = (LinkedList){
		.head = NULL,
		.tail = NULL,
		.total_size = 0
	};
}

void push_linked_list(LinkedList* list, size_t size, void* data) {
	Node* node = create_node(NULL, size, data);
	pushnode_linked_list(list, node);
}

void pop_linked_list(LinkedList* list, size_t* size, void** data) {
	Node* node;
	popnode_linked_list(list, &node);
	*size = node->size;
	*data = node->data;
	destroy_node(node);
}

void pushnode_linked_list(LinkedList* list, Node* node) {
	list->total_size += 1;
	if (list->head == NULL) {
		list->head = node;
	}
	if (list->tail != NULL) {
		list->tail->next = node;
	}
	list->tail = node;
}

void popnode_linked_list(LinkedList* list, Node** node) {
	Node* head = list->head;
	list->head = head->next;
	if (list->tail == head) {
		list->tail = NULL;
	}
	list->total_size -= 1;
	*node = head;
}

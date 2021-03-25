// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#pragma once
#include <stddef.h>
// linkedlist.h

typedef struct Node {
	struct Node* next;
	size_t size;
	void* data;
} Node;

typedef struct {
	Node* head;
	Node* tail;
	size_t total_size;
} LinkedList;

Node* create_node(Node* next, size_t size, void* data);

void destroy_node(Node* node);

void initialize_linked_list(LinkedList* list);

size_t total_sizeof_linked_list(LinkedList* list);

int isempty_linked_list(LinkedList* list);

void push_linked_list(LinkedList* list, size_t size, void* data);

void pop_linked_list(LinkedList* list, size_t* size, void** data);

void pushnode_linked_list(LinkedList* list, Node* node);

void popnode_linked_list(LinkedList* list, Node** node);

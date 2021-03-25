// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "bgenformat.h"
#include "agentformat.h"
// memman.c

// Create
void* create_buf(size_t size, size_t nmemb) {
	/* REMARK
	 * Do not change from calloc to malloc without
	 * also adding a memset to zero.
	 */
	void* ret = calloc(nmemb, size);
	assert(ret != NULL);
	return ret;
}

BgenHeader* create_bgen_header() {
	BgenHeader* ret = malloc(sizeof(*ret));
	assert(ret != NULL);
	return ret;
}

AgentHeader* create_agent_header(uint64_t l0) {
	AgentHeader* ret = malloc(l0);
	assert(ret != NULL);
	return ret;
}

AgentHeader *create_agent_header_bgen(BgenHeader *bgenHeader) {
	uint64_t m = bgenHeader->m;
	AgentHeader* ret = malloc(sizeof(*ret) + sizeof(ret->blocks[0]) * m);
	*ret = (AgentHeader){
		.magic = UINT64_C(0x313030746e656761), // "agent001"
		.n = bgenHeader->n,
		.m = m,
		.unused[0] = 0, .unused[1] = 0, .unused[2] = 0, .unused[3] = 0,
		.l0 = sizeof(*ret) + sizeof(ret->blocks[0]) * m
	};
	assert(ret != NULL);
	return ret;
}

VariantIdData* create_vid() {
	VariantIdData* ret = malloc(sizeof(*ret));
	assert(ret != NULL);
	return ret;
}

ShortField* create_short_field(FILE* file) {
	uint16_t len;
	fread(&len, sizeof(len), 1, file);
	ShortField* ret = malloc(sizeof(*ret) + len);
	assert(ret != NULL);
	ret->length = len;
	fread(&ret->data, len, 1, file);
	return ret;
}

LongField* create_long_field(FILE* file) {
	uint32_t len;
	fread(&len, sizeof(len), 1, file);
	LongField* ret = malloc(sizeof(*ret) + len);
	assert(ret != NULL);
	ret->length = len;
	fread(&ret->data, len, 1, file);
	return ret;
}

ProbabilityData* create_prob() {
	ProbabilityData* ret = malloc(sizeof(*ret));
	assert(ret != NULL);
	return ret;
}

AgentProbabilityData* create_agent_prob() {
	AgentProbabilityData* ret = malloc(sizeof(*ret));
	assert(ret != NULL);
	return ret;
}

AgentFseHeader* create_agent_fse_header() {
	AgentFseHeader* ret = malloc(sizeof(*ret));
	assert(ret != NULL);
	return ret;
}

// Resize

void* resize_buf(void* buf, size_t size, size_t nmemb) {
	void* ret = realloc(buf, nmemb * size);
	assert(ret != NULL);
	return ret;
}

// Destroy

void destroy_buf(void* buf) {
	free(buf);
}

void destroy_bgen_header(BgenHeader* header) {
	free(header);
}

void destroy_agent_header(AgentHeader *header) {
	free(header);
}

void destroy_vid(VariantIdData* vid) {
	free(vid);
}

void destroy_short_field(ShortField* field) {
	free(field);
}

void destroy_long_field(LongField* field) {
	free(field);
}

void destroy_prob(ProbabilityData* prob) {
	free(prob);
}

void destroy_agent_prob(AgentProbabilityData* agentProb) {
	free(agentProb);
}

void destroy_agent_fse_header(AgentFseHeader* agentFseHeader) {
	free(agentFseHeader);
}

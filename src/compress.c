// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "bgenformat.h"
#include "agentformat.h"
#include "memman.h"
#include "fse.h"
// compress.c

uint32_t buf_compress(
		uint8_t* dst, uint32_t dst_cap,
		uint8_t const* src, uint32_t src_sz) {
	size_t status = FSE_compress(
			dst, dst_cap,
			src, src_sz);
	assert(!FSE_isError(status));
	uint32_t ret = status;
	if (ret == 1) { // RLE
		dst[0] = src[0];
	}
	return ret;
}

/* If malloc of dst is wanted:
 * *dst == NULL
 */
void buf_resize_comp(
		uint8_t** dst, uint32_t* dst_sz,
		uint8_t const* src, uint32_t src_sz) {
	uint32_t comp_sz = FSE_compressBound(src_sz);
	uint8_t* buf_bnd = resize_buf(*dst, comp_sz, 1);
	comp_sz = buf_compress(
			buf_bnd, comp_sz,
			src, src_sz);
	uint8_t* buf_trim;
	if (comp_sz == 0) { // Not compressible
		buf_trim = resize_buf(buf_bnd, src_sz, 1);
		memcpy(buf_trim, src, src_sz);
	} else {
		buf_trim = resize_buf(buf_bnd, comp_sz, 1);
	}
	*dst = buf_trim;
	*dst_sz = comp_sz;
}

void compress(
        AgentFseHeader* agentFseHeader,
        AgentProbabilityData* agentProbDataBlock,
        UncompressedProbabilityData *probDataBlock) {
	*agentFseHeader = (AgentFseHeader){
		.data_decomp_sz =
			probDataBlock->c
			- probDataBlock->n
			- 10,
		.data_comp_sz = 0,
		//.flags = 0 // For now, force non-harcall encoding (non-prepacked)
	};
	*agentProbDataBlock = (AgentProbabilityData){
		.c = 0,
		.d = probDataBlock->c,
		.data = NULL
	};
	buf_resize_comp(
			&agentProbDataBlock->data,
			&agentFseHeader->data_comp_sz,
			probDataBlock->data,
			agentFseHeader->data_decomp_sz);
	agentProbDataBlock->c =
		(agentFseHeader->data_comp_sz != 0 ?
		   agentFseHeader->data_comp_sz : agentFseHeader->data_decomp_sz)
		+ 4 + sizeof(*agentFseHeader);
	destroy_buf(probDataBlock->ploidy);
	destroy_buf(probDataBlock->data);
}

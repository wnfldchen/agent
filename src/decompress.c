#include <stddef.h>
#include <string.h>
#include <assert.h>
#include "computetask.h"
#include "bgenformat.h"
#include "memman.h"
#include "zstd.h"
#include "fse.h"
// decompress.c

void zstd_decompress(ComputeTask *task) {
    CompressedProbabilityData* prob_compressed = &task->prob.compressed;
    uint32_t c = prob_compressed->c;
    uint32_t d = prob_compressed->d;
    uint8_t* opaque = prob_compressed->opaque;
    uint8_t* dst = create_buf(sizeof(dst[0]), d);
    size_t sz = ZSTD_decompress(dst, d, opaque, c-4);
    assert(sz == d);
    destroy_buf(opaque);
    UncompressedProbabilityData* prob_uncompressed = &task->prob.uncompressed;
    prob_uncompressed->c = d;
    size_t sz_copied =
            sizeof(prob_uncompressed->n)
            + sizeof(prob_uncompressed->k)
            + sizeof(prob_uncompressed->pmin)
            + sizeof(prob_uncompressed->pmax);
    memcpy(
            &prob_uncompressed->n,
            dst,
            sz_copied);
    prob_uncompressed->ploidy = create_buf(
            sizeof(prob_uncompressed->ploidy[0]),
            prob_uncompressed->n);
    memcpy(
            prob_uncompressed->ploidy,
            dst + sz_copied,
            prob_uncompressed->n);
    sz_copied += prob_uncompressed->n;
    memcpy(
            &prob_uncompressed->phased,
            dst + sz_copied,
            sizeof(prob_uncompressed->phased)
            + sizeof(prob_uncompressed->b));
    sz_copied +=
            sizeof(prob_uncompressed->phased)
            + sizeof(prob_uncompressed->b);
    prob_uncompressed->data = create_buf(
            sizeof(prob_uncompressed->data[0]),
            d - sz_copied);
    memcpy(
            prob_uncompressed->data,
            dst + sz_copied,
            d - sz_copied);
    destroy_buf(dst);
}

void agent_decompress(ComputeTask* task, AgentHeader* agentHeader) {
    CompressedProbabilityData* prob_compressed = &task->prob.compressed;
    uint32_t c = prob_compressed->c;
    uint32_t d = prob_compressed->d;
    uint8_t* opaque = prob_compressed->opaque;
    AgentFseHeader agentFseHeader;
    UncompressedProbabilityData* prob_uncompressed = &task->prob.uncompressed;
    prob_uncompressed->c = d;
    size_t sz_copied = sizeof(agentFseHeader);
    memcpy(&agentFseHeader, opaque, sz_copied);
    prob_uncompressed->n = agentHeader->n;
    prob_uncompressed->k = 2;
    prob_uncompressed->pmin = 2;
    prob_uncompressed->pmax = 2;
    prob_uncompressed->ploidy = create_buf(
            sizeof(prob_uncompressed->ploidy[0]),
            prob_uncompressed->n);
    memset(
            prob_uncompressed->ploidy,
            0x02,
            prob_uncompressed->n);
    prob_uncompressed->phased = 0;
    prob_uncompressed->b = 16;
    prob_uncompressed->data = create_buf(
            sizeof(prob_uncompressed->data[0]),
            agentFseHeader.data_decomp_sz);
    if (agentFseHeader.data_comp_sz != 0) {
        size_t sz = FSE_decompress(
                prob_uncompressed->data,
                agentFseHeader.data_decomp_sz,
                opaque + sz_copied,
                agentFseHeader.data_comp_sz);
        assert(sz == agentFseHeader.data_decomp_sz);
        sz_copied += agentFseHeader.data_comp_sz;
    } else {
        memcpy(
                prob_uncompressed->data,
                opaque + sz_copied,
                agentFseHeader.data_decomp_sz);
        sz_copied += agentFseHeader.data_decomp_sz;
    }
    assert(sz_copied == c-4);
    destroy_buf(opaque);
}

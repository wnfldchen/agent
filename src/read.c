// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#include <stdio.h>
#include "bgenformat.h"
#include "memman.h"
// read.c

void read_bgen_header(FILE* bgenFile, BgenHeader* header) {
	fread(header, sizeof(*header), 1, bgenFile);
}

uint64_t read_l0(FILE *agentFile) {
    // Read L0 from agent header at byte 56
    fseek(agentFile, 56, SEEK_SET);
    uint64_t l0;
    fread(&l0, sizeof(l0), 1, agentFile);
    fseek(agentFile, 0, SEEK_SET);
    return l0;
}

void read_agent_header(FILE *agentFile, AgentHeader *header) {
    // Read the fixed-size fields (64 bytes)
    fread(header, sizeof(*header), 1, agentFile);
    // Read the block offsets (8*M bytes)
    uint64_t m = header->m;
    fread(&header->blocks, sizeof(header->blocks[0]) * m, 1, agentFile);
}

void read_vid(FILE* bgenFile, VariantIdData* vid) {
	vid->id = create_short_field(bgenFile);
	vid->rsid = create_short_field(bgenFile);
	vid->chr = create_short_field(bgenFile);
	fread(
			&vid->pos,
			sizeof(vid->pos) + sizeof(vid->k),
			1,
			bgenFile);
	vid->alleles = create_buf(sizeof(vid->alleles[0]), vid->k);
	for (uint16_t i = 0; i < vid->k; i += 1) {
		vid->alleles[i] = create_long_field(bgenFile);
	}
}

void read_uncompressed_prob(
        FILE *bgenFile,
        UncompressedProbabilityData *prob) {
    fread(
            prob,
            sizeof(prob->c)
            + sizeof(prob->n)
            + sizeof(prob->k)
            + sizeof(prob->pmin)
            + sizeof(prob->pmax),
            1,
            bgenFile);
    prob->ploidy = create_buf(sizeof(prob->ploidy[0]), prob->n);
    fread(
            prob->ploidy,
            prob->n,
            1,
            bgenFile);
    fread(
            &prob->phased,
            sizeof(prob->phased) + sizeof(prob->b),
            1,
            bgenFile);
    uint32_t bytesLeft =
            prob->c
            - sizeof(prob->n)
            - sizeof(prob->k)
            - sizeof(prob->pmin)
            - sizeof(prob->pmax)
            - prob->n
            - sizeof(prob->phased)
            - sizeof(prob->b);
    prob->data = create_buf(sizeof(prob->data[0]), bytesLeft);
    fread(
            prob->data,
            bytesLeft,
            1,
            bgenFile);
}

void read_compressed_prob(
        FILE *bgenFile,
        CompressedProbabilityData *prob) {
    fread(
            prob,
            sizeof(prob->c)
            + sizeof(prob->d),
            1,
            bgenFile);
    prob->opaque = create_buf(sizeof(prob->opaque[0]), prob->c-4);
    fread(
            prob->opaque,
            prob->c-4,
            1,
            bgenFile);
}

void read_prob(FILE *bgenFile, ProbabilityData *prob_union, int is_zstd, int is_agent) {
    if (is_zstd || is_agent) {
        read_compressed_prob(bgenFile, &prob_union->compressed);
    } else {
        read_uncompressed_prob(bgenFile, &prob_union->uncompressed);
    }
}

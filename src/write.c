// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#include <stdio.h>
#include "agentformat.h"
#include "memman.h"
// write.c

void write_bgen_header(
        FILE* bgenFile,
        BgenHeader bgenHeader) {
	fwrite(
            &bgenHeader,
            sizeof(bgenHeader),
            1,
            bgenFile);
}

void write_agent_header(FILE *agentFile, AgentHeader *agentHeader) {
    rewind(agentFile);
    fwrite(
            agentHeader,
            agentHeader->l0,
            1,
            agentFile);
}

void write_short_field(
		FILE* agentFile,
		AgentShortField* field) {
	fwrite(
			field,
			sizeof(*field) + field->length,
			1,
			agentFile);
	destroy_short_field(field);
}

void write_short_field_txt(
        FILE* dosageFile,
        ShortField* field) {
    fprintf(dosageFile, "%.*s\n", field->length, field->data);
    destroy_short_field(field);
}

void write_long_field(
		FILE* agentFile,
		AgentLongField* field) {
	fwrite(
			field,
			sizeof(*field) + field->length,
			1,
			agentFile);
	destroy_long_field(field);
}

void write_long_field_txt(
        FILE* dosageFile,
        LongField* field) {
    fprintf(dosageFile, "%.*s\n", field->length, field->data);
    destroy_long_field(field);
}

void write_vid(
		FILE* agentFile,
		AgentVariantIdData* agentVid) {
	write_short_field(agentFile, agentVid->id);
	write_short_field(agentFile, agentVid->rsid);
	write_short_field(agentFile, agentVid->chr);
	fwrite(
			&agentVid->pos,
			sizeof(agentVid->pos) + sizeof(agentVid->k),
			1,
			agentFile);
	for (uint16_t i = 0; i < agentVid->k; i += 1) {
		write_long_field(agentFile, agentVid->alleles[i]);
	}
	destroy_buf(agentVid->alleles);
}

void write_vid_txt(FILE *dosageFile, VariantIdData *vid) {
    write_short_field_txt(dosageFile, vid->id);
    write_short_field_txt(dosageFile, vid->rsid);
    write_short_field_txt(dosageFile, vid->chr);
    fprintf(dosageFile, "%d\n%d\n", vid->pos, vid->k);
    for (uint16_t i = 0; i < vid->k; i += 1) {
        write_long_field_txt(dosageFile, vid->alleles[i]);
    }
    destroy_buf(vid->alleles);
}

void write_prob(
		FILE* agentFile,
		AgentFseHeader* agentFseHeader,
		AgentProbabilityData* agentProb,
        AgentHeader* agentHeader,
        uint32_t i) {
	fwrite(
			agentProb,
			sizeof(agentProb->c) + sizeof(agentProb->d),
			1,
			agentFile);
	fwrite(
			agentFseHeader,
			sizeof(*agentFseHeader),
			1,
			agentFile);
	fwrite(
			agentProb->data,
			(agentFseHeader->data_comp_sz != 0 ?
			 agentFseHeader->data_comp_sz : agentFseHeader->data_decomp_sz),
			1,
			agentFile);
    long fpos = ftell(agentFile);
    agentHeader->blocks[i] = fpos;
	destroy_buf(agentProb->data);
}

void write_prob_txt(FILE *dosageFile, UncompressedProbabilityData *prob, int is_agent) {
    uint32_t n = prob->n;
    fprintf(dosageFile, "%d\n%d\n%d\n",
            n, prob->pmin, prob->pmax);
    for (uint32_t i = 0; i < n; i++) {
        uint8_t ploidy = prob->ploidy[i];
        uint8_t missing = ploidy >> 7;
        ploidy = ploidy << 1 >> 1;
        fprintf(dosageFile, "%d %d\n",
                missing, ploidy);
    }
    fprintf(dosageFile, "%d\n%d\n",
            prob->phased, prob->b);
    for (uint32_t i = 0; i < n; i++) {
        uint8_t ploidy = prob->ploidy[i];
        uint8_t missing = ploidy >> 7;
        if (missing) {
            fputs("M\n", dosageFile);
        } else {
            uint16_t *p = (uint16_t *) prob->data;
            if (!is_agent) {
                fprintf(dosageFile, "%d %d\n", p[2 * i], p[2 * i + 1]);
            } else {
                fprintf(dosageFile, "%d\n", p[i]);
            }
        }
    }
    fputs("\n", dosageFile);
    destroy_buf(prob->ploidy);
    destroy_buf(prob->data);
}

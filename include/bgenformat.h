// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#pragma once
#include <stdint.h>
// bgenformat.h

// Turn off packing for format structures
#pragma pack(push,1)

typedef struct {
	uint32_t offset;
	uint32_t lh;
	uint32_t m;
	uint32_t n;
	uint32_t magic;
	uint32_t flags;
} BgenHeader;

typedef struct {
	uint32_t length;
	uint8_t data[];
} LongField;

typedef struct {
	uint16_t length;
	uint8_t data[];
} ShortField;

typedef struct {
	ShortField* id;
	ShortField* rsid;
	ShortField* chr;
	uint32_t pos;
	uint16_t k;
	LongField** alleles;
} VariantIdData;

typedef struct {
    uint32_t c;
    uint32_t n;
    uint16_t k;
    uint8_t pmin;
    uint8_t pmax;
    uint8_t* ploidy;
    uint8_t phased;
    uint8_t b;
    uint8_t* data;
} UncompressedProbabilityData;

typedef struct {
    uint32_t c;
    uint32_t d;
    uint8_t* opaque;
} CompressedProbabilityData;

typedef union {
	UncompressedProbabilityData uncompressed;
	CompressedProbabilityData compressed;
} ProbabilityData;

#pragma pack(pop)

void validate_bgen_header(BgenHeader* header);

int is_zstd(BgenHeader* header);

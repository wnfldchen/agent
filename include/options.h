// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#pragma once
#include <stdint.h>
#include <stdio.h>
// options.h

void read_options(
                int argc,
                char** argv,
                int *action,
                char** inputBgenFile,
                char** inputGenFile,
                char** inputA1File,
                char** outputA1File,
                char** inputPhenotypeFile,
                char** outputGwasDirectory,
                char** outputDosageFile,
                uint16_t* computeThreads);

// Convert bgen to a1
#define ACT_BGA1 1

// GWAS on a1 file
#define ACT_GWA1 2

// GWAS on gen file (for debugging)
#define ACT_GWGE 3

// GWAS on bgen file (for debugging)
#define ACT_GWBG 5

// Output dosages for a SNP (for debugging)
#define ACT_DOSE 6

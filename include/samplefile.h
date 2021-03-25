// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#ifndef __SAMPLE_FILE_
#  define __SAMPLE_FILE_

#include <stdint.h>

typedef struct {
  uint32_t ns;
  char *line1;
  char *line2;
  char **ids;
  char **lines;
} SampleFile;

typedef struct {
  uint32_t nl;
  char **lines;
} StringFile;

// read_samples
void read_samples(char *fname, SampleFile **sampleFile);

// write_samples
void write_samples(char *fname, SampleFile **sampleFile);

// destroy_samples
void destroy_samples(SampleFile **sampleFile);

// read_strings
void read_strings(char *fname, StringFile **stringFile);

// destroy_strings
void destroy_strings(StringFile **stringFile);

#endif

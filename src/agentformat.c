#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "agentformat.h"
#include "file.h"
#include "errors.h"
// agentformat.c

void validate_agent_header(AgentHeader* header) {
    assert(header->magic == UINT64_C(0x313030746e656761)); // "agent001"
}

int numSamples(char *filename) {
  if (!ends_with(filename, ".a1")) {
    error("Currently, numVariants must be called on a file in .a1 format");
  }
  FILE *fp;
  if ((fp = fopen(filename, "r")) == NULL) {
    error("Could not open genotype file");
  }
  if (fseek(fp, 8, SEEK_SET) != 0) {
    error("Could not seek on genotype file");
  }
  uint64_t n = 0;
  if (fread(&n, 8, 1, fp) != 1) {
    error("Could not read genotype file");
  }
  return n;
}

int numVariants(char *filename) {
  if (!ends_with(filename, ".a1")) {
    error("Currently, numVariants must be called on a file in .a1 format");
  }
  FILE *fp;
  if ((fp = fopen(filename, "r")) == NULL) {
    error("Could not open genotype file");
  }
  if (fseek(fp, 16, SEEK_SET) != 0) {
    error("Could not seek on genotype file");
  }
  uint64_t m = 0;
  if (fread(&m, 8, 1, fp) != 1) {
    error("Could not read genotype file");
  }
  return m;
}

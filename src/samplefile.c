// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "samplefile.h"
#include "errors.h"

void read_samples(char *fname, SampleFile **sampleFile) {
  SampleFile *sf = (SampleFile *)malloc(sizeof(SampleFile));
  if (sf == NULL) {
    error("Memory exhausted");
  }
  sampleFile = &sf; // ?
  typedef struct {
    char *line;
    void *next;
  } t_line;

  FILE *fp = fopen(fname, "r");
  if (fp == NULL) {
    error("Could not open file");
  }
  t_line *first = (t_line *)malloc(sizeof(t_line));
  if (first == NULL) {
    error("Memory exhausted");
  }
  size_t len = 0;
  t_line *last = first;
  first->line = NULL;
  first->next = NULL;
  ssize_t size = 0;
  int ns = 0;
  char *line = NULL;
  size = getline(&line, &len, fp);
  if (size != -1) {
    first->line = line;
  } else {
    error("Could not read sample file");
  }
  if (line == NULL) {
    error("Could not read sample file");
  }
  line = NULL;
  len = 0;
  while ((size = getline(&line, &len, fp)) != -1) {
    if (line == NULL) {
      error("Could not read sample file");
    }
    if (size < 2) {
      error("Could not read sample file");
    }
    if (line[0] == ' ') {
      error("Could not read sample file");
    }
    for (int i = 1; i < size; i++) {
      if (line[i - 1] == ' ' && line[i] == ' ') {
        error("Could not read sample file");
      }
    }

    last->line = line;
    last->next = (t_line *)malloc(sizeof(t_line));
    if (last->next == NULL) {
      error("Memory exhausted");
    }
    last = last->next;
    last->line = NULL;
    last->next = NULL;
    line = NULL;
    len = 0;
    ns++;
  }

  ns -= 2;
  if (ns < 1) {
    error("Could not read sample file");
  }

  sf->ns = ns;
  if ((sf->ids = (char **)malloc(sizeof(char *) * ns)) == NULL) {
    error("Memory exhausted");
  }
  if ((sf->lines = (char **)malloc(sizeof(char *) * ns)) == NULL) {
    error("Memory exhausted");
  }
  t_line *prev = first;
  sf->line1 = first->line;
  first = first->next;
  if (first == NULL) {
    error("Logic error in SampleFile");
  }
  free(prev);
  sf->line2 = first->line;
  first = first->next;
  free(prev);
  prev = first;
  for (int i = 0; i < ns; i++) {
    line = first->line;
    int found = -1;
    for (int j = 0; j < strlen(line); j++) {
      if (line[j] == ' ') {
        line[j] = '\0';
        found = j;
        break;
      }
    }

    if (found < 0) {
      error("Could not read SampleFile");
    }
    sf->ids[i] = line;
    sf->lines[i] = &(line[found + 1]);
    first = first->next;
    if (first == NULL) {
      error("Logic error in SampleFile");
    }
    free(prev);
    prev = first;
  }

  if (first != NULL) {
    error("Logic error in SampleFile");
  }
}

void write_samples(char *fname, SampleFile **sampleFile) {
  SampleFile *sf = *sampleFile;
  FILE *fp = fopen(fname, "w");
  if (fp == NULL) {
    error("Could not open file");
  }
  fprintf(fp, "%s", sf->line1);
  if (ferror(fp)) {
    error("Error writing to sample file");
  }
  fprintf(fp, "%s", sf->line2);
  if (ferror(fp)) {
    error("Error writing to sample file");
  }
  for (int i = 0; i < sf->ns; i++) {
    fprintf(fp, "%s %s", sf->ids[i], sf->lines[i]);
    if (ferror(fp)) {
      error("Error writing to sample file");
    }
  }
  fclose(fp);
}

void destroy_samples(SampleFile **sampleFile) {
  SampleFile *sf = *sampleFile;
  free(sf->line1);
  free(sf->line2);
  for (int i = 0; i < sf->ns; i++) {
    free(sf->ids[i]);
  }
  free(sf);
}


void read_strings(char *fname, StringFile **stringFile) {
  StringFile *sf = (StringFile *)malloc(sizeof(StringFile));
  if (sf == NULL) {
    error("Memory exhausted");
  }
  stringFile = &sf; // ?
  typedef struct {
    char *line;
    void *next;
  } t_line;

  FILE *fp = fopen(fname, "r");
  if (fp == NULL) {
    error("Could not open file");
  }
  t_line *first = (t_line *)malloc(sizeof(t_line));
  if (first == NULL) {
    error("Memory exhausted");
  }
  size_t len = 0;
  t_line *last = first;
  first->line = NULL;
  first->next = NULL;
  ssize_t size = 0;
  int nl = 0;
  char *line = NULL;
  size = getline(&line, &len, fp);
  if (size != -1) {
    first->line = line;
  } else {
    error("Could not read string file");
  }
  if (line == NULL) {
    error("Could not read string file");
  }
  line = NULL;
  len = 0;
  while ((size = getline(&line, &len, fp)) != -1) {
    if (line == NULL) {
      error("Could not read string file");
    }
    // chomp
    if (line[size] == '\n') {
      line[size] = '\0';
    }
    last->line = line;
    last->next = (t_line *)malloc(sizeof(t_line));
    if (last->next == NULL) {
      error("Memory exhausted");
    }
    last = last->next;
    last->line = NULL;
    last->next = NULL;
    line = NULL;
    len = 0;
    nl++;
  }

  sf->nl = nl;
  if ((sf->lines = (char **)malloc(sizeof(char *) * nl)) == NULL) {
    error("Memory exhausted");
  }
  t_line *prev = first;
  for (int i = 0; i < nl; i++) {
    line = first->line;
    sf->lines[i] = line;
    first = first->next;
    if (first == NULL) {
      error("Logic error in StringFile");
    }
    free(prev);
    prev = first;
  }

  if (first != NULL) {
    error("Logic error in StringFile");
  }
}

void destroy_strings(StringFile **stringFile) {
  StringFile *sf = *stringFile;
  for (int i = 0; i < sf->nl; i++) {
    free(sf->lines[i]);
  }
  free(sf);
}

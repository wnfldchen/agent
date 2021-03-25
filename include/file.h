// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#pragma once
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
// file.h

FILE* open_file(char const* filename, char const* mode);

void close_file(FILE* file);

static inline char *join(char **strings, char *sep) {
  int n = strlen(sep);
  size_t total = 0;
  int done, first, i;

  done = 0;
  i = 0;
  first = 1;
  while (!done) {
    if (strings[i] != NULL) {
      total += strlen(strings[i]);
      if (first) {
        first = 0;
      } else {
        total += n;
      }
    } else {
      done = 1;
    }

    i++;
  }

  done = 0;
  first = 1;
  i = 0;
  char *result = (char *)malloc(sizeof(char) * (total + 1));
  result[0] = '\0';
  while (!done) {
    if (strings[i] != NULL) {
      if (first) {
        first = 0;
      } else {
        strcat(result, sep);
      }

      strcat(result, strings[i]);
    } else {
      done = 1;
    }

    i++;
  }
  return result;
}

static inline int ends_with(char *string, char *suffix) {
  if (string == NULL || suffix == NULL) {
    return 0;
  }
  size_t len = strlen(string);
  size_t lens = strlen(suffix);
  if (lens >  len) {
    return 0;
  }
  return strncmp(string + (len - lens), suffix, lens) == 0;
}

static inline int can_read(char *fname) {
  if (access(fname, R_OK) != -1) {
    return 1;
  } 
  
  return 0;
}

static inline int can_create(char *fname) {
  if (access(fname, F_OK) == 0) {
    return 0;
  } 
  
  return 1;
}

static inline int file_exists(char* fname) {
	if (access(fname, F_OK)) {
          return 1;
        }
        return 0;
}

// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#ifndef __ERROR_H_
#  define __ERROR_H_

#define error(s) { \
  fprintf(stderr, "error: %s (%s:%d).\n", s, __FILE__, __LINE__); \
  exit(-1); \
}

#endif

// Copyright (c) 2019, Winfield Chen and Lloyd T. Elliott.

#ifndef __UTIL_H_
#  define __UTIL_H_

#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cmocka.h>
#include "errors.h"
#include "matrix.h"
#include "agent.h"

int status(void (*fp)(void));
void run_main(char *args);

#define assert_crash(s) { \
  void fp(void) { \
    run_main(s); \
  }; \
  assert_true(!status(fp)); \
} 

#define assert_nocrash(s) { \
  void fp(void) { \
    run_main(s); \
  }; \
  assert_true(status(fp)); \
} 

#endif

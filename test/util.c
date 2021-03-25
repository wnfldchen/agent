// Copyright (c) 2019, Winfield Chen and Lloyd T. Elliott.

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
#include "util.h"

int status(void (*fp)(void)) {
  pid_t p = fork();
  if ( p == -1 ) {
    error("Fork failed");
  } else if (p == 0) {
    fp();
    exit(0);
  }

  int es;
  if (waitpid(p, &es, 0) == -1) {
    error("Waitpid failed");
  }

  if (WIFEXITED(es)) {
    return WEXITSTATUS(es) == 0;
  }

  return 1;
}

void run_main(char *args) {
  int n = strlen(args);
  char *args2 = (char *)malloc(n + 1);
  if (args2 == NULL) {
    error("Memory exhausted");
  }
  strcpy(args2, args);
  args = args2;
  int argc = 1;
  for (int i = 0; i < n; i++) {
    if (args[i] == ' ') {
      argc++;
      args[i] = '\0';
    }
  }

  char **argv = (char **)malloc(argc * sizeof(char *));
  argv[0] = &(args[0]);
  int j = 0;
  for (int i = 0; i < n; i++) {
    if (args[i] == '\0') {
      j++;
      if (j < argc) {
        argv[j] = &(args[i+1]);
      }
    }
  }
  int fd1 = STDOUT_FILENO;
  int fd2 = STDERR_FILENO;
  FILE *fp = fopen("/dev/null", "w");
  if (fp == NULL) {
    error("Could not open /dev/null");
  }
  dup2(fileno(fp), fd1);
  dup2(fileno(fp), fd2);
  agent_main(argc, argv);
  fclose(fp);
  dup2(fd1, fd1);
  dup2(fd2, fd2);
  free(args);
}

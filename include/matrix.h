// Copyright 2019, Winfield Chen and Lloyd T. Elliott.
  
#ifndef __MATRIX_H_
#  define __MATRIX_H_

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <unistd.h>
#include "errors.h"

typedef struct {
  double *X;
  int N;
  int D;
} t_matrix;

void put(t_matrix A, const double f, const int i, const int j);
double get(t_matrix A, const int i, const int j);
void zero(t_matrix A);
t_matrix create(int N, int D);
void destroy(t_matrix A);
t_matrix load2(FILE *fp, int N, int M);
t_matrix load_file(FILE *fp);
t_matrix load_string(char *s);
t_matrix load(char *fname);
void print(t_matrix A);

#endif

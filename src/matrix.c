// Copyright 2019, Winfield Chen and Lloyd T. Elliott.
  
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <unistd.h>
#include "matrix.h"
#include "errors.h"

// matrix.h

void put(t_matrix A, const double f, const int i, const int j) {
  A.X[i + j * A.N] = f;
}

double get(t_matrix A, const int i, const int j) {
  return A.X[i + j * A.N];
}

void zero(t_matrix A) {
  bzero(&(A.X[0]), sizeof(double) * A.N * A.D);
}

t_matrix create(int N, int D) {
  t_matrix result;
  double *X = (double *)malloc(N * D * sizeof(double));
  if (X == NULL) {
    error("Memory exhausted");
  }
  result.X = X;
  result.N = N;
  result.D = D;
  return result;
}

void destroy(t_matrix A) {
  free(A.X);
}

// load(char *):
//
// Creates a matrix and reads a space separated file into the matrix.
//  The space separated file must have a header.
//  The matrix is column major.
//
// Example input:
//   fname = "test.txt"
//   test.txt:
//   V1 V2 V3
//   1.0 2.0 3.0
//   NaN -777.0 1e-6
//
// Example output matrix A:
//   A.X = {1.0, NaN, 2.0, -777.0, 3.0, 1e-6 } 
//   A.N = 2
//   A.D = 3
//

t_matrix load2(FILE *fp, int N, int M) {
  t_matrix result = create(N, M);
  double f;
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < M; j++) {
      f = 0;
      int num = fscanf(fp, "%lf", &f);
      if (num != 1) {
        error("Error converting phenotypes");
      }
      result.X[N * j + i] = f;
    }
  }

  if (fscanf(fp, "%lf", &f) == 1) {
    error("Input/output error");
  }

  return result;
}

t_matrix load_file(FILE *fp) {
  typedef struct {
    double *xs;
    void *next;
  } t_line;

  t_line *first = NULL, *next;
  if ((first = (t_line *)malloc(sizeof(t_line))) == NULL) {
    error("Memory limit");
  }
  int D = 1;
  int c = -1;
  while ((c = fgetc(fp)) != '\n') {
    if (c == EOF) {
      error("Input/output error");
    }
    if (c == ' ') {
      D += 1;
    }
  }

  if (D < 1) {
    error("No matrix in file");
  }
  if ((first->xs = (double *)malloc(D * sizeof(double))) == NULL) {
    error("Memory limit");
  }
  first->next = NULL;
  double f = NAN;
  next = first;
  int d = 0;
  int N = 0;
  // if (header == 0) {
  //   rewind(fp);
  //   if (fp == NULL) {
  //     error("Could not open file for reading");
  //   }
  // }

  while (fscanf(fp, "%lf", &f) == 1) {
    if (d == D) {
      if ((next->next = (t_line *)malloc(sizeof(t_line))) == NULL) {
        error("Memory limit reached");
      }
      next = next->next;
      if ((next->xs = (double *)malloc(D * sizeof(double))) == NULL) {
        error("Memory limit reached");
      }
      next->next = NULL;
      d = 0;
    }

    next->xs[d] = f;
    d += 1;
    if (d == D) {
      N += 1;
    }
  }

  if (fscanf(fp, "%lf", &f) == 1) {
    error("Input/output error");
  }
  if (d != D) {
    error("Input/output error");
  }
  double *X;
  if ((X = (double *)malloc(N * D * sizeof(double))) == NULL) {
    error("Memory exhausted");
  }
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < D; j++) {
      X[i + j * N] = first->xs[j];
    }
    next = first->next;
    free(first->xs);
    first->xs = NULL;
    free(first);
    first = next;
  }
  if (N == 0) {
    error("Input/output error");
  }
  t_matrix result;
  result.X = X;
  result.N = N;
  result.D = D;
  return result;
}

t_matrix load_string(char *s) {
  FILE *fp = fmemopen(s, strlen(s), "r");
  t_matrix result = load_file(fp);
  if (fp == NULL) {
    error("Could not load matrix from strig");
  }
  fclose(fp);
  return result;
}

t_matrix load(char *fname) {
  FILE *fp = fopen(fname, "r");
  if (fp == NULL) {
    error("Could not load matrix from file");
  }
  t_matrix result = load_file(fp);
  fclose(fp);
  return result;
}

void print(t_matrix A) {
  int N = A.N;
  int D = A.D;
  for (int i = 0; i < N; i++) {
    int first = 1;
    for (int d = 0; d < D; d++) {
      if (first) {
        first = 0;
      } else {
        printf(" ");
      }
      printf("%lf", get(A, i, d));
    }
    printf("\n");
  }
}

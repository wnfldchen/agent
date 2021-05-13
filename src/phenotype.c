// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#include "matrix.h"
#include "phenotype.h"

int numPhenotypes(char *fname) {
  FILE *fp = fopen(fname, "r");
  int D = 0;
  if (fp == NULL) {
    error("Could not open phenotype file");
  }

  int c;
  while (1) {
    c = getc(fp);
    if (c == EOF) {
      error("Could not convert phenotype headers");
    }

    if (c == ' ') {
      D += 1;
    }

    if (c == '\n') {
      break;
    }
  }

  return D + 1;
}

void load_phenotypes2(char *fname, t_matrix *y, t_matrix *obs, t_matrix *denom, int N, int D0, int D) {
  FILE *fp = fopen(fname, "r");
  if (fp == NULL) {
    error("Could not open phenotype file");
  }

  int c;
  while (1) {
    c = getc(fp);
    if (c == EOF) {
      error("Could not convert phenotype headers");
    }

    if (c == '\n') {
      break;
    }
  }

  *y = load2(fp, N, D0, D);
  fclose(fp);
  *obs = create(N, D);
  *denom = create(1, D);
  zero(*denom);
  for (int j = 0; j < D0; j++) {
    for (int i = 0; i < N; i++) {
      if (isnan(get(*y, i, j))) {
        put(*obs, 0.0, i, j);
        put(*y, 0.0, i, j);
      } else {
        put(*obs, 1.0, i, j);
        put(*denom, get(*denom, 0, j) + 1, 0, j);
      }
    }
  }

  for (int j = 0; j < D0; j++) {
    if (get(*denom, 0, j) < 0.5) {
      error("Values all missing for j-th phenotype");
    }
  }

  for (int j = 0; j < D0; j++) {
    double mu = 0.0;
    for (int i = 0; i < N; i++) {
      mu += get(*y, i, j);
    }
    mu /= get(*denom, 0, j);
    for (int i = 0; i < N; i++) {
      put(*y, get(*obs, i, j) * (get(*y, i, j) - mu), i, j);
    }
  }

  for (int j = D0; j < D; j++) {
    for (int i = 0; i < N; i++) {
      put(*obs, 1.0, i, j);
    }
    put(*denom, N, 0, j);
  }
}

void load_phenotypes(char *fname, t_matrix *y, t_matrix *obs, t_matrix *denom) {
  *y = load(fname);
  int N = y->N;
  int D = y->D;
  *obs = create(N, D);
  *denom = create(1, D);
  zero(*denom);
  for (int j = 0; j < D; j++) {
    for (int i = 0; i < N; i++) {
      if (isnan(get(*y, i, j))) {
        put(*obs, 0.0, i, j);
        put(*y, 0.0, i, j);
      } else {
        put(*obs, 1.0, i, j);
        put(*denom, get(*denom, 0, j) + 1, 0, j);
      }
    }
  }

  for (int j = 0; j < D; j++) {
    if (get(*denom, 0, j) < 0.5) {
      error("Values all missing for j-th phenotype");
    }
  }

  for (int j = 0; j < D; j++) {
    double mu = 0.0;
    for (int i = 0; i < N; i++) {
      mu += get(*y, i, j);
    }
    mu /= get(*denom, 0, j);
    for (int i = 0; i < N; i++) {
      put(*y, get(*obs, i, j) * (get(*y, i, j) - mu), i, j);
    }
  }
}

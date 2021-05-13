// Copyright 2019, Winfield Chen and Lloyd T. Elliott.

#ifndef __PHENOTYPE_H_
#  define __PHENOTYPE_H_

#include "matrix.h"

typedef struct {
  t_matrix *y;
  t_matrix *yt;
  t_matrix *obs;
  t_matrix *denom;
} t_phenotype;

void load_phenotypes(char *fname, t_matrix *y, t_matrix *obs, t_matrix *denom);
void load_phenotypes2(char *fname, t_matrix *y, t_matrix *obs, t_matrix *denom, int N, int D0, int D);
int numPhenotypes(char *);

#endif

// Copyright 2019, Winfield Chen and Lloyd T. Elliott.
  
#ifndef __STATISTICS_H_
#  define __STATISTICS_H_

#include <gsl/gsl_sf.h>
#include "matrix.h"

double tcdf1m(double t, double nu);
double log_tcdf1m(double t, double nu);
void regression(t_matrix g,     
                t_matrix y,     
                t_matrix yt,     
                t_matrix obs,   
                t_matrix denom, 
                t_matrix beta,  
                t_matrix se,    
                t_matrix tstat, 
                t_matrix pval,
                t_matrix b1,
                t_matrix w1,
                t_matrix w2); 
#endif

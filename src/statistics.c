// Copyright 2019, Winfield Chen and Lloyd T. Elliott.
  
#include "statistics.h"
#include "matrix.h"
#include <mkl_lapack.h>
#include <mkl_blas.h>
#include <immintrin.h>
#include <emmintrin.h>

#define INT MKL_INT
#define DOUBLE double

// statistics.c

// p-value of t-statistic with nu degrees of freedom
double tcdf1m(double t, double nu) {
  double x = nu / (t * t + nu);
  return gsl_sf_beta_inc(nu/2.0, 0.5, x);
}

#define LOG10 2.3025850929940459010936137929093092679977416992188

// -log10 p-value of t-statistic with nu degrees of freedom
double log_tcdf1m(double t, double nu) {
  return -gsl_sf_log(tcdf1m(t, nu)) / LOG10;
}

// MLK matrix/vector multiplication
void dgemv_(char *,
  INT *,
  INT *,
  DOUBLE *,
  DOUBLE *,
  INT *,
  DOUBLE *,
  INT *,
  DOUBLE *,
  DOUBLE *,
  INT *);

void dgemm_ (
  char *,   // TRANSA
  char *,   // TRANSB
  INT *,    // M
  INT *,    // N
  INT *,    // K
  DOUBLE *, // ALPHA
  DOUBLE *, // A
  INT *,    // LDA
  DOUBLE *, // B
  INT *,    // LDB
  DOUBLE *, // BETA
  DOUBLE *, // C
  INT *     // LDC
);

double ddot_ (
  INT *,
  DOUBLE *,
  INT *,
  DOUBLE *,
  INT *
);

// Carryout D univariate linear regressions.
void regression(t_matrix g,               // input variables
                t_matrix y,               // input variables
                t_matrix yt,              // input variables
                t_matrix obs,             // input variables
                t_matrix denom,           // input variables
                t_matrix beta,            // ouput variables
                t_matrix se,              // ouput variables
                t_matrix tstat,           // ouput variables
                t_matrix pval,            // ouput variables
                t_matrix b1,              // temporary variables
                t_matrix w1,              // temporary variables
                t_matrix w2) {            // temporary variables

  const int N = y.N;
  const int D = y.D;
  const int M = g.D;

  int missing = 0;
  for (int j = 0; j < D; j++) {
    if (get(denom, 0, j) < N - 0.5) {
      missing = 1;
      break;
    }
  }


  switch (missing) {
    case 0: {
      const double N1 = N - 1.0;
      double *xxs;
      double *XXS = b1.X;
      bzero(XXS, M * sizeof(double));
      xxs = XXS;
      bzero(w1.X, M * D * sizeof(double));
      for (int k = 0; k < M; k++) {
        double MU = 0.0;
        double *gg1;
        
        gg1 = &(g.X[N * k]);
        for (int i = 0; i < N; i++) {
          MU += (*gg1);
          gg1++;
        }
        
        MU = MU / N;
        gg1 = &(g.X[N * k]);
        *xxs = 0.0;
        for (int i = 0; i < N; i++) {
          (*gg1) = (*gg1) - MU;
          *xxs += (*gg1) * (*gg1);
          gg1++;
        }
        
        xxs++;
      }

      {
        char TRANSA = 'N';
        char TRANSB = 'N';
        INT MM = D;
        INT NN = M;
        INT KK = N;
        DOUBLE alpha = 1.0;
        DOUBLE *AA = yt.X;
        INT LDA = D;
        DOUBLE *BB = g.X;
        INT LDB = N;
        DOUBLE BETA = 0.0;
        DOUBLE *CC = w1.X;
        INT LDC = D;

        dgemm_(
          &TRANSA,  //  char *,   // TRANSA    
          &TRANSB,  //  char *,   // TRANSB    
          &MM,      //  INT *,    // M         
          &NN,      //  INT *,    // N         
          &KK,      //  INT *,    // K         
          &alpha,   //  DOUBLE *, // ALPHA     
          AA,       //  DOUBLE *, // A         
          &LDA,     //  INT *,    // LDA       
          BB,       //  DOUBLE *, // B         
          &LDB,     //  INT *,    // LDB       
          &BETA,    //  DOUBLE *, // BETA      
          CC,       //  DOUBLE *, // C         
          &LDC      //  INT *     // LDC       
        );
      }

      double *XYS = w1.X;
      double *bb1 = beta.X;
      double *ss1 = se.X;
      double *tt1 = tstat.X;
      double *pp1 = pval.X;
      double *xys = XYS;
      xys = XYS;
      xxs = XXS;
      for (int k = 0; k < M; k++) {
        double *yy1 = y.X;
        for (int j = 0; j < D; j++) {
          double BETA = *xys / *xxs;
          xys++;

          double SIGMA = 0.0;                     // here
          double *gg1 = &g.X[k * N];              // here
          __m256d sigmas = _mm256_setzero_pd();
          int i = 0;
          if (N >= 4) {
              __m256d neg_betas = _mm256_set1_pd(-BETA);
              for (; i <= N - 4; i += 4) {
                  __m256d ggs = _mm256_loadu_pd(gg1);
                  __m256d res = _mm256_loadu_pd(yy1);
                  ggs = _mm256_mul_pd(ggs, neg_betas);
                  res = _mm256_add_pd(res, ggs);
                  res = _mm256_mul_pd(res, res);
                  sigmas = _mm256_add_pd(sigmas, res);
                  gg1 += 4;
                  yy1 += 4;
              }
              __m128d s_l = _mm256_castpd256_pd128(sigmas);
              __m128d s_h = _mm256_extractf128_pd(sigmas, 1);
              s_l = _mm_add_pd(s_l, s_h);
              __m128d s_l_h = _mm_unpackhi_pd(s_l, s_l);
              SIGMA = _mm_cvtsd_f64(_mm_add_sd(s_l, s_l_h));
          }
          for (; i < N; i++) {           // here
            double res = (*yy1) - BETA * (*gg1);  // here
            SIGMA += res * res;                   // here
            gg1++;                                // here
            yy1++;                                // here
          }                                       // here

          SIGMA = SIGMA / N1;
          double SE = sqrt(SIGMA / (*xxs)); 
          double TSTAT = BETA / SE;
          double PVAL = log_tcdf1m(TSTAT, N1);
          if (isnan(TSTAT) || isinf(TSTAT)) {
            BETA = NAN;
            SE = NAN;
            TSTAT = NAN;
            PVAL = NAN;
          } else {
            PVAL = log_tcdf1m(TSTAT, N - 1.0);
            if (isnan(PVAL) || isinf(PVAL)) {
              BETA = NAN;
              SE = NAN;
              TSTAT = NAN;
              PVAL = NAN;
            }
          }
          (*bb1) = BETA; bb1++;
          (*ss1) = SE; ss1++;
          (*tt1) = TSTAT; tt1++;
          (*pp1) = PVAL; pp1++;
        }
        
        xxs++;
      }

      break;
    }

    case 1:
    default:
      error("Unknown missingness condition");

  }

  return;
}

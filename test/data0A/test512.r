#!/bin/env Rscript

N = 6
D = 512
a = matrix(rnorm(N*D), nrow = N, ncol = D)
write.table(a, 'test512.txt', append = F, quote = F, sep = ' ', col.names = T, row.names = F)

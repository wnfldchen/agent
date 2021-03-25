set.seed(1)
M = 512
N = 6
p = matrix(rnorm(M * N), N, M)
write.table(p,
  file = 'test01.512',
  row.names = FALSE,
  col.names = TRUE,
  quote = FALSE)

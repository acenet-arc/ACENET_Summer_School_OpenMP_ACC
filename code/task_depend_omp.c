/* --- File task_depend_omp.c --- */
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{

  int N = 8;
  int x[N][N], y[N][N];
  int i, j;

  /* Initialize x,y */
  for (i = 0; i < N; i++)
  {
    x[0][i] = x[i][0] = y[0][i] = y[i][0] = i;
  }

  /* Serial computation */
  for (i = 1; i < N; i++)
  {
    for (j = 1; j < N; j++)
      x[i][j] = x[i - 1][j] + x[i][j - 1];
  }

  /* Parallel computation */
#pragma omp parallel
#pragma omp single
  for (i = 1; i < N; i++)
  {
    for (j = 1; j < N; j++)
#pragma omp task 
      y[i][j] = y[i - 1][j] + y[i][j - 1];
  }

  printf("Serial result:\n");
  for (i = 0; i < N; i++)
  {
    for (j = 0; j < N; j++)
      printf("%6d", x[i][j]);
    printf("\n");
  }
  printf("Parallel result:\n");
  for (i = 0; i < N; i++)
  {
    for (j = 0; j < N; j++)
      printf("%6d", y[i][j]);
    printf("\n");
  }
}

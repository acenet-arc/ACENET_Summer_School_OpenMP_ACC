/* --- File task_depend.c --- */
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {

  int N = 8;
  int x[N][N];
  int i,j;

  /* Initialize x */
  for(i=0;i<N;i++)
    for(j=0;j<N;j++)
      x[i][j]=i+j;

  /* Serial computation */
  for(i=1;i<N;i++){
    for(j=1;j<N;j++)
      x[i][j] = x[i-1][j] + x[i][j-1];
  }

  printf("Serial result:\n");
  for(i=1;i<N;i++){
    for(j=1;j<N;j++)
      printf("%8d ",x[i][j]);
    printf("\n");
  }

  /* Reset x */
  for(i=0;i<N;i++)
    for(j=0;j<N;j++)
      x[i][j]=i+j;

  /* Parallel computation */
#pragma omp parallel
  /* Generate parallel tasks */
  for(i=1;i<N;i++){
    for(j=1;j<N;j++)
      x[i][j] = x[i-1][j] + x[i][j-1];
  }

  printf("Parallel result:\n");
  for(i=1;i<N;i++){
    for(j=1;j<N;j++)
      printf("%8d ",x[i][j]);
    printf("\n");
  }
}

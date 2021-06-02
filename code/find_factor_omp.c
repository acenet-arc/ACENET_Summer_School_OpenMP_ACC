/* File find_factor_omp.c */
#include <stdio.h>
#include <stdlib.h>

int main()
{
  long N = 4993 * 5393;
  long f;
#pragma omp parallel
#pragma omp single
  for (f = 2; f <= N; f++) /* Loop generating tasks */
  {
    if (f % 200 == 0)
    {
      fprintf(stdout, "%li tasks generated\n", f);
      fflush(stdout);
    }
#pragma omp task
    { /* Check if f is a factor */
      if (f % 200 == 0)
        fprintf(stdout, "    %li tasks done\n", f);
      if (N % f == 0)
      { // the remainder is 0, found factor!
        fprintf(stdout, "Factor: %li\n", f);
        exit(0);
      }
      else
        for (int i = 1; i < 4e6; i++)
          ; /* Burn some CPU cycles */
    }
  } 
}
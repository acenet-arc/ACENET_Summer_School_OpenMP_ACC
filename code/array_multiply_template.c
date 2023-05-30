/*  -- File array_multiply.c --- */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char **argv)
{
  	double start, end;
	int size = 5e8;
	int multiplier = 2;
	int *A, *C;
	int i;

	/* Allocate memory for arrays */
	A = malloc(size * sizeof(int));
	C = malloc(size * sizeof(int));

        start = omp_get_wtime();
	/* Multiply array a by multiplier */
	for (i = 0; i < size; i++)
	{
		C[i] = multiplier * A[i];
	}
	end = omp_get_wtime();
	printf("Total time is %f s\n", end-start);
}

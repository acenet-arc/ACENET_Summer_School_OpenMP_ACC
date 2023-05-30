#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char **argv)
{
    double start, end;
    int size = 1e4;
    int **A, *C;
    int i, j;

    /* Allocate memory */
    C = malloc(size * sizeof(int));
    A = (int **)malloc(size * sizeof(int *));
    for (i = 0; i < size; i++)
        A[i] = malloc(size * sizeof(int));
    /* Set all matrix elements to 1 */
    for (i = 0; i < size; i++)
        for (j = 0; j < size; j++)
            A[i][j] = 1;
    /* Zero the accumulator */
    for (i = 0; i < size; i++)
        C[i] = 0;

    start = omp_get_wtime();
    #pragma omp parallel for private(j)
    /* Each thread sums one column */
    for (i = 0; i < size; i++)
        for (j = 0; j < size; j++)
            C[i] += A[i][j];
        
    int total = 0;
    /* Add sums of all columns together */
    for (i = 0; i < size; i++)
        total += C[i];

    end = omp_get_wtime();
    printf("Total is %d, time is %f s\n", total, end-start);
}

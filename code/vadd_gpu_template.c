/* Compute sum of vectors A and B */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char *argv[])
{
    double start, end;
    int size = 1e8;
    float *A;
    float *B;
    float *C;
    double sum = 0.0f;

    int ncycles=atoi(argv[1]);
    A = (float *)malloc(size * sizeof(float *));
    B = (float *)malloc(size * sizeof(float *));
    C = (float *)malloc(size * sizeof(float *));

    /* Initialize vectors */
    for (int i = 0; i < size; i++)
    {
        A[i] = (float)rand() / RAND_MAX;
        B[i] = (float)rand() / RAND_MAX;
    }

    start = omp_get_wtime();
    for (int k = 0; k < ncycles; k++)
        for (int i = 0; i < size; i++)
        {
            C[i] = A[i] + B[i];
            sum += C[i];
        }
    end = omp_get_wtime();

    printf("Time: %f seconds\n", end - start);
    sum = sum / size;
    printf("Sum = %f\n ", sum / ncycles);
}

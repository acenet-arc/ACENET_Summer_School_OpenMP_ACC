/* File: laplace2d.c */
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv)
{
    FILE *output_unit;
    int i, j;
    int n = 2048;
    int m = 2048;            /* Size of the mesh */
    int qn = (int)n * 0.5;   /* x-coordinate of the point heat source */
    int qm = (int)m * 0.5;   /* y-coordinate of the point heat source */
    float h = 0.05;          /* Instantaneous heat */
    int iter_max = 1e4;      /* Maximum number of iterations */
    const float tol = 1e-6f; /* Tolerance */

    struct timespec ts_start, ts_end;
    float time_total;
    float ** __restrict U; 
    float ** __restrict U_new;
    float ** __restrict F;

    /* Allocate memory */
    F = (float **)malloc(m * sizeof(float *)); /* Heat source array */
    for (i = 0; i < m; i++)
        F[i] = malloc(n * sizeof(float));
    F[qn][qm] = h;                             /* Set point heat source */
    U = (float **)malloc(m * sizeof(float *)); /* Plate temperature */
    for (i = 0; i < m; i++)
        U[i] = malloc(n * sizeof(float));
    U_new = (float **)malloc(m * sizeof(float *)); /* Temporary new temperature */
    for (i = 0; i < m; i++)
        U_new[i] = malloc(n * sizeof(float));

    printf("Jacobi relaxation Calculation: %d x %d mesh\n", n, m);
    /* Get calculation  start time */
    clock_gettime(CLOCK_MONOTONIC, &ts_start);

    /* The main loop */
    int iter = 0;       /* Iterration counter */
    float error = 1.0f; /* The initial error */
#pragma acc data copyin(U [0:m] [0:n], U_new [0:m] [0:n], F [0:m] [0:n]) copyout(U [0:m] [0:n])
    while (error > tol && iter < iter_max)
    {
        error = 0.f;
        /* Compute the new temperature  at the point i,j as a weighted average of */
        /* the four neighbors and the heat source function F(i,j) */
#pragma acc kernels
        {
            #pragma omp parallel for private(i) reduction(max: error)
            // You can parallelize using parallel loops instead of kernels
            //  #pragma acc parallel loop
            for (j = 1; j < n - 1; j++)
            {
                for (i = 1; i < m - 1; i++)
                {
                    U_new[j][i] = 0.25f * (U[j][i + 1] + U[j][i - 1] + U[j - 1][i] + U[j + 1][i]) + F[j][i];
                    error = fmaxf(error, fabsf(U_new[j][i] - U[j][i]));
                }
            }
            /*  Update temperature */
            #pragma omp parallel for private(i)
            // #pragma acc parallel loop
            for (int j = 1; j < n - 1; j++)
            {
                for (i = 1; i < m - 1; i++)
                    U[j][i] = U_new[j][i];
            }
            if (iter % 200 == 0) /* Print error every 200 iterrations */
                printf("%5d, %0.6e\n", iter, error);
            iter++;
        }
    }

    /* Get end time */
    clock_gettime(CLOCK_MONOTONIC, &ts_end);
    time_total = (ts_end.tv_sec - ts_start.tv_sec) * 1e9 + (ts_end.tv_nsec - ts_start.tv_nsec);
    printf("\nTotal relaxation time is %f sec\n", time_total / 1e9);

    /* Write data to a binary file for paraview visualization */
    char *output_filename = "poisson_1024x1024_float32.raw";
    output_unit = fopen(output_filename, "wb");
    for (j = 0; j < n; j++)
        fwrite(U[j], m * sizeof(float), 1, output_unit);
    fclose(output_unit);
}

/* --- File elect_energy_omp.c --- */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

int main(int argc, char **argv)
{
	struct timespec ts_start, ts_end;
	float time_total;
	int n = 70;				   /* number of atoms per side */
	int n_charges = n * n * n; /* total number of charges */
	float a = 0.5;			   /* lattice constant a (a=b=c) */
	float *q;				   /* array of charges */
	float *x, *y, *z;		   /* x,y,z coordinates */
	int i, j, k;
	float dx,dy,dz,dist;
	double Energy=0.0;

	q = malloc(n_charges * sizeof(float));
	x = malloc(n_charges * sizeof(float));
	y = malloc(n_charges * sizeof(float));
	z = malloc(n_charges * sizeof(float));
	/* Seed random number generator */
	srand(111);
	/* initialize coordinates and charges */
	int l = 0;
	for (i = 0; i < n; i++)
		for (j = 0; j < n; j++)
			for (k = 0; k < n; k++)
			{
				x[l] = i * a;
				y[l] = j * a;
				z[l] = k * a;
				/* Generate random numbers between -0.5 and 0.5 */
				q[l] = 10*((double)random() / (double)RAND_MAX - 0.5);
				l++;
			}

	clock_gettime(CLOCK_MONOTONIC, &ts_start);
	/* Calculate sum of pairwise interactions: q[i]*q[j]/dist[i,j] */
#pragma omp parallel for private(j, dx, dy, dz, dist) reduction(+:Energy) schedule(dynamic)
	for (i = 0; i < n_charges; i++)
	{
		for (j = i + 1; j < n_charges; j++)
		{
			dx = x[i] - x[j];
			dy = y[i] - y[j];
			dz = z[i] - z[j];
			dist = sqrt(dx * dx + dy * dy + dz * dz);
			Energy += q[i] * q[j] / dist;
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &ts_end);

	time_total = (ts_end.tv_sec - ts_start.tv_sec)*1e9 + (ts_end.tv_nsec - ts_start.tv_nsec);
	printf("\nTotal time is %f ms, Energy is %.3f\n", time_total/1e6, Energy*1e-4);
}

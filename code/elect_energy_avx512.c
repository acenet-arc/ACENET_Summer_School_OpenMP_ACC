#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <immintrin.h>

/* gcc -o evec1 elect_energy_vec_01.c -O4 -lm -fopenmp -march=native */

int main(int argc, char **argv)
{
	struct timespec ts_start, ts_end;
	float time_total;

	int i, j, m, ix, iy, iz;
	int n = 70;				   /* number of atoms per side */
	int n_charges = n * n * n; /* total number of charges */
	float a = 0.5;			   /* Lattice constant */

	long v_element_count = 0;
	long v_count = 0;

	float tmp_vec[4][16] __attribute__((aligned(64)));
	double Energy = 0.0;

	__m512 *X, *Y, *Z, *Q;
	__m512 tmpQ[16], tmpX[16], tmpY[16], tmpZ[16];
	__m512 r_vec, result, vcps, diff[16], mask[16];

	X = aligned_alloc(64, (n_charges) * sizeof(float));
	Y = aligned_alloc(64, (n_charges) * sizeof(float));
	Z = aligned_alloc(64, (n_charges) * sizeof(float));
	Q = aligned_alloc(64, (n_charges) * sizeof(float));

	srand(111); /* Seed random numbers generator */ 
	/* Initialize X,Y,Z,Q  and pack them in 512-bit long vectors */
	for (ix = 0; ix < n; ix++)
		for (iy = 0; iy < n; iy++)
			for (iz = 0; iz < n; iz++)
			{
				tmp_vec[0][v_element_count] = ix * a;									   /* x coordinates */
				tmp_vec[1][v_element_count] = iy * a;									   /* y coordinates */
				tmp_vec[2][v_element_count] = iz * a;									   /* z coordinates */
				tmp_vec[3][v_element_count] = 10*((double)random() / (double)RAND_MAX - 0.5); /* charges */
				v_element_count++;
				/* when 16 elements are computed pack them into _m512 vectors */
				if (v_element_count == 16)
				{
					X[v_count] = _mm512_set_ps(
						tmp_vec[0][15], tmp_vec[0][14], tmp_vec[0][13], tmp_vec[0][12],
						tmp_vec[0][11], tmp_vec[0][10], tmp_vec[0][9], tmp_vec[0][8],
						tmp_vec[0][7], tmp_vec[0][6], tmp_vec[0][5], tmp_vec[0][4],
						tmp_vec[0][3], tmp_vec[0][2], tmp_vec[0][1], tmp_vec[0][0]);
					Y[v_count] = _mm512_set_ps(
						tmp_vec[1][15], tmp_vec[1][14], tmp_vec[1][13], tmp_vec[1][12],
						tmp_vec[1][11], tmp_vec[1][10], tmp_vec[1][9], tmp_vec[1][8],
						tmp_vec[1][7], tmp_vec[1][6], tmp_vec[1][5], tmp_vec[1][4],
						tmp_vec[1][3], tmp_vec[1][2], tmp_vec[1][1], tmp_vec[1][0]);
					Z[v_count] = _mm512_set_ps(
						tmp_vec[2][15], tmp_vec[2][14], tmp_vec[2][13], tmp_vec[2][12],
						tmp_vec[2][11], tmp_vec[2][10], tmp_vec[2][9], tmp_vec[2][8],
						tmp_vec[2][7], tmp_vec[2][6], tmp_vec[2][5], tmp_vec[2][4],
						tmp_vec[2][3], tmp_vec[2][2], tmp_vec[2][1], tmp_vec[2][0]);
					Q[v_count] = _mm512_set_ps(
						tmp_vec[3][15], tmp_vec[3][14], tmp_vec[3][13], tmp_vec[3][12],
						tmp_vec[3][11], tmp_vec[3][10], tmp_vec[3][9], tmp_vec[3][8],
						tmp_vec[3][7], tmp_vec[3][6], tmp_vec[3][5], tmp_vec[3][4],
						tmp_vec[3][3], tmp_vec[3][2], tmp_vec[3][1], tmp_vec[3][0]);
					v_count++;
					v_element_count = 0;
					memset(tmp_vec, 0, 64 * sizeof(float));
				}
			}

	/* Treat the remainder. The last vector is padded with zero charges and x coordinates outside 
	of the range that are guaranteed not to result in any zero pairwise distances */
	if (v_element_count != 0)
	{
		for (float v = n * a * 2; v_element_count < 16; v_element_count++, v += 1)
		{
			tmp_vec[0][v_element_count] = v;
			tmp_vec[1][v_element_count] = 0.0;
			tmp_vec[2][v_element_count] = 0.0;
			tmp_vec[3][v_element_count] = 0.0;
		}
		X[v_count] = _mm512_set_ps(
			tmp_vec[0][15], tmp_vec[0][14], tmp_vec[0][13], tmp_vec[0][12],
			tmp_vec[0][11], tmp_vec[0][10], tmp_vec[0][9], tmp_vec[0][8],
			tmp_vec[0][7], tmp_vec[0][6], tmp_vec[0][5], tmp_vec[0][4],
			tmp_vec[0][3], tmp_vec[0][2], tmp_vec[0][1], tmp_vec[0][0]);
		Y[v_count] = _mm512_set_ps(
			tmp_vec[1][15], tmp_vec[1][14], tmp_vec[1][13], tmp_vec[1][12],
			tmp_vec[1][11], tmp_vec[1][10], tmp_vec[1][9], tmp_vec[1][8],
			tmp_vec[1][7], tmp_vec[1][6], tmp_vec[1][5], tmp_vec[1][4],
			tmp_vec[1][3], tmp_vec[1][2], tmp_vec[1][1], tmp_vec[1][0]);
		Z[v_count] = _mm512_set_ps(
			tmp_vec[2][15], tmp_vec[2][14], tmp_vec[2][13], tmp_vec[2][12],
			tmp_vec[2][11], tmp_vec[2][10], tmp_vec[2][9], tmp_vec[2][8],
			tmp_vec[2][7], tmp_vec[2][6], tmp_vec[2][5], tmp_vec[2][4],
			tmp_vec[2][3], tmp_vec[2][2], tmp_vec[2][1], tmp_vec[2][0]);
		Q[v_count] = _mm512_set_ps(
			tmp_vec[3][15], tmp_vec[3][14], tmp_vec[3][13], tmp_vec[3][12],
			tmp_vec[3][11], tmp_vec[3][10], tmp_vec[3][9], tmp_vec[3][8],
			tmp_vec[3][7], tmp_vec[3][6], tmp_vec[3][5], tmp_vec[3][4],
			tmp_vec[3][3], tmp_vec[3][2], tmp_vec[3][1], tmp_vec[3][0]);
		v_count++;
	}

	/* mask upper triangular matrix elements */
	mask[0] = (__m512)_mm512_set_epi32(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0);
	mask[1] = (__m512)_mm512_set_epi32(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0);
	mask[2] = (__m512)_mm512_set_epi32(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0);
	mask[3] = (__m512)_mm512_set_epi32(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0);
	mask[4] = (__m512)_mm512_set_epi32(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0);
	mask[5] = (__m512)_mm512_set_epi32(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0);
	mask[6] = (__m512)_mm512_set_epi32(-1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0);
	mask[7] = (__m512)_mm512_set_epi32(-1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0);
	mask[8] = (__m512)_mm512_set_epi32(-1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	mask[9] = (__m512)_mm512_set_epi32(-1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	mask[10] = (__m512)_mm512_set_epi32(-1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	mask[11] = (__m512)_mm512_set_epi32(-1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	mask[12] = (__m512)_mm512_set_epi32(-1, -1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	mask[13] = (__m512)_mm512_set_epi32(-1, -1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	mask[14] = (__m512)_mm512_set_epi32(-1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	mask[15] = (__m512)_mm512_set_epi32(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

	clock_gettime(CLOCK_MONOTONIC, &ts_start);
#pragma omp parallel for private(tmpQ, tmpX, tmpY, tmpZ, i, j, m, diff, r_vec, vcps, result) reduction(+:Energy) schedule(dynamic)
	for (i = 0; i < v_count; i++)
	{
		/* For each i prepare 16 - element X, Y, Z, and Q vectors */
		tmpQ[0] = _mm512_set1_ps(Q[i][0]);
		tmpQ[1] = _mm512_set1_ps(Q[i][1]);
		tmpQ[2] = _mm512_set1_ps(Q[i][2]);
		tmpQ[3] = _mm512_set1_ps(Q[i][3]);
		tmpQ[4] = _mm512_set1_ps(Q[i][4]);
		tmpQ[5] = _mm512_set1_ps(Q[i][5]);
		tmpQ[6] = _mm512_set1_ps(Q[i][6]);
		tmpQ[7] = _mm512_set1_ps(Q[i][7]);
		tmpQ[8] = _mm512_set1_ps(Q[i][8]);
		tmpQ[9] = _mm512_set1_ps(Q[i][9]);
		tmpQ[10] = _mm512_set1_ps(Q[i][10]);
		tmpQ[11] = _mm512_set1_ps(Q[i][11]);
		tmpQ[12] = _mm512_set1_ps(Q[i][12]);
		tmpQ[13] = _mm512_set1_ps(Q[i][13]);
		tmpQ[14] = _mm512_set1_ps(Q[i][14]);
		tmpQ[15] = _mm512_set1_ps(Q[i][15]);

		tmpX[0] = _mm512_set1_ps(X[i][0]);
		tmpX[1] = _mm512_set1_ps(X[i][1]);
		tmpX[2] = _mm512_set1_ps(X[i][2]);
		tmpX[3] = _mm512_set1_ps(X[i][3]);
		tmpX[4] = _mm512_set1_ps(X[i][4]);
		tmpX[5] = _mm512_set1_ps(X[i][5]);
		tmpX[6] = _mm512_set1_ps(X[i][6]);
		tmpX[7] = _mm512_set1_ps(X[i][7]);
		tmpX[8] = _mm512_set1_ps(X[i][8]);
		tmpX[9] = _mm512_set1_ps(X[i][9]);
		tmpX[10] = _mm512_set1_ps(X[i][10]);
		tmpX[11] = _mm512_set1_ps(X[i][11]);
		tmpX[12] = _mm512_set1_ps(X[i][12]);
		tmpX[13] = _mm512_set1_ps(X[i][13]);
		tmpX[14] = _mm512_set1_ps(X[i][14]);
		tmpX[15] = _mm512_set1_ps(X[i][15]);

		tmpY[0] = _mm512_set1_ps(Y[i][0]);
		tmpY[1] = _mm512_set1_ps(Y[i][1]);
		tmpY[2] = _mm512_set1_ps(Y[i][2]);
		tmpY[3] = _mm512_set1_ps(Y[i][3]);
		tmpY[4] = _mm512_set1_ps(Y[i][4]);
		tmpY[5] = _mm512_set1_ps(Y[i][5]);
		tmpY[6] = _mm512_set1_ps(Y[i][6]);
		tmpY[7] = _mm512_set1_ps(Y[i][7]);
		tmpY[8] = _mm512_set1_ps(Y[i][8]);
		tmpY[9] = _mm512_set1_ps(Y[i][9]);
		tmpY[10] = _mm512_set1_ps(Y[i][10]);
		tmpY[11] = _mm512_set1_ps(Y[i][11]);
		tmpY[12] = _mm512_set1_ps(Y[i][12]);
		tmpY[13] = _mm512_set1_ps(Y[i][13]);
		tmpY[14] = _mm512_set1_ps(Y[i][14]);
		tmpY[15] = _mm512_set1_ps(Y[i][15]);

		tmpZ[0] = _mm512_set1_ps(Z[i][0]);
		tmpZ[1] = _mm512_set1_ps(Z[i][1]);
		tmpZ[2] = _mm512_set1_ps(Z[i][2]);
		tmpZ[3] = _mm512_set1_ps(Z[i][3]);
		tmpZ[4] = _mm512_set1_ps(Z[i][4]);
		tmpZ[5] = _mm512_set1_ps(Z[i][5]);
		tmpZ[6] = _mm512_set1_ps(Z[i][6]);
		tmpZ[7] = _mm512_set1_ps(Z[i][7]);
		tmpZ[8] = _mm512_set1_ps(Z[i][8]);
		tmpZ[9] = _mm512_set1_ps(Z[i][9]);
		tmpZ[10] = _mm512_set1_ps(Z[i][10]);
		tmpZ[11] = _mm512_set1_ps(Z[i][11]);
		tmpZ[12] = _mm512_set1_ps(Z[i][12]);
		tmpZ[13] = _mm512_set1_ps(Z[i][13]);
		tmpZ[14] = _mm512_set1_ps(Z[i][14]);
		tmpZ[15] = _mm512_set1_ps(Z[i][15]);

		/* Accumulate interactions within 16x16 blocks [i,i] in the vector 'vcps' */
		vcps = _mm512_setzero_ps();
		for (m = 0; m < 16; m++)
		{
			/* compute dx, dy, dz */
			diff[0] = _mm512_sub_ps(tmpX[m], X[i]);
			diff[1] = _mm512_sub_ps(tmpY[m], Y[i]);
			diff[2] = _mm512_sub_ps(tmpZ[m], Z[i]);
			/* compute dx*dx + dy*dy + dz*dz */
			r_vec = _mm512_fmadd_ps(diff[0], diff[0], _mm512_setzero_ps());
			r_vec = _mm512_fmadd_ps(diff[1], diff[1], r_vec);
			r_vec = _mm512_fmadd_ps(diff[2], diff[2], r_vec);
			/* compute reciprocal distance [m][i]*/
			r_vec = _mm512_rsqrt14_ps(r_vec);
			/* compute Q[m]*Q[i]/distance[m][i] */
			result = _mm512_mul_ps(tmpQ[m], Q[i]); /* Q[m]*Q[i] */
			result = _mm512_mul_ps(result, r_vec);
			result = _mm512_and_ps(mask[m], result); /* Apply mask */
			vcps = _mm512_add_ps(vcps, result);
		}
		/* sum all elements of 'vcps' vector and transfer
				the result into double precision accumulator Energy */
		Energy += _mm512_reduce_add_ps(vcps);

		/* Accumulate interactions between different 16x16 blocks [i,j] in the vector 'vcps' */
		for (j = i + 1; j < v_count; j++)
		{
			vcps = _mm512_setzero_ps();
			for (m = 0; m < 16; m++)
			{
				diff[0] = _mm512_sub_ps(tmpX[m], X[j]);
				diff[1] = _mm512_sub_ps(tmpY[m], Y[j]);
				diff[2] = _mm512_sub_ps(tmpZ[m], Z[j]);

				r_vec = _mm512_fmadd_ps(diff[0], diff[0], _mm512_setzero_ps());
				r_vec = _mm512_fmadd_ps(diff[1], diff[1], r_vec);
				r_vec = _mm512_fmadd_ps(diff[2], diff[2], r_vec);
				r_vec = _mm512_rsqrt14_ps(r_vec);
				result = _mm512_mul_ps(tmpQ[m], Q[j]);
				vcps = _mm512_fmadd_ps(result, r_vec, vcps);
			}
			Energy += _mm512_reduce_add_ps(vcps);
		}
	}

	clock_gettime(CLOCK_MONOTONIC, &ts_end);
	time_total = (ts_end.tv_sec - ts_start.tv_sec) * 1e9 + (ts_end.tv_nsec - ts_start.tv_nsec);
	printf("\nTotal time is %f ms, Energy is %.3f\n", time_total / 1e6, Energy*1e-4);
	printf("%i\n", v_count);
}

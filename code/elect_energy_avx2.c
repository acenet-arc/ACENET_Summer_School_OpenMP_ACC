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
	int n = 60;				   /* number of atoms per side */
	int n_charges = n * n * n; /* total number of charges */
	float a = 0.5;			   /* Lattice constant */

	long v_element_count = 0;
	long v_count = 0;

	float tmp_vec[4][8] __attribute__((aligned(32)));
	double Energy = 0.0;

	__m256 *X, *Y, *Z, *Q;
	__m256 tmpQ[8], tmpX[8], tmpY[8], tmpZ[8];
	__m256 r_vec, result, vcps, diff[8], mask[8];

	/* We need an extra block of 8 floats when n_charges is not a multiple of 8 */
	X = aligned_alloc(32, (n_charges) * sizeof(float));
	Y = aligned_alloc(32, (n_charges) * sizeof(float));
	Z = aligned_alloc(32, (n_charges) * sizeof(float));
	Q = aligned_alloc(32, (n_charges) * sizeof(float));

	srand(111); /* Seed random numbers generator */
	/* Initialize X,Y,Z,Q arrays with 256-bit long vectors */
	float tmp_add[8] __attribute__((aligned(32)));

	for (ix = 0; ix < n; ix++)
		for (iy = 0; iy < n; iy++)
			for (iz = 0; iz < n; iz++)
			{
				tmp_vec[0][v_element_count] = ix * a;
				tmp_vec[1][v_element_count] = iy * a;
				tmp_vec[2][v_element_count] = iz * a;
				tmp_vec[3][v_element_count] = 10 * ((double)random() / (double)RAND_MAX - 0.5); /* charges */

				v_element_count++;
				/* when 8 elements are computed pack them into _m256 vectors */
				if (v_element_count == 8)
				{
					X[v_count] = _mm256_set_ps(
						tmp_vec[0][7], tmp_vec[0][6], tmp_vec[0][5], tmp_vec[0][4],
						tmp_vec[0][3], tmp_vec[0][2], tmp_vec[0][1], tmp_vec[0][0]);
					Y[v_count] = _mm256_set_ps(
						tmp_vec[1][7], tmp_vec[1][6], tmp_vec[1][5], tmp_vec[1][4],
						tmp_vec[1][3], tmp_vec[1][2], tmp_vec[1][1], tmp_vec[1][0]);
					Z[v_count] = _mm256_set_ps(
						tmp_vec[2][7], tmp_vec[2][6], tmp_vec[2][5], tmp_vec[2][4],
						tmp_vec[2][3], tmp_vec[2][2], tmp_vec[2][1], tmp_vec[2][0]);
					Q[v_count] = _mm256_set_ps(
						tmp_vec[3][7], tmp_vec[3][6], tmp_vec[3][5], tmp_vec[3][4],
						tmp_vec[3][3], tmp_vec[3][2], tmp_vec[3][1], tmp_vec[3][0]);
					v_count++;
					v_element_count = 0;
					memset(tmp_vec, 0, 32 * sizeof(float));
				}
			}

	/* Treat the remainder. The last vector is padded with zeros */
	if (v_element_count != 0)
	{
		for (float v = n * a * 2; v_element_count < 8; v_element_count++, v += 1)
		{
			tmp_vec[0][v_element_count] = v;
			tmp_vec[1][v_element_count] = 0.0;
			tmp_vec[2][v_element_count] = 0.0;
			tmp_vec[3][v_element_count] = 0.0;
		}
		X[v_count] = _mm256_set_ps(
			tmp_vec[0][7], tmp_vec[0][6], tmp_vec[0][5], tmp_vec[0][4],
			tmp_vec[0][3], tmp_vec[0][2], tmp_vec[0][1], tmp_vec[0][0]);
		Y[v_count] = _mm256_set_ps(
			tmp_vec[1][7], tmp_vec[1][6], tmp_vec[1][5], tmp_vec[1][4],
			tmp_vec[1][3], tmp_vec[1][2], tmp_vec[1][1], tmp_vec[1][0]);
		Z[v_count] = _mm256_set_ps(
			tmp_vec[2][7], tmp_vec[2][6], tmp_vec[2][5], tmp_vec[2][4],
			tmp_vec[2][3], tmp_vec[2][2], tmp_vec[2][1], tmp_vec[2][0]);
		Q[v_count] = _mm256_set_ps(
			tmp_vec[3][7], tmp_vec[3][6], tmp_vec[3][5], tmp_vec[3][4],
			tmp_vec[3][3], tmp_vec[3][2], tmp_vec[3][1], tmp_vec[3][0]);
		v_count++;
	}

	/* mask upper triangular elements */
	mask[0] = (__m256)_mm256_set_epi32(-1, -1, -1, -1, -1, -1, -1, 0);
	mask[1] = (__m256)_mm256_set_epi32(-1, -1, -1, -1, -1, -1, 0, 0);
	mask[2] = (__m256)_mm256_set_epi32(-1, -1, -1, -1, -1, 0, 0, 0);
	mask[3] = (__m256)_mm256_set_epi32(-1, -1, -1, -1, 0, 0, 0, 0);
	mask[4] = (__m256)_mm256_set_epi32(-1, -1, -1, 0, 0, 0, 0, 0);
	mask[5] = (__m256)_mm256_set_epi32(-1, -1, 0, 0, 0, 0, 0, 0);
	mask[6] = (__m256)_mm256_set_epi32(-1, 0, 0, 0, 0, 0, 0, 0);
	mask[7] = (__m256)_mm256_set_epi32(0, 0, 0, 0, 0, 0, 0, 0);

	clock_gettime(CLOCK_MONOTONIC, &ts_start);
#pragma omp parallel for private(tmpQ, tmpX, tmpY, tmpZ, i, j, m, diff, r_vec, vcps, tmp_add, result) reduction(+ \
																												: Energy) schedule(dynamic)

	for (i = 0; i < v_count; i++)
	{

		tmpQ[0] = _mm256_broadcast_ss(&Q[i][0]);
		tmpQ[1] = _mm256_broadcast_ss(&Q[i][1]);
		tmpQ[2] = _mm256_broadcast_ss(&Q[i][2]);
		tmpQ[3] = _mm256_broadcast_ss(&Q[i][3]);
		tmpQ[4] = _mm256_broadcast_ss(&Q[i][4]);
		tmpQ[5] = _mm256_broadcast_ss(&Q[i][5]);
		tmpQ[6] = _mm256_broadcast_ss(&Q[i][6]);
		tmpQ[7] = _mm256_broadcast_ss(&Q[i][7]);

		tmpX[0] = _mm256_broadcast_ss(&X[i][0]);
		tmpX[1] = _mm256_broadcast_ss(&X[i][1]);
		tmpX[2] = _mm256_broadcast_ss(&X[i][2]);
		tmpX[3] = _mm256_broadcast_ss(&X[i][3]);
		tmpX[4] = _mm256_broadcast_ss(&X[i][4]);
		tmpX[5] = _mm256_broadcast_ss(&X[i][5]);
		tmpX[6] = _mm256_broadcast_ss(&X[i][6]);
		tmpX[7] = _mm256_broadcast_ss(&X[i][7]);

		tmpY[0] = _mm256_broadcast_ss(&Y[i][0]);
		tmpY[1] = _mm256_broadcast_ss(&Y[i][1]);
		tmpY[2] = _mm256_broadcast_ss(&Y[i][2]);
		tmpY[3] = _mm256_broadcast_ss(&Y[i][3]);
		tmpY[4] = _mm256_broadcast_ss(&Y[i][4]);
		tmpY[5] = _mm256_broadcast_ss(&Y[i][5]);
		tmpY[6] = _mm256_broadcast_ss(&Y[i][6]);
		tmpY[7] = _mm256_broadcast_ss(&Y[i][7]);

		tmpZ[0] = _mm256_broadcast_ss(&Z[i][0]);
		tmpZ[1] = _mm256_broadcast_ss(&Z[i][1]);
		tmpZ[2] = _mm256_broadcast_ss(&Z[i][2]);
		tmpZ[3] = _mm256_broadcast_ss(&Z[i][3]);
		tmpZ[4] = _mm256_broadcast_ss(&Z[i][4]);
		tmpZ[5] = _mm256_broadcast_ss(&Z[i][5]);
		tmpZ[6] = _mm256_broadcast_ss(&Z[i][6]);
		tmpZ[7] = _mm256_broadcast_ss(&Z[i][7]);

		/* Accumulate coupling between all lower triangular elements of the diagonal 8x8 blocks */
		vcps = _mm256_setzero_ps();
		for (m = 0; m < 8; m++)
		{
			/* dx,dy,dz */
			diff[0] = _mm256_sub_ps(tmpX[m], X[i]);
			diff[1] = _mm256_sub_ps(tmpY[m], Y[i]);
			diff[2] = _mm256_sub_ps(tmpZ[m], Z[i]);
			/* dx*dx + dy*dy + dz*dz */
			r_vec = _mm256_fmadd_ps(diff[0], diff[0], _mm256_setzero_ps());
			r_vec = _mm256_fmadd_ps(diff[1], diff[1], r_vec);
			r_vec = _mm256_fmadd_ps(diff[2], diff[2], r_vec);
			/* distance^-1 */
			r_vec = _mm256_rsqrt_ps(r_vec);
			/* Q[m]*Q[i]*distance^-1 */
			result = _mm256_mul_ps(tmpQ[m], Q[i]);
			result = _mm256_mul_ps(result, r_vec);
			result = _mm256_and_ps(mask[m], result);
			vcps = _mm256_add_ps(vcps, result);
		}
		/* transfer vcps to double precision accumulator VC */
		_mm256_store_ps(tmp_add, vcps);
		Energy += tmp_add[0] + tmp_add[1] + tmp_add[2] + tmp_add[3] + tmp_add[4] + tmp_add[5] + tmp_add[6] + tmp_add[7];

		/* Accumulate coupling between all elemnts of lower triangular 8x8 blocks */
		for (j = i + 1; j < v_count; j++)
		{
			vcps = _mm256_setzero_ps();
			for (m = 0; m < 8; m++)
			{
				diff[0] = _mm256_sub_ps(tmpX[m], X[j]);
				diff[1] = _mm256_sub_ps(tmpY[m], Y[j]);
				diff[2] = _mm256_sub_ps(tmpZ[m], Z[j]);

				r_vec = _mm256_fmadd_ps(diff[0], diff[0], _mm256_setzero_ps());
				r_vec = _mm256_fmadd_ps(diff[1], diff[1], r_vec);
				r_vec = _mm256_fmadd_ps(diff[2], diff[2], r_vec);
				r_vec = _mm256_rsqrt_ps(r_vec);
				result = _mm256_mul_ps(tmpQ[m], Q[j]);
				vcps = _mm256_fmadd_ps(result, r_vec, vcps);
			}
			_mm256_store_ps(tmp_add, vcps);
			Energy += tmp_add[0] + tmp_add[1] + tmp_add[2] + tmp_add[3] + tmp_add[4] + tmp_add[5] + tmp_add[6] + tmp_add[7];
		}
	}

	clock_gettime(CLOCK_MONOTONIC, &ts_end);
	time_total = (ts_end.tv_sec - ts_start.tv_sec) * 1e9 + (ts_end.tv_nsec - ts_start.tv_nsec);
	printf("\nTotal time is %f ms, Energy is %.3f\n", time_total / 1e6, Energy * 1e-4);
	printf("%i\n", v_count);
}

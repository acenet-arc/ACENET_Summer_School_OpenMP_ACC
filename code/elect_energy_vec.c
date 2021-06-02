#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <immintrin.h>
#include <limits.h>


int main(int argc, char **argv) {
		struct timespec ts_start, ts_end;
		int size = 60;
		int n_charges = size*size*size;
		float scale=0.5;
		float *charge, *x, *y, *z;
		int i,j,k;
		float time_total;

		__m256 *X, *Y, *Z, *Q;
		__m256 tmpQ[8], tmpX[8], tmpY[8], tmpZ[8];

		X = aligned_alloc(32, (n_charges+8) * sizeof(float));
		Y = aligned_alloc(32, (n_charges+8) * sizeof(float));
		Z = aligned_alloc(32, (n_charges+8) * sizeof(float));
		Q = aligned_alloc(32, (n_charges+8) * sizeof(float));

		charge=malloc(n_charges*sizeof(float));
		x=malloc(n_charges*sizeof(float));
		y=malloc(n_charges*sizeof(float));
		z=malloc(n_charges*sizeof(float));

		/* initialize x,y,z coordinates and charges */
		int n=0;
		for (i=0; i<size; i++)
				for (j=0; j<size; j++)
						for (k=0; k<size; k++) {
								x[n]=i*scale;
								y[n]=j*scale;
								z[n]=k*scale;
								charge[n]=0.33;
								n++;
						}

		/* Initialize X,Y,Z,Q arrays with 256-bit long vectors */
		int ix,iy,iz;
		float tmp_vec[4][8]  __attribute__ ((aligned (32)));
		long v_element_count = 0;
		long v_count = 0;

		for (ix=0; ix<size; ix++)
				for (iy=0; iy<size; iy++)
						for (iz=0; iz<size; iz++)
						{
								tmp_vec[0][v_element_count] = ix*scale;
								tmp_vec[1][v_element_count] = iy*scale;
								tmp_vec[2][v_element_count] = iz*scale;
								tmp_vec[3][v_element_count] = 0.33;
								v_element_count++;

								if ( v_element_count == 8 ) {
										X[v_count] = _mm256_set_ps( \
												tmp_vec[0][7],tmp_vec[0][6],tmp_vec[0][5],tmp_vec[0][4], \
												tmp_vec[0][3],tmp_vec[0][2],tmp_vec[0][1],tmp_vec[0][0]);
										Y[v_count] = _mm256_set_ps( \
												tmp_vec[1][7],tmp_vec[1][6],tmp_vec[1][5],tmp_vec[1][4], \
												tmp_vec[1][3],tmp_vec[1][2],tmp_vec[1][1],tmp_vec[1][0]);
										Z[v_count] = _mm256_set_ps( \
												tmp_vec[2][7],tmp_vec[2][6],tmp_vec[2][5],tmp_vec[2][4], \
												tmp_vec[2][3],tmp_vec[2][2],tmp_vec[2][1],tmp_vec[2][0]);
										Q[v_count] = _mm256_set_ps( \
												tmp_vec[3][7],tmp_vec[3][6],tmp_vec[3][5],tmp_vec[3][4], \
												tmp_vec[3][3],tmp_vec[3][2],tmp_vec[3][1],tmp_vec[3][0]);
										v_count++;
										v_element_count=0;
										memset(tmp_vec,0,32*sizeof(float));
								}
						}

		/* Treat the remainder. Pad with 0's */
		if ( v_element_count !=0 ) {
				X[v_count] = _mm256_set_ps( \
						tmp_vec[0][7],tmp_vec[0][6],tmp_vec[0][5],tmp_vec[0][4], \
						tmp_vec[0][3],tmp_vec[0][2],tmp_vec[0][1],tmp_vec[0][0]);
				Y[v_count] = _mm256_set_ps( \
						tmp_vec[1][7],tmp_vec[1][6],tmp_vec[1][5],tmp_vec[1][4], \
						tmp_vec[1][3],tmp_vec[1][2],tmp_vec[1][1],tmp_vec[1][0]);
				Z[v_count] = _mm256_set_ps( \
						tmp_vec[2][7],tmp_vec[2][6],tmp_vec[2][5],tmp_vec[2][4], \
						tmp_vec[2][3],tmp_vec[2][2],tmp_vec[2][1],tmp_vec[2][0]);
				Q[v_count] = _mm256_set_ps( \
						tmp_vec[3][7],tmp_vec[3][6],tmp_vec[3][5],tmp_vec[3][4], \
						tmp_vec[3][3],tmp_vec[3][2],tmp_vec[3][1],tmp_vec[3][0]);
				v_count++;
		}

		double VC=0;
		__m256 r_vec, result, vcps, dist_check,R_CUTOFF;
		__m256 diff[8];
		R_CUTOFF = _mm256_set1_ps(scale*scale);

		float tmp_add[4][8] __attribute__ ((aligned (32)));
		int m;
		clock_gettime(CLOCK_MONOTONIC, &ts_start);


		#pragma omp parallel for private(tmpQ,tmpX,tmpY,tmpZ,i,j,m,diff,r_vec,vcps,tmp_add,result,dist_check) reduction(+:VC) schedule(dynamic)

		for(i=0; i<v_count; i++) {
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

				for(m=0; m<8; m++)
				{

						for(j=0; j<v_count; j++)
						{
								vcps  = _mm256_setzero_ps();
								diff[0] = _mm256_sub_ps(tmpX[m],X[j]);
								diff[1] = _mm256_sub_ps(tmpY[m],Y[j]);
								diff[2] = _mm256_sub_ps(tmpZ[m],Z[j]);

								diff[0] = _mm256_mul_ps(diff[0],diff[0]);
								diff[1] = _mm256_mul_ps(diff[1],diff[1]);
								diff[2] = _mm256_mul_ps(diff[2],diff[2]);

								r_vec = _mm256_add_ps(diff[0],diff[1]);
								r_vec = _mm256_add_ps(r_vec,diff[2]);
	              dist_check = _mm256_cmp_ps(r_vec,R_CUTOFF,29);
								r_vec = _mm256_rsqrt_ps(r_vec);


								result = _mm256_mul_ps(tmpQ[m],Q[j]);
								result = _mm256_mul_ps(result,r_vec);
								result = _mm256_and_ps(dist_check,result);

								vcps = _mm256_add_ps(vcps,result); // Accumulate coupling between all voxels
								_mm256_store_ps(tmp_add[0],vcps);
								VC += tmp_add[0][0] + tmp_add[0][1] + tmp_add[0][2] + tmp_add[0][3] + tmp_add[0][4] + tmp_add[0][5] + tmp_add[0][6] + tmp_add[0][7];
						}

				}

		}


		// Calculate electrostatic energy: sum of charge[i]*charge[j]/dist[i,j] */
/*		float dx, dy, dz, dist;
		double Energy=0.0f;
#pragma omp parallel for private(j,dx,dy,dz,dist,n) reduction(+:Energy) schedule(static,50)
		for (i = 0; i < n_charges; i++) {
				for (j = i+1; j < n_charges; j++) {
						dx = x[i]-x[j];
						dy = y[i]-y[j];
						dz = z[i]-z[j];
						dist=sqrt(dx*dx + dy*dy + dz*dz);
						Energy += charge[i]*charge[j]/dist;
				}
		}
*/
		clock_gettime(CLOCK_MONOTONIC, &ts_end);
		time_total = (ts_end.tv_sec - ts_start.tv_sec)*1000000000 + (ts_end.tv_nsec - ts_start.tv_nsec);
		printf("\nTotal time is %f ms, Energy is %f %f\n", time_total/1000000,VC,VC*0.5);
		printf("%i\n", v_count);
}

/* --- File array_max_omp.c --- */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv) {
	struct timespec ts_start, ts_end;
	float time_total;
	int size = 1e7;
	int *rand_nums;
	int i;
	int curr_max;
	time_t t;

	rand_nums=malloc(size*sizeof(int)); 

	/* Intialize random number generator */
	srand((unsigned) time(&t));

	/* Initialize array with random values */
	for (i=0; i<size; i++) {
		rand_nums[i] = rand();
	}

	curr_max = 0.0;

	/* Get start time */
	clock_gettime(CLOCK_MONOTONIC, &ts_start);

#pragma omp parallel for reduction(max:curr_max)
	for (i=0; i<size; i++) {
		if (curr_max < rand_nums[i]) {
			curr_max = rand_nums[i];
		}
	}

	/* Get end time */
	clock_gettime(CLOCK_MONOTONIC, &ts_end);
	time_total = (ts_end.tv_sec - ts_start.tv_sec)*1e9 + \
		     (ts_end.tv_nsec - ts_start.tv_nsec);
	printf("Total time is %f ms\n", time_total/1e6);
	printf("Max value is %d\n", curr_max);
}

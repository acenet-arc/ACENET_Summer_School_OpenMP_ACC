/*  -- File array_multiply.c --- */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv)
{
	struct timespec ts_start, ts_end;
	int size = 1e8;
	int multiplier = 2;
	int *a, *c;
	int i;
	float time_total;

	/* Allocate memory for arrays */
	a = malloc(size * sizeof(int));
	c = malloc(size * sizeof(int));

	/* Get start time */
	clock_gettime(CLOCK_MONOTONIC, &ts_start);

	/* Multiply array a by multiplier */
	for (i = 0; i < size; i++)
	{
		c[i] = multiplier * a[i];
	}

	/* Get end time */
	clock_gettime(CLOCK_MONOTONIC, &ts_end);

	time_total = (ts_end.tv_sec - ts_start.tv_sec) * 1e9 +
				 (ts_end.tv_nsec - ts_start.tv_nsec);
	printf("Total time is %f ms\n", time_total / 1e6);
}

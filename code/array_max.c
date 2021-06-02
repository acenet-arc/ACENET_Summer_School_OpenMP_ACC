/* --- File array_max.c --- */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv)
{
	int size = 10000;
	int *rand_nums;
	int i;
	int curr_max;
	time_t t;

	rand_nums = malloc(size * sizeof(int));

	/* Intialize random number generator */
	srand((unsigned)time(&t));

	/* Initialize array with random values */
	for (i = 0; i < size; i++)
	{
		rand_nums[i] = rand();
	}

	curr_max = 0.0;
	for (i = 0; i < size; i++)
	{
		if (curr_max < rand_nums[i])
		{
			curr_max = rand_nums[i];
		}
	}
	printf("Max value is %d\n", curr_max);
}

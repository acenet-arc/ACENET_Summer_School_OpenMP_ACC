---
title: "Searching through data"
teaching: 20
exercises: 15
questions:
- "How to search in parallel"
objectives:
- "Use general parallel sections"
- "Have a single thread execute code"
- "Use a reduction operator"
keypoints:
- "Reduction operators handle the common case of summation, and analogous operations"
- "OpenMP can manage general parallel sections"
- "You can use 'pragma omp single' to have a single thread execute something"
---

So far, we have looked at parallelizing loops.
- OpenMP also allows you to use general parallel sections ([parallel **Construct**](https://www.openmp.org/spec-html/5.0/openmpse14.html)).

When a thread encounters `#pragma omp parallel` directive OpenMP creates a team of threads. The thread that encountered the `parallel` directive first becomes the master thread of the new team, with a thread number of zero. Parallel region is executed by all of the available threads.

In these cases, it is up to you as a programmer to manage what work gets done by each thread. A basic example would look like the following.

~~~
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char **argv) {
   int id, size;

   #pragma omp parallel private(id, size)
   {
      size = omp_get_num_threads();
      id = omp_get_thread_num();
      printf("This is thread %d of %d\n", id, size);
   }
}
~~~
{:.language-c}

> ## Private variables
> What happens if you forget the `private` keyword?
{: .challenge}

Using this as a starting point, we could use this code to have each available thread do something interesting. For example, we could write the text out to a series of individual files.

## Single threaded function

There are times when you may need to drop out of a parallel section in order to have a single one of the threads executing some code.

- The `#pragma omp single` directive allows us to do this.

A code block associated with the `single` directive will be executed by only the first thread to see it.

More information: [**single Construct**](https://www.openmp.org/spec-html/5.0/openmpsu38.html)

> ## Which thread runs first?
> Modify the following code to print out only the thread that gets to run first in the parallel section.
> You should be able to do it by adding only one line.
> Here's a (rather dense) reference guide in which you can look for ideas:
> [Directives and Constructs for C/C++](https://www.openmp.org/wp-content/uploads/OpenMP-4.5-1115-CPP-web.pdf).
>
> ~~~
> #include <stdio.h>
> #include <stdlib.h>
> #include <omp.h>
>
> int main(int argc, char **argv) {
>    int id, size;
>
>    #pragma omp parallel private(id,size)
>    {
>       size = omp_get_num_threads();
>       id = omp_get_thread_num();
>       printf("This thread %d of %d is first\n", id, size);
>    }
> }
> ~~~
> {:.language-c}
> {: .source}
>
> > ## Solution
> > ~~~
> > #include <stdio.h>
> > #include <stdlib.h>
> > #include <omp.h>
> >
> > int main(int argc, char **argv) {
> >    int id, size;
> >
> >    #pragma omp parallel private(id,size)
> >    {
> >       size = omp_get_num_threads();
> >       id = omp_get_thread_num();
> >       #pragma omp single
> >       printf("This thread %d of %d is first\n", id, size);
> >    }
> > }
> > ~~~
> > {:.language-c}
> {: .solution}
{: .challenge}

## Finding the maximum value in an array

Let's say that we need to search through an array to find the largest value. How could we do this type of search in parallel? Let's begin with the serial version.

~~~
/* --- File array_max.c --- */
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
   int size = 10000;
   float rand_nums[size];
   int i;
   float curr_max;

   /* Initialize with random values */
   for (i=0; i<size; i++) {
      rand_nums[i] = rand();
   }

   /* Find maximum */
   curr_max = 0.0;
   for (i=0; i<size; i++) {
      if (curr_max < rand_nums[i]) {
         curr_max = rand_nums[i];
      }
   }

   printf("Max value is %f\n", curr_max);
}
~~~
{:.language-c}

The first stab would be to make the `for` loop a `parallel for` loop. You would want to make sure that each thread had a private copy of the *curr_max* variable, since it will be written to. But, how do you find out which thread has the largest value?

## Reduction Operators

You could create an array of `curr_max` variables, but getting that to work right would be messy. How do you adapt to different NUM_THREADS?

 The keys here are
 1. To recognize the analogy with the problem of `total` from the last episode, and
 2. To know about `reduction variables`.

 A reduction variable is used to accumulate some value over parallel threads, like a sum, a global maximum, a global minimum, etc.
 The reduction operators that you can use are:
 +, *, -, &, |, ^, &&, ||, max, min.

 ~~~
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
	for (i=0; i<size; i++)
		rand_nums[i] = rand();

	curr_max = 0.0;
	/* Get start time */
	clock_gettime(CLOCK_MONOTONIC, &ts_start);

 #pragma omp parallel for reduction(max:curr_max)
	for (i=0; i<size; i++)
		if (curr_max < rand_nums[i]) {
			curr_max = rand_nums[i];
		}

	/* Get end time */
	clock_gettime(CLOCK_MONOTONIC, &ts_end);
	time_total = (ts_end.tv_sec - ts_start.tv_sec)*1e9 + \
		     (ts_end.tv_nsec - ts_start.tv_nsec);
	printf("Total time is %f ms\n", time_total/1e6);
	printf("Max value is %d\n", curr_max);
 }
 ~~~
 {:.language-c}

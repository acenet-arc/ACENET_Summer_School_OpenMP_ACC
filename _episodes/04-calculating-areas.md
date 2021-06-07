---
title: "Race Conditions with OpenMP"
teaching: 20
exercises: 5
questions:
- "How can we calculate integrals in parallel?"
- "How to compile a program using math functions with gcc?"
objectives:
- "Understand what is a race condition"
- "Learn how to avoid race conditions"
- "Learn how to use *reduction* variables"
keypoints:
- "Race conditions can be avoided by using the *omp critical* or the *omp atomic* directives"
- "The best option to parallelize summation is to use the *reduction* directive"
---
## Introduction
A data race occurs when two threads access the same memory without proper synchronization. This can cause the program to produce incorrect results in parallel mode. As we have learned, loops are parallelized by assigning different loop iterations to different threads. Because loop iterations can run in any order when two threads write to a shared variable in a parallel region the final value of the variable depends on which iteration writes last. In sequential mode, this is always the last iteration, but in parallel mode, this is not guaranteed. 

In this section, we will use two example problems: parallel numerical integration and finding maximum in an array to look at how to control access to global variables.

## Parallel numerical integration
As our example, let's integrate the sine function from 0 to $\pi$. This is the same as the area under the first half of a sine curve. To compute approximation of an integral we will use the simplest Rectangle Method. We will partition area under the curve into a number of very narrow rectangles and add areas of these small shapes together. The single-threaded version is:
~~~
/* --- File integrate_sin.c --- */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char **argv) {
	int steps = 1e7;
	double delta = M_PI/steps;
	double total = 0.0;
	int i;

	printf("Using %.0e steps\n", (float)steps);
	for (i=0; i<steps; i++) {
		total = total + sin(delta*i) * delta;
	}
	printf("The integral of sine from 0 to Pi is %.12f\n", total);
}
~~~
{:.language-c}

### Compiling code with mathematical functions
 To compile a C program that uses math library with GCC we need to explicitly link to it:
 ~~~
 gcc -o integrate integrate_sin_omp.c -lm
 ~~~
 {: .language-bash}

Let's run the program. 
~~~
./integrate
~~~
{: .language-bash}
~~~
Using 1e+07 steps
The integral of sine from 0 to Pi is 2.000000000000
~~~
{:.output}
 
The result in this case should be 2.0, and with 1e7 steps our program computed it accurately. 
To see what happens to the time this program takes, we'll use a new tool. Since
we just want to see the total time, we can use the command *time*:

~~~
srun time -p ./integrate
~~~
{:.language-bash}
~~~
Using 1e+07 steps
The integral of sine from 0 to Pi is 2.000000000000
real 0.94
user 0.94
sys 0.00
~~~
{:.output}
The output *real* is the useful one; this example took 0.941 seconds to run.
The *user* and *sys* lines describe how much time was spent in the "user" code and how much in the "system" code, a distinction that doesn't interest us today.

## Parallelizing Numerical Integration
The program spends most of its time computing areas of small rectangles and adding them together. Let's parallelize the main loop and execute the code.  

The program works, but the result is incorrect when we use more than one thread. What is the problem?  

The data dependency on *total* leads to a race condition. Since we are updating a global variable, there is a race between the various threads as to who can read and then write the value of *total*. Multiple threads could read the current value, before a working thread can write the result of its addition. So these reading threads essentially miss out on some additions to the total.

### How to avoid data race conditions?
One strategy to avoid data race is to synchronize threads to ensure that the variable is accessed in the right order. OpenMP provides a number of ways to ensure that threads are executed in the right order. 

#### The *omp critical* directive
Race conditions can be avoided by adding a *critical* directive. A *critical* directive only allows one thread at a time to run some block of code.
~~~
/* --- File integrate_sin_omp.c --- */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

int main(int argc, char **argv) {
	int steps = 1e4;
	double delta = M_PI/steps;
	double total = 0.0;
	int i;

	printf("Using %.0e steps\n", (float)steps);

#pragma omp parallel for
	for (i=0; i<steps; i++) {
// #pragma omp critical
// #pragma omp atomic
		total += sin(delta*i) * delta;
	}

	printf("The integral of sine from 0 to Pi is %.12f\n", total);
}
~~~
{:.language-c}

The *critical* directive is a very general construct; it can be applied to any arbitrary code block. It lets you ensure it is executed exclusively by one thread. It does it by locking this code block for other threads when it is executed.  The drawback of this directive is poor performance.  The first thread that acquires the lock makes progress, while others sit and wait in line until it has finished. In addition, significant overhead is added every time a thread enters and exits the critical section. And this overhead is on top of the inherent cost of serialization.

Let's add the *omp critical* directive to the statement in the main loop and rerun the code. The addition of the *critical* directive slows down the program relative to the serial version. And if we run it with more threads, it slows down even more. Using *critical*, in this case, is a wrong decision because *critical* serializes the whole loop. 

#### The *omp atomic* directive
Another way to avoid race conditions is to use *omp atomic* directive. The *omp atomic* directive is similar to *critical* but one thread being in an atomic operation doesn't block any other atomic operations about to happen. Where available, *atomic* takes advantage on the CPU instructions providing atomic operations. Depending on the CPU architecture, some CPU instructions such as such as read-modify-write, fetch-and-add, compare-and-swap, ..etc) are atomic. These instructions perform multiple things in memory in a single, atomic step which can not be interrupted. In that case there's no lock/unlock needed on entering/exiting the line of code, it just does the atomic operation, and hardware (or OS) ensures that it is not interfered with. Another advantage of the *omp atomic* directive is much lower overhead.

The downsides are that it can be used only to control a single statement and the set of operations that atomic supports is restricted. Of course, with both *omp critical* and *omp atomic*, you incur the cost of serialization.

Examples demonstrating how to use atomic:
~~~
#pragma omp atomic update
     k += n*mass;  /* update k atomically */

#pragma omp atomic read
     tmp = c;		/* read c atomically */

#pragma omp atomic write
     count = n*m;		/* write count atomically */

#pragma omp atomic capture
{ /* capture original value of v in d, then atomically update v */
		 d = v; 
		 v += n; 
} 
~~~
{:.language-c}

The *atomic* clauses: update (default), write, read, capture

> ## Parallel Performance 
> - Insert the *omp critical* directive, recompile the code and run it on more that one thread. Try different number of threads and record execution time.
> - Repeat with the *omp atomic* directive.
> - How execution time with one thread compares to 2 and 4 threads?
>
> > ## Solution
>>
>> | Version | Threads | Time 
>> |---------|-----------|
>> serial    |  |                          0.94
>> parallelized using *critical*  | 1  |   1.11
>> parallelized using *critical*  | 2  |   1.32 
>> parallelized using *critical*  | 4  |   1.71
>> parallelized using *atomic*    | 1  |   1.03
>> parallelized using *atomic*    | 2  |   0.69
>> parallelized using *atomic*    | 4  |   0.72
>  {: .solution}
{: .challenge}

### The Optimal Way to Parallelize Integration with OpenMP
The best option to parallelize summation is to let each thread to operate on its own chunk of data and when all threads finish add up their sums together.
OpenMP provides a specific thread-safe mechanism to do this: the *reduction* clause. The *reduction* clause lets you specify thread-private variables that are subject to a reduction operation at the end of the parallel region. 

As we are doing summation, the reduction operation we are looking for is "+". At the end of the reduction, the values of all private copies of the shared variable will be added together, and the final result will be written to the global shared variable. 

Let's comment out both the *critical* and the *atomic* directives and add the *reduction* variable *total* subjected to the reductions operator "+" to the parallel *for* loop:

~~~
#pragma omp parallel for reduction(+: total)
~~~
{:.language-c}

Recompile the code and execute. Now we got the right answer, and x3.7 speedup with 4 threads!

In addition to summation OpenMP supports several other reduction operations, such as multiplication, minimum, maximum, logical operators. Next, we will look at other uses of reduction variables.

## Finding the maximum value in an array
Let's say that we need to search through an array to find the largest value. How could we do this type of search in parallel? Let's begin with the serial version.

~~~
/* --- File array_max.c --- */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

int main(int argc, char **argv) {
	int size = 1e8;
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

    /* Find maximum */
	curr_max = 0.0;
	for (i=0; i<size; i++) {
		curr_max = fmax(curr_max, rand_nums[i]);
	}

	printf("Max value is %d\n", curr_max);
}
~~~
{:.language-c}

This problem is analogous to the summation. You would want to make sure that each thread has a private copy of the *curr_max* variable, since it will be written to. When all threads have found the maximum value in their share of data you would want to find out which thread has the largest value. 

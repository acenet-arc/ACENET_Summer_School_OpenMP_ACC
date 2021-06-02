---
title: "Calculating Many-Body Potentials"
teaching: 20
exercises: 10
questions:
- "How do we handle irregular tasks?"
objectives:
- "Learn about the schedule() clause"
keypoints:
- "Different loop scheduling may compensate for unbalanced loop iterations"
---
## Introduction
Calculating the potential energy of many-body systems is a common problem arising in many disciplines. The algorithms for the computation of gravitational potential are used in physics and astronomy. Molecular modeling and simulations need to evaluate electrostatic potential describing interactions between atoms. The algorithms for the calculation of both types of interactions are similar. They both involve calculating the distance matrix - the matrix of distances between all pairs of interacting particles.  Applications from many different fields need distance matrices. For example, dimensionality reduction in machine learning and statistics, wireless sensor network localization, clustering analysis, etc.   

As the following parallelization example, we will consider calculating the total electrostatic potential energy of a set of charges in 3D. As usual, we start with the serial code, parallelize it and see how much faster we can solve this problem in parallel. Recollect that AVX512 CPU cores can apply instructions to 16 floating-point numbers, so the parallel version using ten CPU cores theoretically should run 10x16=160 times faster than a serial code. This speedup is, however, a theoretical maximum assuming negligible parallelization overhead and 100% parallel code. What real-life speedup will we be able to get?

## The algorithm
To compute total electrostatic potential energy we need to sum interactions between all pairs of charges:

$$ E = \sum\frac{charge_i*charge_j}{distance_{i,j}}$$

Let's consider a simple nano crystal particle with a cubic structure. 
- The model has *n* atoms per side
- The lattice constant *a* = 0.5 nm 
- The charges of atoms are randomly assigned with values between -5 and 5 a.u.

Let's start with the following serial code:
~~~
/* --- File elect_energy.c --- */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

int main(int argc, char **argv)
{
	struct timespec ts_start, ts_end;
	float time_total;
	int n = 70;	/* number of atoms per side */
	int n_charges = n * n * n; /* total number of charges */
	float a = 0.5; /* lattice constant a (a=b=c) */
	float *q; /* array of charges */
	float *x, *y, *z; /* x,y,z coordinates */
	float dx, dy, dz, dist; /* temporary  pairwise distances */
	double Energy = 0.0; /* potential energy */
	int i, j, k;

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
				/* Generate random numbers between -5 and 5 */
				q[l] = 10 * ((double)random() / (double)RAND_MAX - 0.5);
				l++;
			}

	/* Calculate sum of all pairwise interactions: q[i]*q[j]/dist[i,j] */
	clock_gettime(CLOCK_MONOTONIC, &ts_start);
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
	time_total = (ts_end.tv_sec - ts_start.tv_sec) * 1e9 + (ts_end.tv_nsec - ts_start.tv_nsec);
	printf("\nTotal time is %f ms, Energy is %.3f\n", time_total / 1e6, Energy * 1e-4);
}
~~~
{:.language-c}

## Performance considerations 

First we need the serial version to use as a reference for calculation of parallel scaling. We will use a couple of new compiler options: 
- turn on optimization reporting *(-qopt-report=1)* 
- ask specifically for vectorization report *(-qopt-report-phase=vec)*.   

At optimization level *-O3* compiler automatically tries to vectorize code. To compile a pure serial version with the same optimization level we can turn off auto vectorization (add *-no-vec*).
~~~
module load StdEnv/2020 intel/2021.2.0
icc -qopt-report=1 -qopt-report-phase=vec -O3 -no-vec elect_energy.c 
srun -c1 ./a.out
~~~
{:.language-bash}

Intel compiler does not print optimization information on screen, it is saved in *\*.optrpt* files. Optimization report saved in the file *elect_energy.optrpt* indicates that our main nested *for* loops computing potential energy at lines 42 and 44 are not vectorized:
~~~
LOOP BEGIN at elect_energy.c(42,2)
   remark #25460: No loop optimizations reported

   LOOP BEGIN at elect_energy.c(44,3)
      remark #25460: No loop optimizations reported
   LOOP END
LOOP END
~~~
{:.output}

This is what we wanted for the reference serial code. Let's run it and record execution time. 
~~~ 
srun -c1 ./a.out
~~~
{:.language-bash}

The runtime of the serial version is about 185/260 sec on the real/training clusters respectively.

Next, recompile the code without *-no-vec* option. The optimization report indicates that the inner *for* loop at line 44 is now vectorized:
~~~
   LOOP BEGIN at elect_energy.c(44,3)
      remark #15300: LOOP WAS VECTORIZED
   LOOP END
~~~
{:.output}

The auto vectorized program runs in about 60/80 sec on the real/training clusters respectively. This is some improvement, but not very impressive. The speedup relative to the serial version is only about 3x. The CPU is able to process 16 floating pointing numbers at once and the theoretical speedup should be 16x, right? Can we do better? 

Use *-march=skylake-avx512* to enforce using avx512 instructions. Now it runs in 50 sec on MC, that is better (5x speedup), but still does not live up to our expectations. Can we do better? 

Yes, if we parallelize explicitly using Intel Intrinsic AVX-512 Instructions. This is not simple, and a learning curve is steep, but it is worth it because you will have full control of the CPU. 

The example of the same program written with AVX-512 Intrinsics is in the file *elect_energy_avx512.c*. The code is well annotated with explanations of everything that is done. The code 250 lines vs 60 lines  

*Note: On the training cluster this program will run only on "c" or "g" flavour nodes because only these flavours have avx512-capable CPUs. These nodes can be requested using 'salloc --nodelist = large-node1'*
~~~
icc -qopt-report=1 -qopt-report-phase=vec -O3 elect_energy_avx512.c
./a.out
~~~
{:.language-bash}
~~~
Total time is 777.719360 ms, Energy is -9.698
21438
~~~
{:.output}

The runtime is 8.5 sec, this is 18.5x speedup!

Compilers are very conservative in automatic vectorization and in general they will use the safest solution. If compiler suspects data dependency, it will not parallelize code. The last thing compiler developers want is for a program to give the wrong results! But you can analyze the code, if needed modify it to eliminate data dependencies and try different parallelization strategies to optimize the performance.


Parallel auto-parallelized 8.7 sec  speedup 21x
Parallel explicit avx512   1.1 sec  speedup 168x


- To parallelize with OpenMP add the line just before the main loop
~~~
#pragma omp parallel for private(j, dx, dy, dz, dist) reduction(+:Energy) schedule(dynamic)
~~~
{:.language-c}

The parallel auto-vectorized version runs in 


## Performance - vectorization?
- GCC automatically vectorizes code at optimization level *-O3*, or with the option *-ftree-vectorize*.
- Intel automatically vectorizes code at optimization level *-O3*. To turn off vectorization use the option *-no-vec*.

### How can we find out if loops were vectorized?
- GNU compiler will print info about unsuccessful optimization:
~~~
gcc -O3 -fopt-info-vec-missed  
~~~
{:.language-bash}
- Intel compiler will save optimization info into a file "*.optrpt" with the following options: 
~~~
icc -qopt-report=1 -qopt-report-phase=vec -O3 
~~~
{:.language-bash}

The code that can be vectorized:
~~~
/* File: vectorize_1.c */
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main()
{
        struct time#spec ts_start, ts_end;
        float time_total;

        int i,j;
        long N=1e8; /* Size of test array */
        float a[N], b[N], c[N];  /* Declare static arrays */

/* Generate some random data */
        for(i=0;i<N;i++) {
                a[i]=random();
                b[i]=random();
        }
        printf("Test arrays generated\n");

/* Run test 3 times, compute average execution time */
        float average_time=0.0f;
        for(int k=0;k<3;k++)
        {
        clock_gettime(CLOCK_MONOTONIC, &ts_start);

          for(int j=0;j<50;j++)
            for(i=0;i<N;i++)
                c[i]=a[i]*b[i]+j;


        clock_gettime(CLOCK_MONOTONIC, &ts_end);
        time_total = (ts_end.tv_sec - ts_start.tv_sec)*1e9 + \
                     (ts_end.tv_nsec - ts_start.tv_nsec);
        printf("Total time is %f ms\n", time_total/1e6);
        average_time+=time_total;
        }
printf("Average time is %f ms\n", average_time/3e6);
}
~~~
{:.language-c}


- Compiler does not even look in nested loops ?
- Compiler does not vectorize dynamically allocated arrays -??

Now comes the parallelization.

> ## Parallelize the energy code
> 1. Decide what variable or variables should be made private, and then compile and test the code.
> 2. Run on few different numbers of CPUs. How does the performance scale?
>
> > ## Solution
> > ~~~
> > /* --- File elect_energy_omp.c --- */
> > #include <stdio.h>
> > #include <stdlib.h>
> > #include <time.h>
> > #include <math.h>
> >
> > int main(int argc, char **argv) {
> > 	struct timespec ts_start, ts_end;
> > 	int size = 30;
> > 	int n_charges = size*size*size;
> > 	float scale=0.5;
> > 	float *charge, *x, *y, *z;
> > 	float **M;
> > 	int i,j,k;
> > 	float time_total;
> >
> > 	charge=malloc(n_charges*sizeof(float));
> > 	x=malloc(n_charges*sizeof(float));
> > 	y=malloc(n_charges*sizeof(float));
> > 	z=malloc(n_charges*sizeof(float));
> > 	M=(float**)malloc(n_charges*sizeof(float*));
> > 	for (i=0;i<n_charges;i++)
> > 		M[i]=malloc(n_charges*sizeof(float));
> >
> > /* initialize x,y,z coordinates and charges */
> > 	int n=0;
> > 	for (i=0; i<size; i++)
> > 		for (j=0; j<size; j++)
> > 			for (k=0; k<size; k++) {
> > 				x[n]=i*scale;
> > 				y[n]=j*scale;
> > 				z[n]=k*scale;
> > 				charge[n]=0.33;
> > 				n++;
> > 			}
> > 	clock_gettime(CLOCK_MONOTONIC, &ts_start);
> >
> > 	// Calculate electrostatic energy: sum of charge[i]*charge[j]/dist[i,j] */
> > 	float dx, dy, dz, dist;
> > 	double Energy=0.0f;
> > #pragma omp parallel for private(j,dx,dy,dz,dist) reduction(+:Energy) schedule(static,50)
> > 	for (i = 0; i < n_charges; i++) {
> > 		for (j = i+1; j < n_charges; j++) {
> > 			dx = x[i]-x[j];
> > 			dy = y[i]-y[j];
> > 			dz = z[i]-z[j];
> > 			dist=sqrt(dx*dx + dy*dy + dz*dz);
> > 			Energy += charge[i]*charge[j]/dist;
> > 		}
> > 	}
> >
> > 	clock_gettime(CLOCK_MONOTONIC, &ts_end);
> > 	time_total = (ts_end.tv_sec - ts_start.tv_sec)*1e9 + (ts_end.tv_nsec - ts_start.tv_nsec);
> > 	printf("\nTotal time is %f ms, Energy is %f\n", time_total/1e6, Energy);
> > }
> > ~~~
> > {:.language-c}
> {: .solution}
{: .challenge}

## The schedule() clause

OpenMP loop directives (`parallel for, parallel do`) can take several other clauses besides the `private()` clause we've already seen. One is `schedule()`, which allows us to specify how loop iterations are divided up among the
threads.

The default is *static* scheduling, in which all iterations are allocated to threads before they execute any loop iterations.

In *dynamic* scheduling, only some of the iterations are allocated to threads at the beginning of the loop's execution. Threads that complete their iterations are then eligible to get additional work. The allocation process continues until all the iterations have been distributed to threads.

There's a tradeoff between overhead (i.e., how much time is spent setting up the schedule) and load balancing (i.e., how much time is spent waiting for the most heavily-worked thread to catch up). Static scheduling has low overhead but
may be badly balanced; dynamic scheduling has higher overhead.

Both scheduling types also take a *chunk size*; larger chunks mean less overhead and greater memory locality, smaller chunks may mean finer load balancing. You can omit the chunk size, it defaults to 1 for *dynamic* schedule and to $N_{iterrations}/{N_{threads}}$ for *static* schedule.

Bad load balancing might be what's causing this code not to parallelize very well. As we are computing triangular part of the interaction matrix *static* scheduling with the default *chunk size* will lead to uneven load.

Let's add a `schedule(dynamic)` or `schedule(static,100)` clause and see what happens.

> ## Play with the schedule() clause
>
> Try different `schedule()` clauses and tabulate the run times with different thread numbers. What seems to work best for this problem?
>
> Does it change much if you grow the problem size? That is, if you make `size` bigger?
>
> There's a third option, `guided`, which starts with large chunks and gradually decreases the chunk size as it works through the iterations.
> Try it out too, if you like. With `schedule(guided,<chunk>)`, the chunk parameter is the smallest chunk size it will try.
{: .challenge}

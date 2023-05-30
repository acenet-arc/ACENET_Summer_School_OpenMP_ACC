---
title: "Calculating Many-Body Potentials"
teaching: 20
exercises: 5
questions:
- "How to speed up calculation leveraging both OpenMP and SIMD instructions"
objectives:
- "Learn using automatic vectorization in Intel and GCC compilers"
- "Learn about the *schedule* clause"
keypoints:
- "Writing vectorization-friendly code can take additional time but is mostly worth the effort"
- "Different loop scheduling may compensate for unbalanced loop iterations"
---

Calculating the potential energy of many-body systems is a common problem arising in many disciplines. The algorithms for the computation of gravitational potential are used in physics and astronomy. Molecular modeling and simulations need to evaluate electrostatic potential describing interactions between atoms. The algorithms for the calculation of both types of interactions are similar. They both involve calculating the distance matrix - the matrix of distances between all pairs of interacting particles.  Applications from many different fields need distance matrices. For example, dimensionality reduction in machine learning and statistics, wireless sensor network localization, clustering analysis, etc.   
{:.instructor_notes}

As the following parallelization example, we will consider calculating the total electrostatic potential energy of a set of charges in 3D. As usual, we start with the serial code, parallelize it and see how much faster we can solve this problem in parallel. Recollect that AVX512 CPU cores can apply instructions to 16 floating-point numbers, so the parallel version using ten CPU cores theoretically should run 10x16=160 times faster than a serial code. This speedup is, however, a theoretical maximum assuming negligible parallelization overhead and 100% parallel code. What real-life speedup will we be able to get?
{:.instructor_notes}

## The Algorithm
To compute total electrostatic potential energy we need to sum interactions between all pairs of charges:

$$ E = \sum\frac{charge_i*charge_j}{distance_{i,j}}$$

Let's consider a simple nano crystal particle with a cubic structure. 
- The model has *n* atoms per side
- The lattice constant *a* = 0.5 nm 
- The charges of atoms are randomly assigned with values between -5 and 5 a.u.

Let's start with the following serial code:

<div class="gitfile" markdown="1">

~~~
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

int main(int argc, char **argv)
{
	struct timespec ts_start, ts_end;
	float time_total;
	int n = 60;	/* number of atoms per side */
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
[elect_energy_template.c](https://github.com/ssvassiliev/ACENET_Summer_School_OpenMP_2023/raw/gh-pages/code/elect_energy_template.c)
</div>

Our goal is to speed up calculation leveraging OpenMP and SIMD instructions (vectorization).
{:.self_study_text}

Our goal is to speed up calculation leveraging both levels of parallelism available in modern CPUs: OpenMP to execute the code concurrently on several CPU cores and SIMD instructions enabling each CPU core to operate on multiple data elements simultaneously (vectorization). 
{:.instructor_notes}

Writing vectorized code can take additional time but is mostly worth the effort, because the performance increase may be substantial. Modern C and Fortran compilers are capable to automatically generate SIMD instructions. However, not all code can be auto-vectorized as there are many potential obstacles for auto-vectorization. The discussing detailed guidelines to writing vectorization-friendly code are outside the scope of this workshop. In this session we will briefly look at the performance benefits of vectorization learn how to use auto-vectorization compiler feature, and how to get information about details of auto-vectorization results. 
{:.instructor_notes}

## Performance Considerations 
### Serial Performance
First we need to compile the serial version to use as a reference for calculation of parallel scaling. For this section we will use Intel compiler. 

#### Using the Auto-vectorizer in Intel Compilers

At optimization levels *-O2* or higher Intel compiler automatically tries to vectorize code. To compile an unvectorized version with the same optimization level we need to turn off auto vectorization.

- disable auto-vectorizer *(-no-vec)*

How do we know if compiler was able to vectorize any loops?

For this we will use a couple of new compiler options: 
- turn on optimization reporting *(-qopt-report=1)* 
- turn on vectorization report *(-qopt-report-phase=vec)*. 

~~~
module load intel/2022.1.0
icc -qopt-report=1 -qopt-report-phase=vec -O3 -no-vec elect_energy_template.c 
srun -c1 ./a.out
~~~
{:.language-bash}

Intel compiler does not print optimization information on screen, it creates optimization report *\*.optrpt* files. 

Optimization report saved in the file *elect_energy.optrpt* indicates that our main nested *for* loops computing potential energy at lines 42 and 44 are not vectorized:
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

The runtime of the serial version is about 80 sec on a real cluster with Skylake CPUs. It will run somewhat slower on the training cluster.

>## Using the auto-vectorizer in GCC
>- auto-vectorizer is enabled by default at optimization level *-O3*
>- auto-vectorizer can be enabled with the option *-ftree-vectorize* and disabled with *-fno-tree-vectorize*
>- print info about unsuccessful optimization *-fopt-info-vec-missed*
>- print info about optimized loops *-fopt-info-vec-optimized*
{:.callout}


### Parallel Performance
#### Using Automatic Vectorization
Next we will use use the auto-vectorizer in the compiler. This means do nothing except ensuring that the coding will not prevent vectorization. Many things, can prevent automatic vectorization or produce suboptimal vectorized code. 
{:.instructor_notes}

- Recompile the code without *-no-vec* option. 

The optimization report indicates that the inner *for* loop at line 44 is now vectorized:
~~~
   LOOP BEGIN at elect_energy.c(44,3)
      remark #15300: LOOP WAS VECTORIZED
   LOOP END
~~~
{:.output}

The auto vectorized program runs in about 26 sec on the real cluster. This is some improvement, but not very impressive. The speedup relative to the serial version is only about 3x. Skylake CPUs are able to process 16 floating pointing numbers simultaneously, so theoretical speedup should be 16x, right? Can we do better? 

- By default, the compiler uses avx2 instructions.
- Enforce using avx512 instructions: *-march=skylake*. 

On the training cluster this version will run only on "c" or "g" flavour nodes because only these flavours have Skylake CPUs supporting AVX-512. 

- AVX-512 nodes can be requested using the *--nodelist* option, e.g.
~~~
srun --nodelist=large-node1 ./a.out
~~~
{:.language-bash}

Now the program runs in 15.7 sec (on the real cluster), this is better (5x speedup), but still does not live up to our expectations. Can we do better? 

- Yes, if we parallelize explicitly using Intrinsic AVX Functions. 

#### Using Intel's Advanced Vector Extensions (AVX) Intrinsic Functions.
This is not too complicated, and worth doing because you will be able to decide how you data is organized, moved in and out of vector processing units and what AVX instructions are used for calculations. With Intrinsic Functions, you are fully responsible for design and optimization of the algorithm, compiler will follow your commands.
{:.instructor_notes}

The examples of the same program written with AVX2 and AVX-512 Intrinsics are in the files *elect_energy_avx2.c* and *elect_energy_avx512.c*, respectively. The code is well annotated with explanations of all statements. 

To summarize the vectorized algorithm:
- allocate memory for _m512 vectors, each one hold 16 floating point values
- memory must be aligned with 64 byte margins for optimal transfer 
- pack data into _m512 vectors
- accumulate interactions looping over 16x16 tiles 

Compile the code and run it.
~~~
icc -qopt-report=1 -qopt-report-phase=vec -O3 elect_energy_avx2.c
srun ./a.out
~~~
{:.language-bash}

On the real cluster the code with explicit AVX2 instructions is twice faster than the auto-vectorized AVX-512 version,so this is 10x faster than the serial code. 

But we can do even better. Try the AVX-512 version.  AVX-512 registers are twice wider than AVX2; they can operate on 16 floating point numbers.

Compile the code and run it.
~~~
icc -qopt-report=1 -qopt-report-phase=vec -O3 elect_energy_avx512.c
srun ./a.out
~~~
{:.language-bash}

The benchmark on all versions on the real cluster is below. As you can see,  AVX-512 version is 1.7x faster than AVX2. Adapting AVX2 to AVX-512 is very straightforward. 

#### Benchmark n=60, Single Thread, Cluster Siku

 Version          | Time |  Speedup
------------------|------|
Serial            | 81.2 | 1.0
auto-vec          | 26.1 | 3.1     
auto-vec, skylake | 15.7 | 5.2
avx2              | 7.7  | 10.5
avx512            | 4.58 | 17.7 

Compilers are very conservative in automatic vectorization and in general they will use the safest solution. If compiler suspects data dependency, it will not parallelize code. The last thing compiler developers want is for a program to give the wrong results! But you can analyze the code, if needed modify it to eliminate data dependencies and try different parallelization strategies to optimize the performance.
{:.instructor_notes}

#### What Loops can be Vectorized?
- The number of iterations must be known (at runtime, not at compilation time) and remain constant for the duration of the loop. 
- The loop must have single entry and single exit. For example no data-dependent exit.
- The loop must have straight-line code. 

Because SIMD instructions perform the same operation on data elements it is not possible to switch instructions for different iterations. The exception is *if* statements that can be implemented as masks. For example the calculation is performed for all data elements, but the result is stored only for those elements for which the mask evaluates to true.
{:.instructor_notes}

- No function calls are allowed. Only intrinsic vectorized math functions or functions that can be inlined are allowed.
 
### Adding OpenMP Parallelization 

The vectorized program can run in parallel on each of the cores of modern multicore CPUs, so the maximum theoretical speedup should be proportional to the numbers of threads. 

> ## Parallelize the Code
> 1. Decide what variable or variables should be made private, and then compile and test the code.
> 2. Run on few different numbers of CPUs. How does the performance scale?
>
> > ## Solution
> > Add the following OpenMP *pragma* just before the main loop
> > ~~~
> > #pragma omp parallel for private(j, dx, dy, dz, dist) reduction(+:Energy) schedule(dynamic)
> > ~~~
> > {:.language-c}
> {: .solution}
{: .challenge}

The benchmark on all versions on the real cluster is below. The speedup of the OpenMP version matches our expectations.

#### Benchmark n=60, OpenMP 10 Threads, Cluster Siku.

Version            | Time   |  Speedup
-------------------|--------|
not vectorized     | 7.8    | 10.4
auto-vect          | 2.5    | 32.5     
auto-vect, skylake | 1.43   | 56.8
avx2               | 0.764  | 106.3
avx512             | 0.458  | 177.3

### OpenMP Scheduling

- In *static* scheduling all iterations are allocated to threads before they execute any loop iterations.
- In *dynamic* scheduling, OpenMP assigns one iteration to each thread. Threads that complete their iteration will be assigned the next iteration that hasn’t been executed yet. 
{:.self_study_text}

OpenMP automatically partitions the iterations of a *parallel for* loop. By default it divides all iterations in a number of chunks equal to the number of threads. The number of iterations in each chunk is the same, and each thread is getting one chunk to execute. This is *static* scheduling, in which all iterations are allocated to threads before they execute any loop iterations. However, a *static* schedule can be non-optimal. This is the case when the different iterations need different amounts of time. This is true for our program computing potential. As we are computing triangular part of the interaction matrix *static* scheduling with the default *chunk size* will lead to uneven load.
{:.instructor_notes}

This program can be improved with a *dynamic* schedule. In *dynamic* scheduling, OpenMP assigns one iteration to each thread. Threads that complete their iteration will be assigned the next iteration that hasn’t been executed yet. The allocation process continues until all the iterations have been distributed to threads.
{:.instructor_notes}

- If *dynamic* scheduling balances load why would we want to use *static* scheduling at all? 

The problem is that here is some overhead to *dynamic* scheduling. After each iteration, the threads must stop and receive a new value of the loop variable to use for its next iteration. Thus, there's a tradeoff between overhead and load balancing.
{:.instructor_notes}

- Static scheduling has low overhead but may be badly balanced
- Dynamic scheduling has higher overhead.

Both scheduling types also take a *chunk size* argument; larger chunks mean less overhead and greater memory locality, smaller chunks may mean finer load balancing. Chunk size defaults to 1 for *dynamic* schedule and to $\frac{N_{iterrations}}{N_{threads}}$ for *static* schedule.

Let's add a `schedule(dynamic)` or `schedule(static,100)` clause and see what happens.

> ## Experiment with Scheduling
> 1. Try different scheduling types. How does it affect the performance? What seems to work best for this problem? Can you suggest the reason? 
> 2. Does the optimal scheduling change much if you grow the problem size? That is, if you make *n* bigger?
> 3. There's a third scheduling type, *guided*, which starts with large chunks and gradually decreases the chunk size as it works through the iterations. Try it out too, if you like. With  the *guided* scheduling, the chunk parameter is the smallest chunk size OpenMP will try.
>
>> ## Solution
>> We are computing triangular part of the interaction matrix. The range of pairwise interactions computed by a thread will vary from *1* to *(n_charges - 1)*. Therefore, *static* scheduling with the default *chunk size* will lead to uneven load.
>{:.solution}
>
{: .challenge}


> ## Example of the Code that can be Vectorized:
>
> ~~~
> /* File: vectorize_1.c */
> #include <stdlib.h>
> #include <stdio.h>
> #include <time.h>
> 
> int main()
> {
>         struct timespec ts_start, ts_end;
>         float time_total;
> 
>         int i,j;
>         long N=1e8; /* Size of test array */
>         float a[N], b[N], c[N];  /* Declare static arrays */
> 
> /* Generate some random data */
>         for(i=0;i<N;i++) {
>                 a[i]=random();
>                 b[i]=random();
>         }
>         printf("Test arrays generated\n");
> 
> /* Run test 3 times, compute average execution time */
>         float average_time=0.0f;
>         for(int k=0;k<3;k++)
>         {
>         clock_gettime(CLOCK_MONOTONIC, &ts_start);
> 
>           for(int j=0;j<50;j++)
>             for(i=0;i<N;i++)
>                 c[i]=a[i]*b[i]+j;
> 
> 
>         clock_gettime(CLOCK_MONOTONIC, &ts_end);
>         time_total = (ts_end.tv_sec - ts_start.tv_sec)*1e9 + \
>                      (ts_end.tv_nsec - ts_start.tv_nsec);
>         printf("Total time is %f ms\n", time_total/1e6);
>         average_time+=time_total;
>         }
> printf("Average time is %f ms\n", average_time/3e6);
> }
> ~~~
> {:.language-c}
>
{:.callout}





---
title: "OpenMP Work Sharing Constructs"
teaching: 20
exercises: 10
questions:
- During parallel code execution, how is the work distributed between threads?
objectives:
- Learn about the OpenMP constructs for worksharing.
keypoints:
- Data parallelism refers to the execution of the same task simultaneously on multiple computing cores.
- Functional parallelism refers to the concurrent execution of different tasks on multiple computing cores.
---

The general parallel sections allow for explicit workload distribution across threads. Even though this feature can be useful, it does not always save programmers as much time and effort as they would like. Parallelizing tasks such as iterating over a sequence would involve a lot of coding with general parallel sections. The programmer would have to determine which threads are available and to configure each thread to process a fraction of the entire data set. 
{: .instructor_notes}

OpenMP offers several types of work sharing constructs that assist in automating and simplifying parallelization. Work-sharing constructs distribute the execution of parallel code between members of the threads team.
{: .instructor_notes}

- OpenMP offers several types of work sharing constructs that assist in parallelization. 
{: .self_study_text}

- The work-sharing constructs do not create new threads, instead they use a team of threads created by the `parallel` directive.
- At the end of a work-sharing construct, a program will wait for all threads to complete. This behavior is called an *implied barrier*.

### The *omp for*
- The *omp for* construct divides loop iterations across the thread team.
- It must be used after the parallel region has been initiated.  Otherwise, the program will run serially on a single processor.
- Each thread executes the same instructions. 
- The *omp for* represents a type of parallelism known as *data parallelism*.

~~~
...
#pragma omp parallel for
    for (i=0; i < N; i++)
        c[i] = a[i] + b[i];
...
~~~
{: .language-c}

> ## Using arrays in C
> Declaring arrays as static is as simple as this:
> ~~~
>  A[2048][2048];
> ~~~
>{: .language-c}
>Using static arrays has its disadvantages. A global static array is allocated when a program starts, and it occupies memory until it ends. It is thus impossible to release memory occupied by a static array when it is no longer needed. This can lead to your program crashing with a "Segmentation fault" error if large arrays are declared static. 
>
>Another issue is that local static arrays are allocated on the stack. The stack is like a first-in, last-out data buffer, allowing system memory to be used as temporary storage. In order to prevent programs from using excessive amounts of stack space, the OS usually imposes a limit. 
>- The stack memory limit can be checked with the command:  
>~~~
>ulimit -s
>~~~
>{:.language-bash}
>- If you want to modify the stack memory limit, you can use the ulimit command:
>~~~
>ulimit -s unlimited
>~~~
>{:.language-bash}
>- On compute nodes stack is unlimited
>
>~~~
>/* --- File test_stack.c --- */
>#include <stdio.h>
>#include <stdlib.h>
>
>#define N_BYTES 10000000 // 10 MB
>
>void test_heap(int N) {
>  char *B;
>  B=malloc(N*sizeof(char)); // Allocating on heap
>  printf("Allocation of %i bytes on heap succeeded\n", N);
>  free(B);
>}
>
>void test_stack(int N) {
>  char A[N]; // Allocating on stack
>  printf("Allocation of %i on stack succeeded\n", N);
>}
>
>int main(int argc, char **argv) {
>  test_heap(N_BYTES);
>  test_stack(N_BYTES);
>}
>~~~
>{: .language-c}
{: .callout}

### The *omp sections*
- The *omp sections* construct breaks work into separate, discrete sections.
- It is is a non-iterative work-sharing construct.
- It specifies that the enclosed sections of code are to be divided among the threads in the team. Independent *section* directives are nested within a *sections* directive. Each *section* contains different instructions and is executed once by a thread.
- It is possible for a thread to execute more than one *section*.
- Sections can be used to implement a *functional parallelism* (concurrent execution of different tasks).

~~~
...
#pragma omp parallel shared(a,b,c,d) private(i)
  {
#pragma omp sections nowait
    {
#pragma omp section
    for (i=0; i < N; i++)
      c[i] = a[i] + b[i];
#pragma omp section
    for (i=0; i < N; i++)
      d[i] = a[i] * b[i];
    }  /* end of sections */
  }  /* end of parallel region */
...
~~~
{: .language-c}

Here *nowait* clause means that the program will not wait at the end of the *sections* block for all threads to finish.

> ## Exercise
> Compile the file *sections.c* and run it on a different number of CPUs. 
> Start with 1 CPU:
>~~~
> srun -c1 ./a.out
>~~~
>{:.language-bash}
>
This example has two sections and the program prints out which threads are doing them.
> - What happens if the number of threads and the number of *sections* are different?
> - More threads than *sections*?
> - Less threads than sections?
>
> > ## Solution
> > If there are more threads than sections, only some threads will execute a section.  If there are more sections than threads, the implementation defines how the extra sections are executed.
> {: .solution}
{: .challenge}


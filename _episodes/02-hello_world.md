---
title: "Hello World"
teaching: 20
exercises: 10
questions:
- How do you compile and run an OpenMP program?
- What are OpenMP pragmas?
- How to identify threads?
objectives:
- Write, compile and run a multi-threaded program where each thread prints “hello world”.
keypoints:
- "Pragmas are directives to the compiler to parallelize something"
- "Thread number is typically controlled with an environment variable, OMP_NUM_THREADS"
- "Order of execution of parallel elements is not guaranteed."
- "If the compiler doesn't recognize OpenMP pragmas, it will compile a single-threaded program.  But you may need to escape OpenMP function calls."
---

## Adding parallelism to a program
Since OpenMP is an extension to the compiler, you need to be able to tell the compiler when and where to add the code necessary to create and use threads for the parallel sections. This is handled through special statements called pragmas. To a compiler that doesn't understand OpenMP, pragmas look like comments. The basic forms for C/C++ and Fortran are:

~~~
#pragma omp < OpenMP directive >
~~~
{: .language-c}

~~~
!$OMP < OpenMP directive >
~~~
{: .language-fortran}

In C all OpenMP - specific directives start with `#pragma omp`.

How do we add in parallelism to the basic hello world program?

OpenMP is a library of functions and macros, so we need to include a header file *omp.h* with prototypes and macro definitions.

The very first directive that we will look at is the `parallel` directive. The `parallel` directive forks threads to carry out the work given in the `parallel` block of code.
~~~
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char **argv) {

#pragma omp parallel
   printf("Hello World\n");
}
~~~
{: .language-c}

To compile it, you'll need to add an extra flag to tell the compiler to treat the source code as an OpenMP program.

~~~
gcc -fopenmp -o hello hello.c
~~~
{: .language-bash}

If you prefer Intel compilers to GCC, use:
~~~
icc -qopenmp -o hello hello.c
~~~
{: .language-bash}

**NOTE:** The default compilers in StdEnv/2020 are gcc/9.3.0 and Intel/2020.1.217. Other compilers can be used by loading their respective modules. For example, to load gcc/10:
~~~
module load gcc/10.2.0
~~~
{: .language-bash}

When you run this program, you should see the output "Hello World" multiple
times. But how many?

The OpenMP standard says this is implementation dependent. But the usual default is that OpenMP will look at the machine it is running on and see how many cores there are. It will then launch a thread for each core.

You can control the number of threads with environment variable OMP_NUM_THREADS. For example, if you want only 3 threads, do the following:

~~~
export OMP_NUM_THREADS=3
./hello
~~~
{: .language-bash}

### Execution steps of the parallel "Hello, world" program
- The *parallel* pragma directs compiler to make a code that will start a number of threads equal to what was passed to the program via the variable OMP_NUM_THREADS 
- Each thread then executes the function *printf("Hello World\n")*
- The threads rejoin the main thread when they return
from the *printf()* function, at which point they are terminated 
- The main thread is then terminated itself

> ## Using multiple cores
> Try running the "hello" program with different numbers of threads.
> - Can you use more threads than the cores on the machine?  
>
> You can use *nproc* command to find out how many cores are on the machine.
>
>>## Solution
>> Threads are an OS abstraction and have no direct relationship to cores. You can launch as many threads as you want (the maximum number of threads can be limited by OS and/or OpenMP implementation), however the performance may degrade if you use more threads than physical cores.
> {: .solution} 
{: .challenge}

> ## OpenMP with SLURM
> When you wish to submit an OpenMP job to the job scheduler SLURM, you can use the following boilerplate.
> ~~~
> #!/bin/bash
> #SBATCH --account=sponsor0
> #SBATCH --time=0:01:0
> #SBATCH --cpus-per-task=3
> export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
> ./hello
> ~~~
> {: .language-bash}
>
> You could also ask for an interactive session with multiple cores like so:
> ~~~
> [user45@login1 ~]$ salloc --mem-per-cpu=1000 --cpus-per-task=3 --time=1:0:0
> ~~~
> {: .language-bash}
> ~~~
>salloc: Granted job allocation 179
>salloc: Waiting for resource configuration
>salloc: Nodes node1 are ready for job
>[user45@node1 ~]$ 
> ~~~
> {: .output}
>  The most practical way to run our short parallel program on our test cluster is using *srun* command. Instead of submitting the job to the queue  *srun* will run the program from the interactive shell as soon as requested resources will become available. After the job is finished slurm will release the allocated resources and exit. *Srun* understands the same keywords as *sbatch* and *salloc*.
>
> In SLURM environment operating system will see as many CPUs as you requested, so strictly there is no need to set OMP_NUM_THREADS variable to $SLURM_CPUS_PER_TASK.  
>
> ~~~
> srun --cpus-per-task=4 hello
> # or even shorter:
> srun -c4 hello
> ~~~
> {: .language-bash}
{: .callout}

## Identifying threads

> ## Download and Unpack the Code.
> If you have not yet done so, download and unpack the code:
> ~~~
> cd scratch
> wget https://github.com/ssvassiliev/Summer_School_OpenMP/raw/master/code/omp.tar.gz
> tar -xf omp.tar.gz
> cd code
> ~~~
> {: .language-bash}
{: .callout}

How can we tell which thread is doing what?   
The OpenMP specification includes a number of functions that are made available through the included header file "omp.h". One of them is the function "omp_get_thread_num( )", used to get an ID of the thread running the code.

~~~
/* --- File hello_world_omp.c --- */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char **argv) {
   int id;

#pragma omp parallel
   {
     id = omp_get_thread_num();
     printf("Hello World from thread %d\n", id);
   }
}
~~~
{: .language-c}

Here, you will get each thread tagging their output with their unique ID, a number between 0 and (number of threads - 1).

> ## Pragmas and code blocks in FORTRAN
> An OpenMP directive applies to the *code block* following it in C or C++. Code blocks are either a single line, or a series of lines wrapped by curly brackets.
>
> Because Fortran doesn't have an analogous construction, many OpenMP directives in Fortran are paired with the matching "end" directive, such as `!$omp parallel end`.
{: .callout}

> ## Thread ordering
> Try running the program a few times.
> - What order do the threads write out their messages in?
> - What's going on?
>
> > ## Solution
> > The messages are emitted in random order. This is an important rule of not only OpenMP programming, but parallel programming in general: parallel elements are scheduled to run by the operating system and order of their execution is not guaranteed.
> {: .solution}
{: .challenge}

> ## Conditional Compilation
> We said earlier that you should be able to use the same code for both OpenMP and serial work. Try compiling the code without the -fopenmp flag.
> - What happens?
> - Can you figure out how to fix it?
>
> Hint: If compiler is called with the OpenMP option it defines preprocessor macro \_OPENMP, so you can use `#ifdef _OPENMP` and `#endif` preprocessor directives to tell compiler to process the line calling the *omp_get_thread_num()* function only if this macro is defined.
> > ## Solution
> > ~~~
> >
> > #include <stdio.h>
> > #include <stdlib.h>
> > #include <omp.h>
> >
> > int main(int argc, char **argv) {
> >    int id = 0;
> >    #pragma omp parallel
> >    {
> > #ifdef _OPENMP
> >    id = omp_get_thread_num();
> > #endif
> >    printf("Hello World from thread %d\n", id);
> >    }
> > }
> > ~~~
> > {: .language-c}
> {: .solution}
{: .challenge}


### Work-Sharing Constructs

A work-sharing construct divides the execution of the enclosed code region among the members of the thread team that encounter it.

- Work-sharing constructs do not launch new threads
- A program will wait for all threads to finish at the end of a work sharing construct. This behaviour is called "implied barrier".

#### *For*
- ***For*** construct divides iterations of a loop across the team of threads.
- Each thread executes the same instructions. This assumes a parallel region has already been initiated, otherwise it executes in serial on a single processor.
- *For* represents a type of *data parallelism*.

~~~
...
#pragma omp parallel for
    for (i=0; i < N; i++)
        c[i] = a[i] + b[i];
...
~~~
{: .language-c}

> ## Stack Overflow
> The easiest way to declare arrays as static: 
> ~~~
>  A[2048][2048];
> ~~~
>{: .language-c}
>Globally defined static arrays are allocated when a program starts, and they occupy memory until a program ends. If you declare large arrays as static, your program may crash with a "Segmentation fault" error. Static arrays are allocated on the stack, and the OS limits the size of the stack memory available to a user. You can check your stack memory limit using the command:
>~~~
>ulimit -s
>~~~
>{:.language-c}
{: .callout}

#### *Sections*
- ***Sections*** construct breaks work into separate, discrete sections.
- It is is a non-iterative work-sharing construct.
- It specifies that the enclosed section(s) of code are to be divided among the threads in the team. Independent *section* directives are nested within a *sections* directive. Each *section* contains different instructions and is executed once by a thread.
- It is possible for a thread to execute more than one *section*.
- *Sections* can be used to implement a *functional parallelism*.

~~~
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
~~~
{: .language-c}

Here *nowait* keyword (clause) means that the program will not wait at the end of `sections` block for all threads to finish.

> ## Exercise
> Compile the file *sections.c* and run it on a different number of CPUs. This example has two sections and the program prints out which threads are doing them.
> - What happens if the number of threads and the number of *sections* are different?
> - More threads than *sections*?
> - Less threads than sections?
>
> > ## Solution
> > If there are more threads than sections, only some threads will execute a section.  If there are more sections than threads, the implementation defines how the extra sections are executed.
> {: .solution}
{: .challenge}

#### *Single*
- The *single* directive specifies that the enclosed code is to be executed by only one thread in the team.
- May be useful when dealing with sections of code that are not thread safe (such as I/O).

{% include links.md %}

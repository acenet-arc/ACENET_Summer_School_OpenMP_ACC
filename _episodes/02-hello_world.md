---
title: "A Parallel Hello World Program"
teaching: 20
exercises: 10
questions:
- How do you compile and run an OpenMP program?
- What are OpenMP pragmas?
- How to identify threads?
objectives:
- Write, compile and run a multi-threaded program where each thread prints “hello world”.
keypoints:
- "OpenMP pragmas direct the compiler what to parallelize and how to parallelize it."
- "By using the environment variable `OMP_NUM_THREADS`, it is possible to control how many threads are used."
- "The order in which parallel elements are executed cannot be guaranteed."
- "A compiler that isn't aware of OpenMP pragmas will compile a single-threaded program."
---

## Adding Parallelism to a Program
OpenMP requires you to tell the compiler where to add the code necessary to create and use threads for the parallel sections.

This is handled through special compiler directive called pragmas. The word pragma is short for pragmatic information. A pragma is a way to communicate information to the compiler. The information is nonessential in the sense that the compiler may ignore the information and still produce a correct object program. However, the information provided by the pragma can help the compiler to optimize the program.
So OpenMP pragmas look like comments to a compiler that does not understand OpenMP. OpenMP statements in C/C++ have the following syntax:
{: .instructor_notes}

#### OpenMP core syntax
{: .self_study_text}
~~~
#pragma omp < OpenMP directive >
~~~
{: .language-c}


- All OpenMP directives begin with `#pragma omp`.

### How to add parallelism to the basic *hello_world* program?
Since OpenMP is a library, we must include a header file called `omp.h` that contains function prototypes and macro definitions.
{: .instructor_notes}
We will start by looking at the `parallel` directive. This directive forks threads so the parallel block of code can be executed.
{: .instructor_notes}

- Include OpenMP header
- Use the `parallel` directive
{: .self_study_text}

~~~
/* --- File: hello_world.c --- */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char **argv) {

#pragma omp parallel
   printf("Hello World\n");
}
~~~
{: .language-c}

#### How to compile an OpenMP program?
You will need to add an extra flag `-fopenmp` to your compiler command to make it treat the source code as OpenMP code.
{: .instructor_notes}

- Compiling OpenMP code requires the use of a compiler flag (`-fopenmp` for GCC, `-qopenmp` for Intel).
{: .self_study_text}

~~~
gcc -fopenmp -o hello hello.c
icc -qopenmp -o hello hello.c
~~~
{: .language-bash}


#### Running an OpenMP program

If you run this program, the output should say "Hello World" several times. Can you guess how many?
{: .instructor_notes}

Based on the OpenMP standard, this is determined by the implementation. OpenMP will normally look at the machine it is running on to determine how many cores are available. Then it will launch a thread for each core.
{: .instructor_notes}

You can control the number of threads by setting the environment variable OMP_NUM_THREADS. To create only three threads, for example, you would do the following:
{: .instructor_notes}

- Use the environment variable `OMP_NUM_THREADS` to control the number of threads.
{: .self_study_text}

~~~
export OMP_NUM_THREADS=3
./hello
~~~
{: .language-bash}

### What happens when the parallel hello_world program runs?

1. The `parallel` pragma instructs the compiler to create a code that starts a number of threads equal to what was passed into the program via the variable `OMP_NUM_THREADS`. 
2. Each team thread then executes the function `printf("Hello World\n")`
3. Threads rejoin the main thread when they return from the `printf()` function. At this point, team threads are terminated and only the main thread remains.
4. After reaching the end of the program, the main thread terminates.

> ## Using multiple cores
> Try running the `hello` program with different thread counts.
> - Is it possible to use more threads than the number of cores on the machine?
>
> You can use the `nproc` command to find out how many cores are on the machine.
>
>>## Solution
>> 
Since threads are a programming abstraction, there is no direct relationship between them and cores. In theory, you can launch as many threads as you like however, if you use more threads than physical cores, performance may suffer. There is also a possibility that the OS and/or OpenMP implementation can limit the number of threads.
> {: .solution} 
{: .challenge}

> ## OpenMP with SLURM
> To submit an OpenMP job to the SLURM scheduler, you can use the following submission script:
> ~~~
> #!/bin/bash
> #SBATCH --time=1:0:0
> #SBATCH --cpus-per-task=3
> #SBATCH --mem-per-cpu=1000
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
>  The most practical way to run our short parallel program on our test cluster is using `srun` command. Instead of submitting the job to the queue  `srun` will run the program from the interactive shell as soon as requested resources will become available. After the job is finished SLURM will release the allocated resources and exit. The `srun` command accepts the same keywords as `sbatch` and `salloc`.
>
> With SLURM environment, operating system will see as many CPUs as you have requested, so technically, OMP_NUM_THREADS variable inmost cases does not need to be set to the value of the variable `SLURM_CPUS_PER_TASK`. This may be needed, for example, when you run programs requesting both MPI tasks and cores on the same node.
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
>
> If you have not yet done so, download and unpack the code:
> ~~~
> cd ~/scratch
> mkdir workshop
> cd workshop
> wget https://github.com/acenet-arc/ACENET_Summer_School_OpenMP_ACC/raw/gh-pages/code/omp_code.tar 
> tar -xf omp_code.tar
> ~~~
> {: .language-bash}
{: .callout}

#### How can we tell which thread is doing what?   
The OpenMP specification includes a number of functions that are made available through the included header file `omp.h`. One of those functions is `omp_get_thread_num()` that returns the ID of the currently running thread.
{: .instructor_notes}

- The function `omp_get_thread_num()` returns the thread ID of the currently running process.
{: .self_study_text}

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

With this code, each thread's output will be tagged with its unique ID, a number between 0 and (number_of_threads - 1).
{: .instructor_notes}

- Another useful function is `omp_get_num_threads()`, which returns the number of threads.
{: .self_study_text}

> ## Thread ordering
> Run the `hello` program several times.  
> In what order do the threads write out their messages?  
> What is happening here? 
>
> > ## Solution
> > The messages are emitted in random order. This is an important rule of not only OpenMP programming, but parallel programming in general: parallel elements are scheduled to run by the operating system and order of their execution is not guaranteed.
> {: .solution}
{: .challenge}

> ## Conditional Compilation
> We said earlier that you should be able to use the same code for both OpenMP and serial work. Try compiling the code without the `-fopenmp` flag.
> - What happens?
> - Can you figure out how to fix it?
>
> Hint: When compiler is run with the `-fopenmp` option it defines preprocessor macro `_OPENMP`, so you may use the `#ifdef _OPENMP` preprocessor directive to tell compiler to only process the line containing `omp_get_thread_num()` function if the macro is defined.
> > ## Solution
> > ~~~
> >
> > #include <stdio.h>
> > #include <stdlib.h>
> > #include <omp.h>
> >
> > int main(int argc, char **argv) {
> >   #pragma omp parallel
> >   #ifdef _OPENMP
> >   printf("Hello World %i\n", omp_get_thread_num());
> >   #else
> >   printf("Hello World \n");
> >   #endif
> > }
> > ~~~
> > {: .language-c}
> {: .solution}
{: .challenge}

### General Parallel Sections

- In general parallel sections , it's up to you to decide what work each thread takes on.
{: .self_study_text}

We have just learned how to use the parallel directive. Now let's review what it does.
{: .instructor_notes}

- When a thread encounters `parallel` directive OpenMP creates a team of threads. 
- The thread that encountered the *parallel* directive first becomes the master thread of the new team, with a thread number of zero. 
- Parallel region is executed by all of the available threads.
{: .instructor_notes}

In our example each thread of the parallel hello program just printed its ID. When you know thread's ID you as a programmer can manage what work gets done by each thread. Using the parallel hello example as a starting point, we could use this code to make each thread do something interesting. We could write the text into a series of individual files, for example.
{: .instructor_notes}

### The *omp single*

- By using a *single* directive, we can run a block of code by just one thread, the thread that encounters it first.
{: .self_study_text}

There are times when you may need to drop out of a parallel section in order to have a single one of the threads executing some code. A code block associated with the `single` directive will be executed by only the first thread to come across it.
It might be useful for writing results from a parallel computation into a file, for example.
{: .instructor_notes}

View more information about the [*omp single* directive](https://www.openmp.org/spec-html/5.0/openmpsu38.html)
{: .instructor_notes}

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


---
title: "General Parallel Sections"
teaching: 10
exercises: 0
questions:
- "How to make each thread to do a different work?"
objectives:
- "Use general parallel sections"
- "Have a single thread execute code"
keypoints:
- "OpenMP can manage general parallel sections"
- "You can use 'pragma omp single' to have a single thread execute something"
---

So far, we have looked at parallelizing loops.
- OpenMP also allows you to use general parallel sections ([parallel **Construct**](https://www.openmp.org/spec-html/5.0/openmpse14.html)).

### The *omp parallel* directive
When a thread encounters *omp parallel* directive OpenMP creates a team of threads. The thread that encountered the *parallel* directive first becomes the master thread of the new team, with a thread number of zero. Parallel region is executed by all of the available threads.

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


Using this as a starting point, we could use this code to have each available thread do something interesting. For example, we could write the text out to a series of individual files.

### The *omp single* directive
There are times when you may need to drop out of a parallel section in order to have a single one of the threads executing some code.

- The *omp single* directive allows us to do this.

A code block associated with the *single* directive will be executed by only the first thread to see it.

View more information about the [*omp single* directive](https://www.openmp.org/spec-html/5.0/openmpsu38.html)

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


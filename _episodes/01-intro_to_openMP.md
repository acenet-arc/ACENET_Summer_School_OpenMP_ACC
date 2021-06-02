---
title:  "Introduction"
teaching: 20
exercises: 0
questions:
- "How shared memory parallel programs work?"
- "What is OpenMP?"
- "How to write and compile parallel programs in C?"
objectives:
- "Understand the shared memory programming environment provided by OpenMP"
- "Learn how to write and compile simple parallel programs in C"
keypoints:
- "Shared-memory parallel programs break up large problems into a number of smaller ones and execute them simultaneously" 
- "OpenMP programs are limited to a single physical machine"
- "OpenMP libraries are built into all commonly used C, C++, or Fortran compilers"
---


## Shared Memory OpenMP Programming
As we learned in the *General Parallel Computing* lesson, parallel programs come in two broad flavors: shared-memory and distributed memory or message-passing. In this lesson, we will be looking at shared-memory programming, with a focus on Open Multi-Processing (OpenMP) programming.

In any parallel program, the general idea is to have multiple threads of execution so that you can break up your problem and have each thread handle one part. These multiple threads need to be able to communicate with each other as your program runs. In a shared-memory program, this communication happens through the use of global variables stored in the global memory of the computer running the code. This means that communication between the various threads is extremely fast, as it happens at the speed of local memory (RAM) access. The drawback is that your program will be limited to a single physical machine (compute node of HPC network), since all threads need to be able to see the same RAM.

[OpenMP](https://www.openmp.org) is one way of writing shared-memory parallel programs. OpenMP is a specification, which has been implemented by many vendors.

The OpenMP effort began in 1996 when Accelerated Strategic Computing Initiative of the DOE brought together a handful of computer vendors including HP, IBM, Intel, SGI and DEC to create a portable API for shared memory computers. Vendors do not typically work well together unless an outside force encourages cooperation. So this committee communicated that DOE would only purchase systems with a portable API for shared memory programming.

The current OpenMP v.5.1 specification is 600+ pages long, but you need to know only a very small fraction of it to be able to use it in your code.

OpenMP standard describes extensions to a C/C++ or FORTRAN compiler. OpenMP libraries are built into a compiler, and this means that you need use a compiler that supports OpenMP. There are different OpenMP implementations (compilers) and not all sections of OpenMP specifications are equally supported by all compilers. It is up to programmers to investigate the compiler they want to use and see if it supports the parts of OpenMP specification that they wish to use. Luckily, the vast majority of OpenMP behaves the way you expect it to with most modern compilers. When possible, we will try and highlight any odd behaviors. 
All commonly used compilers such as gcc, clang, Intel, Nvidia HPC, and Absoft support OpenMP. 

[View OpenMP specifications](https://www.openmp.org/specifications/)  
[View Full List of Compilers supporting OpenMP](https://www.openmp.org/resources/openmp-compilers-tools/)  
For an overview of the past, present and future of the OpenMP read the paper ["The Ongoing Evolution of OpenMP"](https://ieeexplore.ieee.org/document/8434208).

## OpenMP Execution Model

The philosophy of OpenMP is to not sacrifice ease of coding and maintenance in the name of performance.

- OpenMP programs realizes parallelism through the use of threads. Recollect that a thread is the smallest unit of computing that can be scheduled by an operating system. In other words a thread is a subroutine that can be scheduled to run autonomously. Threads exist within the resources of a single process. Without the process, they cease to exist.  
- OpenMP offers the programmer full control over parallelization. It is an explicit, not automatic programming model.  
- OpenMP uses the so-called fork-join model of parallel execution:
  - OpenMP programs start as a single process, the master thread. The master thread executes sequentially. When the first parallel region is encountered. The master thread creates a team of parallel threads.We call this “forking”.
  - The statements in the parallel region of the program are executed in parallel by various team threads.
  - When the team threads complete the statements in the parallel region construct, they synchronize and terminate, leaving only the master thread. We call this “joining”.  
- OpenMP divides the memory into two types: Global (or shared) memory, and thread-local memory. Every thread can read and write the global memory, but each thread also gets a little slice of memory that can only be read or written by that thread. 

![](../fig/OpenMP-execution-model.svg)

## Compiling OpenMP programs
Since OpenMP is meant to be used with either C/C++ or FORTRAN, you will need to know how to work with at least one of these languages. This workshop will use C as the language for the examples. 
Before we start using a compiler let's load the most recent environment module:
~~~
module load StdEnv/2020
~~~
{:.language-bash}

You don't need to run this command on the real CC clusters because StdEnv/2020 is the default, but we need to run it on the training cluster.

 As a reminder, a simple hello world program in C would look like the following.

> ## Compiling C code
> ~~~
> #include <stdio.h>
> #include <stdlib.h>
>
> int main(int argc, char **argv) {
>   printf("Hello World\n");
> }
> ~~~
> {: .language-c}
> 
> In order to compile this code, you would need to use the following command:
>
> ~~~
> gcc -o hello hello_world.c
> ~~~
> {: .language-bash}
>
> This gives you an executable file "hello" that will print out the text "Hello World". You run it with the command:
>
> ~~~
> ./hello
> ~~~
> {: .language-bash}
> ~~~
> Hello World
> ~~~
> {: .output}
> If you don't specify the output filename with the -o option compiler will use the default output name "a.out" (assembler output).
{: .callout}

> ## GCC on Compute Canada
>
> Currently the default environment on the general purpose clusters (Beluga, Cedar, Graham) is StdEnv/2020. The default compilers available in this environment on Graham and Beluga are Intel/2020.1.217 and gcc/9.3.0. On Cedar the default compiler is gcc/8.4.0.
>
> To load another compiler you can use the command *module load*. For example, the command to load gcc version 10.2.0 is: 
>~~~
>module load gcc/10.2.0
>~~~
>{: .language-bash}
{: .callout}

### A Very Quick Introduction to C  
#### Preprocessor directives
  - The `#include` directive tells the preprocessor to insert the contents of another file into the source code.
  - Include directives are typically used to include header files for C functions that are defined outside of the current source file:

    ~~~
    #include <header_file> // Search in a predefined folders
    #include "header_file" // Search in the current directory
    ~~~
    {: .language-c}

  - Header files contain declarations of functions, variables and macros.

  - The `#define` directive declares constant values to be used throughout your code. For example it is a convenient way to change some parameters used throughout the code:
       ~~~
       # define SIZE 1000
       ~~~
      {: .language-c}

  - The *conditional group* `#ifdef`.

      ~~~
      #ifdef MACRO
      controlled code
      #endif /* MACRO */
      ~~~
       {: .language-c}

  - The block inside `#ifdef ... #endif` statements  is called a conditional group. Controlled code will be included in the output of the preprocessor only if `MACRO` (a block of C statements) is defined.

#### Basic Syntax
- Curly braces `{ ...  }` are used to group statements into a block of statements.  
- Statement terminator is `;`  
- For Loop syntax:  
  ~~~
  for (initialization; condition test; increment or decrement)
  {
         //Statements to be executed repeatedly
  }
  ~~~
   {: .language-c}

- You can skip any statements from for loop, but semicolons must be retained. For example, you can do initialization before loop:
  ~~~
  i=10;
  for(;i<100; i++)
  ~~~
   {: .language-c}

-  Skipping all loop statements  will result in an infinite loop: `for(;;)`

#### Defining Functions 
Function definitions have the following format:
  ~~~
  return_type function_name( parameter list ) {
     body of the function
  }
  ~~~
  {: .language-c}

  - Example:
    ~~~
    int max(int num1, int num2)
      {
         int result;
         if (num1 > num2)
            result = num1;
         else
            result = num2;
         return result;
      }
    ~~~
     {: .language-c}

#### Using Memory 

- Static arrays are allocated on stack.
- Size of stack-allocated arrays is limited by system-dependent threshold. Programs that attempt to stack-allocate arrays requiring more than this threshold will therefore crash as soon as they try to use the array.

Dynamic memory allocation is when an executing program requests that the operating system give it a block of main memory. The program then uses this memory for some purpose. 
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

### Shared Memory OpenMP Programming Overview
As we learned in the *General Parallel Computing lesson*, parallel programs can be divided into two broad types: shared memory and distributed memory or message-passing. The objective of this lesson is to teach you about shared memory programming using Open Multi-Processing (OpenMP).
{: .instructor_notes}

Shared memory parallel programs usually have multiple threads of execution. By breaking your problem up and having each thread handle a part of it, you solve the problem faster. In order for your program to run properly, the multiple threads need to communicate with each other. This communication happens through the use of global variables stored in the memory of the computer running the code. Thus, communication between the various threads is extremely fast - it happens at the speed of access to local memory (RAM). This programming model has the disadvantage that all threads in your program need access to the same RAM, so you will only be able to run the program on one device (single computing node).
{: .instructor_notes}


- **The [OpenMP](http://www.openmp.org) standard outlines how parallel programs should be written for shared memory systems.** 
{: .self_study_text}
- **The majority of compilers support OpenMP.**
{: .self_study_text}

An effort to create a portable API for shared memory computers began in 1996 with a DOE project called "Accelerated Strategic Computing Initiative" bringing together HP, IBM, Intel, SGI, and DEC. Most vendors do not work well together unless an outside force encourages them to do so. So this committee has communicated that DOE will only purchase systems that have a portable API for shared memory programming.
{: .instructor_notes}

The current OpenMP v.5.2 specification is 600+ pages long, but you need to know only a very small fraction of it to use it effectively in your code.
{: .instructor_notes}

An OpenMP standard describes extensions for C/C++ or FORTRAN compilers. Because OpenMP libraries are built into a compiler, you need one that supports OpenMP. Compilers implement OpenMP differently, and not all sections of the specification are supported by all compilers. The programmers are responsible for researching the compiler they intend to use and determining if it supports the parts of the OpenMP specification they are interested in using. With most modern compilers, OpenMP behaves as expected in the vast majority of situations. A rare event may occur where an OpenMP program behaves oddly.
{: .instructor_notes}

OpenMP is supported by all commonly used compilers, such as gcc, clang, Intel, Nvidia, and Absoft.
{: .instructor_notes}

[View OpenMP specifications](https://www.openmp.org/specifications/)  
[View Full List of Compilers supporting OpenMP](https://www.openmp.org/resources/openmp-compilers-tools/)  
For an overview of the past, present and future of the OpenMP read the paper ["The Ongoing Evolution of OpenMP"](https://ieeexplore.ieee.org/document/8434208).

### OpenMP Execution Model
OpenMP gives programmers full control over parallelization. Unlike automatic parallelization, it requires explicit programming. Among the guiding principles of OpenMP is that performance should not come at the expense of ease of coding and maintenance. 
{: .instructor_notes}

How does OpenMP assist programmers in using multiple CPU cores efficiently?
{: .instructor_notes}

- **OpenMP programs realize parallelism through the use of threads.** 

Recollect that a thread is the smallest unit of computing that can be scheduled by an operating system. In other words a thread is a subroutine that can be scheduled to run autonomously. Threads are contained within a single process, they use the resources of a single process. When the process ends, the threads cease to exist.
{: .instructor_notes}

**Parallel OpenMP execution is based on a concept called fork-join:**
{: .instructor_notes}
- An OpenMP program starts as a single thread, the master thread. This thread executes sequentially. 
- The master thread creates a team of parallel threads when it encounters the first parallel region, and the system divides a task among them. This process is called forking.
- Different team threads execute the statements in the parallel region of the program at the same time. The runtime environment allocates threads to different processors (or cores).
- After the team threads have finished the statements in the parallel region, they synchronize and terminate, leaving only the master thread. 
{: .instructor_notes}

- **Parallel OpenMP execution is based on a concept called fork-join:**
  1. An OpenMP program starts as a single thread, the master thread. 
  2. The master thread creates a team of parallel threads.
  3. Team threads execute statements in parallel.
  4. Team threads synchronize and terminate. 
{: .self_study_text}


![](../fig/OpenMP-execution-model.svg)

- **Memory is categorized into two types: shared and thread-local.**
{: .self_study_text}

In OpenMP, memory is categorized into two types: shared (or global) memory and thread-local memory. Each thread has access to the global memory, but it also receives its own slice of memory, which can only be read or written by that thread.
{: .instructor_notes}

## Compiling OpenMP Programs
Since the OpenMP library is designed to work with C/C++ or FORTRAN, you must be familiar with one of those languages. We will be using C in this workshop. Please do not worry if you don't know this language. We will introduce it to you shortly.
{: .instructor_notes}

It is now time to get the code examples. 
{: .instructor_notes}

Create a working directory in ~/scratch:
~~~
cd ~/scratch
mkdir workshop
cd workshop
~~~
{:.language-bash}

Code examples on the training cluster can be obtained by following these steps:
~~~
cp /tmp/omp_code.tar .
tar -xf omp_code.tar
~~~
{:.language-bash}

If you are using a real cluster, you can download examples from the web:
~~~
wget https://github.com/acenet-arc/ACENET_Summer_School_OpenMP_ACC/raw/gh-pages/code/omp_code.tar 
tar -xf omp_code.tar
~~~
{:.language-bash}

## A Very Quick Introduction to C  
### Preprocessor Directives
The compilation begins with preprocessing. The preprocessor is a simple tool for performing text substitutions before the compilation process begins. Preprocessor commands always begin with a hash symbol (#).
{: .instructor_notes}

- **The `#include` directive inserts the contents of a file.**  
{: .self_study_text}

- **The `#include` directive instructs the preprocessor to insert the contents of another file into the source code.**  
{: .instructor_notes}

~~~
#include <header_file.h> 
~~~
{: .language-c}

- Include directives are typically used to include files known as headers:
- Header files are used to declare functions, variables, and macros.  
{: .instructor_notes}

- **The `#define` directive declares constant values.**
{: .self_study_text}

- **The `#define` directive declares constant values that should be used throughout your code.** It is a convenient way to change certain parameters used throughout the code:
{: .instructor_notes}

~~~
# define SIZE 1000
~~~
{: .language-c}

- **The conditional group `#ifdef`.**

~~~
#ifdef MACRO
controlled code
#endif 
~~~
{: .language-c}

- Unlike #define directives which are always executed, `#ifdef` directives are only run when a certain condition is met.
- The block inside `#ifdef ... #endif` statements  is called a conditional group. Controlled code will be included in the preprocessor output if the macro (a block of statements) with the name MACRO is defined.
{: .instructor_notes}
  
### Basic Syntax
- **`{ ...  }`**  Curly braces are used to group statements into blocks. 
- **`;`** The semicolon operator is used to separate statements
- **`,`** The comma operator separates expressions (which have value) 

- **For Loop**  
  ~~~
  for ([initialization]; [condition test]; [increment or decrement])
  {
         //Statements to be executed repeatedly
  }
  ~~~
   {: .language-c}

- Example for loop:
  ~~~
  int i,c;
  for(i=10,c=1;i<100;i++) {
    c += 2*i; }
  ~~~
   {: .language-c}


- Any statements in a for loop can be skipped, but semicolons must remain. For example, you can do initialization before loop:
  ~~~
  i=10, c=1;
  for(; i<100; i++) {
    c += 2*i; }
  ~~~
   {: .language-c}

- Skipping all loop statements  will result in an infinite loop: `for(;;)`

### Functions 
- The format of function definitions is as follows:
  ~~~
  return_type function_name( parameters ) {
      body
  }
  ~~~
  {: .language-c}

- Here is an example of a function that finds the maximum of two numbers:
~~~
int max(int num1, int num2) {
    int result;
    if (num1 > num2)
      result = num1;
    else
      result = num2;
    return result;
}
~~~
    {: .language-c}

### Pointers
- Pointers are special variables used to store addresses of variables rather than values.

- The **`&`** operator (reference) returns the variable's address: address of **A** is **`&A`**.
{: .self_study_text :}

- With the **`&`** operator, you can retrieve the variable's address. For example, you can use ```&A``` to get the address of the variable ```A``` in memory.
{: .instructor_notes :}

- The **`*`** symbol has two meanings
  1. The **`*`** operator (dereference) allows you to access an object through a pointer. **`*X`**, for example, will give you the value of variable **`X`** if **`X`** is the memory address where it is stored.
  2. In the statement **`float* my_array;`** the variable **`my_array;`**  is defined as a pointer to a location in memory containing floating-point numbers.

Many C programming tasks such as array traversal or accessing complex data structures are more efficient when pointers are used. Other tasks, such as dynamic memory allocation, can only be accomplished by pointers. Learning pointers is vital if you wish to become a good C programmer.
{: .instructor_notes}

### Using Memory 
Static arrays are created during a program's compilation by specifying their size in source code. They are available throughout the program.
{: .instructor_notes}

- **Static arrays.**

~~~
int A[500]; 
~~~
{: .language-c}

A dynamic memory allocation occurs when a program requests that the operating system give it a block of main memory. The `malloc( )` is one of the functions used to allocate a block of memory dynamically. Dynamic arrays provide random access to their elements and are resizable. Programs can initialize them with variable sizes and change their sizes later on. An array's memory can be released when it is no longer required. 
{: .instructor_notes}

- **Dynamic arrays.**

~~~
int * A;  
size = 500; // Define array size
A = (int *)malloc(size*sizeof(int)); // Allocate memory
A = (int *)realloc(A,1000);  // Increase size
A[i]=10;  // Set array elements 
free(A); // Free memory 
~~~
{: .language-c} 

While declaring arrays as static is simple, using them has its disadvantages. Once we begin using arrays, we will discuss the drawbacks of static memory allocation in detail.
{: .instructor_notes}

### Compiling C Code
The following is an example of a simple hello world program written in C.
~~~
/* --- File hello_world.c --- */
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  printf("Hello World\n");
}
~~~
{: .language-c}
The command line arguments are passed to main subroutine in C by `argc` (argument count) and `**argv` (argument vector). If you don't intend to use command line arguments, you can omit them altogether: `int main()`.
{: .instructor_notes}

In order to compile this code, you would need to use the following command:
~~~
gcc -o hello hello_world.c
~~~
{: .language-bash}

This gives you an executable file *hello* that will print out the text "Hello World". You run it with the command:
~~~
./hello
~~~
{: .language-bash}
~~~
Hello World
~~~
{: .output}
- If you don't specify the output filename with the **`-o`** option compiler will use the default output name **a.out** (assembler output).

> ## Using C compilers on Compute Canada Systems
>
> - Currently the default environment on all general purpose clusters (Beluga, Cedar, Graham, Narval) is **`StdEnv/2020`**. 
> - The default compilers available in this environment on Graham and Beluga are **`Intel/2020.1.217`** and **`gcc/9.3.0`**. 
> - To load another compiler you can use the command **`module load`**. For example, the command to load **`gcc/10.3.0`** is: 
>
>~~~
>module load gcc/10.3.0
>~~~
>{: .language-bash}
{: .callout}


---
title: "Introduction to GPU Programming with OpenACC"
teaching: 20
exercises: 10
questions:
- "How to program GPU?"
objectives:
- "Learn about tools available for programming GPU"
- "Learn about programming GPU with openACC"
keypoints:
- "Different loop scheduling may compensate for unbalanced loop iterations"
---

## Introduction
 
As we learned in the *General Parallel Computing* lesson increasing performance is based on various strategies such as CPU frequency, multicore processing, vectorization, parallel distributed computing. At present performance is mostly limited by power consumption. Accelerators such as Nvidia Tesla GPUs are based on a very high level of parallelism and are capable to reach high performance at relatively low power consumption.  GPUs can deliver very high performance per compute node and today, GPGPUs are the choice of hardware to accelerate computational workloads in HPC settings. Let's look at the GPU architecture to understand why they are a good fit for various types of HPC jobs.

### The GPU Architecture 
CPUs are optimized to carry out tasks as quick as possible, while keeping the ability to quickly switch between operations. It’s nature is all about processing tasks in a serialized way

GPUs achieve high throughput by trading single-threaded performance in favor of several orders in magnitude more parallelism.

### Tools for programming GPUs
 There are several tools available for programming GPU.
 - CUDA. CUDA is NVIDIA-specific programming model and language. You can get the most out of your GPU with CUDA. CUDA-C and CUDA-Fortran compilers are available. Difficult to program, porting existing C/C++ or Fortran code onto the GPU with CUDA requires significant code refactoring.
 - OpenMP via the `target` construct. [OpenMP on GPUs](https://on-demand.gputechconf.com/gtc/2018/presentation/s8344-openmp-on-gpus-first-experiences-and-best-practices.pdf)
 - OpenCL. Open Computing Language is a framework that allows to write programs executing across platforms consisting of CPU, GPU, FPGA, and other hardware accelerators. It is very complex and hard to program. Adoption of OpenCL is still low.
 - PyCuDA. Gives access to CUDA functionality from Python code.
 - OpenACC. OpenACC offers a quick path to accelerated computing with less programming effort.

Examples of OpenACC accelerated applications: VASP 6 (*ab initio* atomic scale modeling), TINKER 9 (molecular dynamics)
[NVIDIA GPU Accelerated VASP 6 uses OpenACC to Deliver 15X More Performance](https://developer.nvidia.com/blog/nvidia-gpu-accelerated-vasp-6-uses-openacc-to-deliver-15x-more-performance/)

### OpenACC compilers

- GCC
- PGI
- Nvidia HPC SDK 

### OpenACC Example: Solving the Discrete Poisson's Equation using Jacobi's Method.

Poisson's equation arises in heat flow, diffusion, electrostatic and gravitational potential, fluid dynamics, .. etc. The governing equations of these processes are partial differential equations, which describe how each of the variables changes as a function of all the others.

$$ \nabla^2U(x,y)=f(x,y) $$

Here $ \nabla $ is the discrete Laplace operator, $ f(x,y) $ is a known function, and $ U(x,y) $ is the function to be determined.

This equation describes, for example, steady-state temperature of a uniform square plate. To make a solution defined we need to specify boundary conditions, i.e the value of $ U $ on the boundary of our square plate. We will consider the simple case with the boundaries kept at temperature $ U = 0 $

#### Jacobi's method in 2D.

- Common useful iterative algorithm.  
- For simplicity we will consider the case where $ f(x,y)=0 $ everywhere except one hot spot in the middle of the plate.
- For discretization on a uniform spatial grid $ n\times m $ the solution can be found by iteratively computing next value at the grid point $ i,j $ as an average of its four neighboring values and the function $ f(i,j) $.

$$ U(i,j,m+1) = ( U(i-1,j,m) + U(i+1,j,m) + U(i,j-1,m) + U(i,j+1,m) + f(i,j) )/4 $$


[View NVIDIA HPC-SDK](https://docs.nvidia.com/hpc-sdk/compilers/index.html) documentation.



compiling:
gcc laplace2d_omp.c -O3  -fopenmp  -lm
nvc laplace2d_omp.c -O3 -mp 

running: 
OMP_NUM_THREADS=8 ./a.out

2048x2048, 10000 iter
gcc  8 threads, 62.328627 sec
nvc, v100     , 22.748066 sec

4096x4096, 10000 iter
gcc, 8 threads,  122.991738 sec
icc, 8 threads,  83.892027 sec
nvc, v100     ,  48.860725 sec


## Open ACC


nvc laplace2d_acc.c -fast -acc -ta=nvidia -Minfo=accel


Let’s just drop in a single, simple OpenACC directive before each of our for loop nests in the previous code. Just after the #pragma omp lines, we add the following. (We leave the OpenMP directive in place so we retain portability to multicore CPUs.)

#pragma acc kernels
This tells the compiler to generate parallel accelerator kernels (CUDA kernels in our case) for the loop nests following the directive. To compile this code, we tell the PGI compiler to compile OpenACC directives and target NVIDIA GPUs using the -acc -ta=nvidia command line options (-ta=nvidia means “target architecture = NVIDIA GPU”). We can also enable verbose information about the parallelization with the -Minfo=accel option. If you are interested in the details, check out the Appendix below.

Let’s run the program. Our test PC has an Intel Xeon X5550 CPU and an NVIDIA Tesla M2090 GPU. When I run it, I see that it takes about 75 seconds. Uh oh, that’s a bit of a slow-down compared to my parallel CPU code. What’s going on? We can modify the compilation command to enable built-in profiling by specifying -ta=nvidia,time. Now when I run it I get:


## Vectorization: pointer aliasing

Compiler did not identify loops as parallelizable.



MC:
ml StdEnv/2020 nvhpc/20.7 cuda/11.0
nvc laplace2d_acc.c -O3 -acc -ta=tesla -Minfo=accel,time

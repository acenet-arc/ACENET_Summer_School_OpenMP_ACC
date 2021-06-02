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

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char **argv) {
   int id, size;

   #pragma omp parallel private(id,size)
   {
      size = omp_get_num_threads();
      id = omp_get_thread_num();
      printf("This thread %d of %d is first\n", id, size);
   }
}

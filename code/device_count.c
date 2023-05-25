#include <stdio.h>
#include <omp.h>
 
int main() 
{
  int num_devices = omp_get_num_devices();
  printf("Number of devices: %d\n", num_devices);
}

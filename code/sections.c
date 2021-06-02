/* --- File sections.c --- */
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#define N     5000

int main (int argc, char *argv[])
{
    int i, th;
    float a[N], b[N], c[N], d[N];

    /* Initialize arrays */
    for (i=0; i<N; i++) {
	a[i] = i * 2.3;
	b[i] = i + 10.35;
    }

#pragma omp parallel private(i,th)
    {
	th = omp_get_thread_num();
	printf("Thread %d starting...\n",th);
#pragma omp sections nowait
	{
#pragma omp section
	    {
		printf("Thread %d doing section 1\n",th);
		for (i=0; i<N; i++)
		    c[i] = a[i] + b[i];
		printf("Thread %d done\n",th);
	    }
#pragma omp section
	    {
		printf("Thread %d doing section 2\n",th);
		for (i=0; i<N; i++)
		    d[i] = a[i] * b[i];
    printf("Thread %d done\n",th);        
	    }
	}  /* end of sections */
    }  /* end of parallel section */

}

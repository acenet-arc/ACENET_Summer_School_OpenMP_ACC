/* File: vectorize_1.c */
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main()
{
        struct timespec ts_start, ts_end;
        float time_total;

        int i,j;
        long N=1e8; /* Size of test array */
        float a[N], b[N], c[N];  /* Declare static arrays */

/* Generate some random data */
        for(i=0;i<N;i++) {
                a[i]=random();
                b[i]=random();
        }
        printf("Test arrays generated\n");

/* Run test 3 times, compute average execution time */
        float average_time=0.0f;
        for(int k=0;k<3;k++)
        {
        clock_gettime(CLOCK_MONOTONIC, &ts_start);

          for(int j=0;j<50;j++)
            for(i=0;i<N;i++)
                c[i]=a[i]*b[i]+j;


        clock_gettime(CLOCK_MONOTONIC, &ts_end);
        time_total = (ts_end.tv_sec - ts_start.tv_sec)*1e9 + \
                     (ts_end.tv_nsec - ts_start.tv_nsec);
        printf("Total time is %f ms\n", time_total/1e6);
        average_time+=time_total;
        }
printf("Average time is %f ms\n", average_time/3e6);
}

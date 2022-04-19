/* File: find_factor.c */
#include <stdio.h>
#include <stdlib.h>

int main()
{
    int N = 26927249;
    int f;
    for (f = 2; f <= N; f++)
    {
        if (f % 200 == 0) /* Print progress */
        {
            fprintf(stdout, "checking: %li\n", f);
            fflush(stdout);
        }

        /* Check if f is a factor */
        if (N % f == 0)
        { // the remainder is 0, found factor!
            fprintf(stdout, "Factor: %li\n", f);
            exit(0);
        }
        for (int i = 1; i < 4e6; i++)
            ; /* Burn some CPU cycles */
    }
}
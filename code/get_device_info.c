#include <stdio.h>
#include <omp.h>

int main()
{
    int nteams, nthreads;
    int num_devices = omp_get_num_devices();
    printf("Number of available devices %d\n", num_devices);

#pragma omp target teams defaultmap(tofrom : scalar)
    {
        if (omp_is_initial_device())
            printf("Running on host\n");
        else
        {
            nteams = omp_get_num_teams();
#pragma omp parallel
#pragma omp single
            if (!omp_get_team_num())
            {
                nthreads = omp_get_num_threads();
                printf("Running on device with %d teams x %d threads\n", nteams, nthreads);
            }
        }
    }
}

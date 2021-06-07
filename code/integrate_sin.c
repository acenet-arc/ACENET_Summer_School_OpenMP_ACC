/* --- File integrate_sin.c --- */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char **argv) {
	int steps = 1e7;  /* Number of rectangles */ 
	double delta = M_PI/steps;/* Width of each rectangle */
	double total = 0.0;  /* Accumulator */
	int i;

	printf("Using %.0e steps\n", (float)steps);
	for (i=0; i<steps; i++) {
		total = total + sin(delta*i) * delta;
	}
	printf("The integral of sine from 0 to Pi is %.12f\n", total);
}

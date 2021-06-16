---
title: "Drawing the Mandelbrot set"
teaching: 5
exercises: 0
questions:
- "How do we handle irregular tasks?"
objectives:
- "Learn about the schedule() clause"
keypoints:
- "Different loop scheduling may compensate for unbalanced loop iterations"
---

// mandel color https://plus.maths.org/issue9/features/mandelbrot/pas/mbrot1.pas

C max = 0xffffff
R = C/(256^2);
G = (C/256) % 256;
B = C%256;


The <a href="https://en.wikipedia.org/wiki/Mandelbrot_set">Mandelbrot set</a>
was a hot subject of computer art in the 1980s.  The algorithm is quite simple:
- For each point on the screen, do an iterative calculation and decide whether the calculation diverges or not. Color that spot on the screen according to how many iterations it took to diverge or black if it didn't diverge in 1000 iterations.

~~~
# include <stdlib.h>
# include <stdio.h>
# include <math.h>
# include <time.h>

int main ( )
{
int m = 1000;
int n = 1000;

struct timespec ts_start, ts_end;
float time_total;
int c;
int count_max = 1000;
int i, j, k;
int jhi, jlo;
char *output_filename = "mandelbrot.ppm";
FILE *output_unit;
int **r, **g, **b, **count;
double x_max =   1.25;
double x_min = - 2.25;
double x, x1, x2;
double y_max =   1.75;
double y_min = - 1.75;
double y, y1, y2;
int i4_min ( int i1, int i2 );

/* Variables:
i, j - loop variables
k - pixel iteration variable
n, m - width and height of the image
x, x1, x2, y, y1, y2 - pixel coordinates mapped to the image range [n,m]
r, g, b - red, green, blue pixel color values [0-255] */

/* Allocate memory for pixels */
r=(int**)malloc(m*sizeof(int*));
for(i=0;i<m;i++)
  r[i]=malloc(n*sizeof(int));
g=(int**)malloc(m*sizeof(int*));
for(i=0;i<m;i++)
  g[i]=malloc(n*sizeof(int));
b=(int**)malloc(m*sizeof(int*));
for(i=0;i<m;i++)
  b[i]=malloc(n*sizeof(int));
count=(int**)malloc(m*sizeof(int*));
for(i=0;i<m;i++)
  count[i]=malloc(n*sizeof(int));

/* Record start time */ 
clock_gettime(CLOCK_MONOTONIC, &ts_start);

/* Carry out the iteration for each pixel, determining COUNT */
for ( i = 0; i < m; i++ )
  {
    y = ((i-1)*y_max + (m-i)*y_min)/(m-1);
    for ( j = 0; j < n; j++ )
      {
      x = ((j-1)*x_max + (n-j)*x_min)/(n-1);
      count[i][j] = 0;
      x1 = x;
      y1 = y;

      for ( k = 1; k <= count_max; k++ )
        {
          x2 = x1*x1 - y1*y1 + x;
          y2 = 2*x1*y1 + y;

          if (x2 < -2.0 || 2.0 < x2 || y2 < -2.0 || 2.0 < y2 )
          {
          count[i][j] = k;
          break;
        }
        x1 = x2;
        y1 = y2;
      }
      /* If count is 0 the point is not in set */
      if ( ( count[i][j] % 2 ) == 1 )
      {
        r[i][j] = 255;
        g[i][j] = 255;
        b[i][j] = 255;
      }
      else
      {
        c = (int) (255.0*(1-log(count[i][j])/log(count_max)));
        r[i][j] = c;
        g[i][j] = c;
        b[i][j] = c;
      }
    }
  }

clock_gettime(CLOCK_MONOTONIC, &ts_end);
time_total = (ts_end.tv_sec - ts_start.tv_sec)*1e9 + (ts_end.tv_nsec - ts_start.tv_nsec);
printf("\nTotal time is %f ms", time_total/1e6);

/* Write data to an ASCII PPM file. */
output_unit = fopen ( output_filename, "wt" );

fprintf ( output_unit, "P3\n" );
fprintf ( output_unit, "%d  %d\n", n, m );
fprintf ( output_unit, "%d\n", 255 );

for ( i = 0; i < m; i++ )
{
for ( jlo = 0; jlo < n; jlo = jlo + 4 )
  {
    jhi = i4_min ( jlo + 4, n );
    for ( j = jlo; j < jhi; j++ )
      {
        fprintf ( output_unit, "  %d  %d  %d", r[i][j], g[i][j], b[i][j] );
      }
      fprintf ( output_unit, "\n" );
  }
}

fclose ( output_unit );
printf ( "\n" );
printf ( "  Graphics data written to \"%s\".\n", output_filename );
printf ( "\n" );
return 0;
}

int i4_min ( int i1, int i2 ) {
int value;
if ( i1 < i2 )
  value = i1;
else
  value = i2;
return value;
}
~~~
{: .language-c}

- First, compile and run the program without OpenMP.
- Note how long it took to run. A millisecond is not enough to get good performance measurements on.
- Next, increase the dimensions `m,n` to `3000,3000` and recompile. Check the run time.

Now comes the parallelization.

> ## Parallelize the Mandelbrot Code
> 1. Decide what variable or variables should be made private, and then compile and test the code.
> 2. Run on few different numbers of CPUs. How does the performance scale?
> 3. Try different scheduling types. How does it affect the performance? What seems to work best for this problem? 
{: .challenge}

 
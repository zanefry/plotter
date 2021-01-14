#ifndef RK_H
#define RK_H

#include <unistd.h>

const short s = 4;
const float c[] = {0, 1.0/2, 1.0/2, 1};
const float b[] = {1.0/8, 3.0/8, 3.0/8, 1.0/8};
const float a[][3] = {{1.0/2}, {0, 1.0/2}, {0, 0, 1}};

void rk (double (*f)(double, double),
	  double base_x, double base_y,
	  double xmin, double xmax, double ymin, double ymax,
	  double stepsize, FILE* output)
{
  double x = base_x, y = base_y;

  double k[s];

  while (1)
    {
      fprintf (output, "%f %f\n", x, y);

      if ((x < xmin || xmax < x) || (y < ymin || ymax < y))
	return;

      for(int i = 0; i < s; i++)
	{
	  double sum = 0;

	  for(int j = 0; j < i; j++)
	    sum += a[i-1][j] * k[j];

	  k[i] = (*f)(x + c[i] * stepsize, y + sum * stepsize);
	}

      double avg = 0;
      for (int i = 0; i < s; i++)
	avg += b[i] * k[i];

      x += stepsize;
      y += avg * stepsize;
    }
}

#endif

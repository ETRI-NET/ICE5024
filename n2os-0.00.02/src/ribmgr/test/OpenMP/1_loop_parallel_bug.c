#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <omp.h>


int main()
{
  const int MAX =  100000000;
  double * Data;
  int i;

  Data = (double *)malloc(sizeof(double) * MAX);

  for (i=0; i<MAX; i++)
  {
    Data[i] = (double)i;
  }

#pragma omp parallel
{
  for (i=0; i<MAX; i++)
  {
    Data[i] = sqrt(Data[i]);
  }
}

  printf("Data : %g, %g, %g, %g, %g\n", Data[0], Data[1], Data[2], Data[3], Data[4]);

  return 0;
}

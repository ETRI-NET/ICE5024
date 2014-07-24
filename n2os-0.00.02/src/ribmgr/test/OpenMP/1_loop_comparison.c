#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <omp.h>
#include <utmpx.h>
#include <time.h>

int
getDiffTimeStr(char * buff, struct timespec * end, struct timespec * start)
{
  u_int64_t timeElapsed = 0;
  unsigned int second;
  unsigned int milliSecond;

  memset(buff, 0, 1024);

  /* Get difference time */
  timeElapsed = ((end->tv_sec * 1000000000) + end->tv_nsec) 
                - ((start->tv_sec * 1000000000) + start->tv_nsec);

  second = timeElapsed/1000000000; 
  milliSecond = (timeElapsed % 1000000000)/1000000;

  sprintf(buff, 
          "Elapsed time = %d seconds %d milliseconds", 
          second, milliSecond); 

  return 0;
}

void InputData(double * Data, int Size)
{
  int i = 0;
  for (i=0; i<Size; i++)
    Data[i] = (double)i;
}

void PrintData(double * Data)
{
  printf(":: Data : %g, %g, %g, %g, %g\n", 
          Data[0], Data[1], Data[2], Data[3], Data[4]);
}

int main()
{
  const int MAX =  100000000;
  double * Data;
  int i;
  struct timespec time1, time2, time3, time4;
  char buff[1024] = {};

  /* Check time */
  clock_gettime(CLOCK_MONOTONIC, &time1);

  Data = (double *)malloc(sizeof(double) * MAX);

  InputData(Data, MAX);

  /* Check & print elapsed time */
  clock_gettime(CLOCK_MONOTONIC, &time2);
  getDiffTimeStr(buff, &time2, &time1);
  printf(":: malloc -> %s\n", buff);

  /* Seauencial processing */
  for (i=0; i<MAX; i++)
    Data[i] = sqrt(Data[i]);

  /* Check & print elapsed time */
  clock_gettime(CLOCK_MONOTONIC, &time3);
  getDiffTimeStr(buff, &time3, &time2);
  printf(":: sequenctial squrt -> %s\n", buff);

  /* Set thread number */
  omp_set_num_threads(8);

  /* Parallel processing */
#pragma omp parallel
{
#pragma omp for
  for (i=0; i<MAX; i++)
    Data[i] = sqrt(Data[i]);
}

  /* Check & print elapsed time */
  clock_gettime(CLOCK_MONOTONIC, &time4);
  getDiffTimeStr(buff, &time4, &time3);
  printf(":: parallel squrt -> %s\n", buff);

  PrintData(Data);

  free(Data);

  return 0;
}

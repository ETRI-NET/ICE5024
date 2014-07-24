#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
int
getDiffTimeStr(char * buff, struct timespec * end, struct timespec * start)
{
  u_int64_t timeElapsed;
  unsigned int second;
  unsigned int milliSecond;

  /* Get difference time */
  timeElapsed = ((end->tv_sec * 1000000000) + end->tv_nsec) 
                - ((start->tv_sec * 1000000000) + start->tv_nsec);

  second = (unsigned int)timeElapsed/1000000000; 
  milliSecond = timeElapsed - second;

  sprintf(buff, 
          "Elapsed time = %d seconds %d nanoseconds", 
          second, milliSecond); 

  return 0;
}

int main()
{
  const int MAX =  100000000;
  double * Data;
  int i;
  struct timespec start, end;
  char buff[1024] = {};

  Data = (double *)malloc(sizeof(double) * MAX);

  for (i=0; i<MAX; i++)
  {
    Data[i] = (double)i;
  }

  /* Check start time */
  clock_gettime(CLOCK_MONOTONIC, &start);

  /* Calculate Root */
  for (i=0; i<MAX; i++)
  {
    Data[i] = sqrt(Data[i]);
  }

  /* Check end time */
  clock_gettime(CLOCK_MONOTONIC, &end);

  getDiffTimeStr(buff, &end, &start);
  printf(":: %s\n", buff);

  printf("Data : %g, %g, %g, %g, %g\n", Data[0], Data[1], Data[2], Data[3], Data[4]);

  return 0;
}

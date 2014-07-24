#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <omp.h>
#include <utmpx.h>
#include <time.h>
#include <ctime>

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

void CalcSQRT(double * Data, int Size)
{
  int i = 0;
  for (i=0; i<Size; i++)
    Data[i] = sqrt(Data[i]);
}

void CalcLOG(double * Data, int Size)
{
  int i = 0;
  for (i=0; i<Size; i++)
    Data[i] = log(Data[i]);
}


int main()
{
  int i;
  const int MAX = 100000000;
  double * Data1, * Data2;
  unsigned int time1, time2, time3, time4, time5, time6;
  char buff[1024] = {};

  /* Check time */
  time1 = clock();

  Data1 = (double *)malloc(sizeof(double) * MAX);
  Data2 = (double *)malloc(sizeof(double) * MAX);

 
  time2 = clock();
  printf("malloc -> %lu\n", time2 - time1);

  /* Input data */
  InputData(Data1, MAX);
  InputData(Data2, MAX);

  time3 = clock();
  printf("malloc -> %lu\n", time3 - time2);

  /* Seauencial processing */
  CalcSQRT(Data1, MAX);
  CalcSQRT(Data2, MAX);

  time4 = clock();
  printf("malloc -> %lu\n", time4 - time3);

  /* Input data */
  InputData(Data1, MAX);
  InputData(Data2, MAX);

  time5 = clock();
  printf("malloc -> %lu\n", time5 - time4);

  /* Set thread number */
  omp_set_num_threads(2);

  /* Parallel processing */
#pragma omp parallel
{
#pragma omp sections
  {
  #pragma omp section
    CalcSQRT(Data1, MAX);
  #pragma omp section
    CalcLOG(Data2, MAX);
  }
}

  time6 = clock();
  printf("malloc -> %lu\n", time6 - time5);

  PrintData(Data1);
  PrintData(Data2);

  free(Data1);
  free(Data2);

  return 0;
}

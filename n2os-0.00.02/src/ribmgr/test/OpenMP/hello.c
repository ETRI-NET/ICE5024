#include "stdio.h"
#include "pthread.h"

const int THREAD_COUNT = 4;

void * PrintHelloWorld(void * arg)
{
  printf ("Hello World! \n");
  return 0;
}

int main()
{
  pthread_t hThreadArray[THREAD_COUNT];
  int i = 0;

  for (i=0; i<THREAD_COUNT; i++)
  {
    pthread_create(&hThreadArray[i], NULL, PrintHelloWorld, (void *)NULL);
  }

  for (i=0; i<THREAD_COUNT; i++)
  {
    pthread_join(hThreadArray[i], (void *)NULL);
  }
  
}

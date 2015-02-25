#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

void *threadfun()
{
  int i;

  printf("thread: start counting\n");
  for (i = 1; i <= 10; i++)
  {
    printf("%d\n", i);
    sleep(1);
  }

  return NULL;
}

int main()
{
  pthread_t thread;

  printf("main: creating thread\n");
  if (pthread_create(&thread, NULL, &threadfun, NULL) != 0)
  {
    printf("Error creating thread!\n");
    return 1;
  }
  printf("main: thread created\n");

  if (pthread_join(thread, NULL) != 0)
  {
    printf("Error joining thread!\n");
    return 1;
  }
  printf("main: thread joined successfully\n");

  return 0;
}


#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>

#include <wiringPi.h>
#include <softPwm.h>

#define LED1 7
#define LED2 0
#define LED3 2
#define LED4 3

void init()
{
  wiringPiSetup();
}

void reset()
{
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);
}

void *ledfun()
{
  init();

  int i = 0;
  for (i = -10; i <= 10; i++)
  {
    int n = 10 - abs(i);
    digitalWrite(LED1, (n >> 0) & 1);
    digitalWrite(LED2, (n >> 1) & 1);
    digitalWrite(LED3, (n >> 2) & 1);
    digitalWrite(LED4, (n >> 3) & 1);

    printf("ledthread\t%lu: %d\n", pthread_self(), n);
    sleep(1);
  }

  reset();
  return NULL;
}

void *countfun()
{
  int i;
  for (i = -10; i <= 10; i++)
  {
    printf("countthread\t%lu: %d\n", pthread_self(), abs(i));
    sleep(1);
  }

  return NULL;
}

void arrayprint(size_t ncols, int numbers[][ncols])
{
  printf("arraythread\t%lu:\n", pthread_self());
  for (int i = 0; i < 10; i++)
  {
    for (int j = 0; j < 10; j++)
    {
      printf("%d\t", numbers[j][i]);
    }
    printf("\n");
  }
}

int compare (const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}

void *arrayfun()
{
  int numbers[10][10];

  srand(time(NULL));
  for (int i = 0; i < 10; i++)
  {
    for (int j = 0; j < 10; j++)
    {
      numbers[j][i] = 100.f * rand() / RAND_MAX ;
    }
  }
  arrayprint(10, numbers);

  qsort(numbers[0], 10, sizeof(int), compare);
  arrayprint(10, numbers);

  return NULL;
}

int create(pthread_t *thread, void *(threadfun)(void*), char* message)
{
  if (pthread_create(thread, NULL, threadfun, NULL) != 0)
  {
    printf("Error creating %s\n", message);
    return 1;
  }
  printf("%s id: %lu\n", message, *thread);
  return 0;
}

int join(pthread_t thread, char* message)
{
  if (pthread_join(thread, NULL) != 0)
  {
    printf("Error joining %s\n", message);
    return 1;
  }
  return 0;
}

int main()
{
  pthread_t led_thread;
  pthread_t count_thread;
  pthread_t array_thread;

  create(&led_thread,   &ledfun,   "led_thread");
  create(&count_thread, &countfun, "count_thread");
  create(&array_thread, &arrayfun, "array_thread");

  join(led_thread,   "led_thread");
  join(count_thread, "count thread");
  join(array_thread, "array thread");

  return 0;
}


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
  softPwmCreate(LED1, 0, 100);
  softPwmCreate(LED2, 0, 100);
  softPwmCreate(LED3, 0, 100);
  softPwmCreate(LED4, 0, 100);
}

void reset()
{
  softPwmStop(LED1);
  softPwmStop(LED2);
  softPwmStop(LED3);
  softPwmStop(LED4);
}

int pwmval(int i, int loc)
{
  float dist = abs(i - loc);
  float ratio = 1 - (dist / 64);
  return ratio < 0 ? 0 : ratio * 100;
}

void *ledfun()
{
  init();

  int inc;
  int i = 0;
  int idx = 0;
  while (i < 4)
  {
    if (idx == 0) { inc = 1; i++; }
    if (idx == 255) inc = -1;
    softPwmWrite(LED1, pwmval(idx, 31));
    softPwmWrite(LED2, pwmval(idx, 95));
    softPwmWrite(LED3, pwmval(idx, 159));
    softPwmWrite(LED4, pwmval(idx, 223));

    idx += inc;
    delay(10);
  }

  reset();
  return NULL;
}

void *countfun()
{
  int i = 1;
  while (i <= 20)
  {
    printf("%d\n", i++);
    sleep(1);
  }

  return NULL;
}

void *arrayfun()
{
  int numbers[10][10];

  srand(time(NULL));
  for (int i = 0; i < 10; i++)
  {
    for (int j = 0; j < 10; j++)
    {
      numbers[i][j] = 100.f * rand() / RAND_MAX ;
    }
  }

  for (int i = 0; i < 10; i++)
  {
    for (int j = 0; j < 10; j++)
    {
      printf("%d\t", numbers[i][j]);
    }
    printf("\n");
  }

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


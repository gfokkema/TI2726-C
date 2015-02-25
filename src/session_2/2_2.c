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
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);
}

int pwmval(int i, int loc)
{
  float dist = MIN(abs(i - loc), abs(i - 255 - loc));
  float ratio = 1 - (dist / 64);
  return ratio < 0 ? 0 : ratio * 100;
}

void *threadfun()
{
  int i = 0;
  while (1)
  { 
    softPwmWrite(LED1, pwmval(i, 32));
    softPwmWrite(LED2, pwmval(i, 96));
    softPwmWrite(LED3, pwmval(i, 160));
    softPwmWrite(LED4, pwmval(i, 224));

    i = (i + 1) % 255;
    delay(10);
  }

  return NULL;
}

int main()
{
  pthread_t thread;

  printf("main: creating thread\n");
  init();
  if (pthread_create(&thread, NULL, &threadfun, NULL) != 0)
  {
    printf("Error creating thread!\n");
    return 1;
  }
  printf("main: thread created\n");

  sleep(20);
  if (pthread_cancel(thread) != 0)
  {
    printf("Error cancelling thread!\n");
    return 1;
  }
  reset();
  printf("main: thread cancelled successfully\n");

  return 0;
}


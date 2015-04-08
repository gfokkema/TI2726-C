#include <math.h>
#include <pthread.h>
#include <semaphore.h>
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

pthread_mutex_t mutex;
int shared = 0;

void init()
{
  wiringPiSetup();
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
}

void reset()
{
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);
}

void *dispfun()
{
  init();

  while (1) {
    int n = shared;
    printf("dispthread\t%lu: %d\n", pthread_self(), n);

    digitalWrite(LED1, (n >> 0) & 1);
    digitalWrite(LED2, (n >> 1) & 1);
    digitalWrite(LED3, (n >> 2) & 1);
    digitalWrite(LED4, (n >> 3) & 1);

    sleep(1);
  }

  reset();
  return NULL;
}

void *incfun()
{
  pthread_mutex_lock(&mutex);           /** down semaphore : enter mutual exclusion */

  int i;
  for (i = 0; i < 15; i++)
  {
    shared++;
    printf("incthread\t%lu\n", pthread_self());

    sleep(1);
  }

  pthread_mutex_unlock(&mutex);           /**   up semaphore : exit mutual exclusion */
  return NULL;
}

void *decfun()
{
  pthread_mutex_lock(&mutex);           /** down semaphore : enter mutual exclusion */

  int i;
  for (i = 0; i < 10; i++)
  {
    shared--;
    printf("decthread\t%lu\n", pthread_self());

    sleep(1);
  }

  pthread_mutex_unlock(&mutex);           /**   up semaphore : exit mutual exclusion */
  return NULL;
}

int create(pthread_t *thread, pthread_attr_t *thread_attr, void *(threadfun)(void*), char* message)
{
  if (pthread_create(thread, thread_attr, threadfun, NULL) != 0)
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
  pthread_mutex_init(&mutex, NULL);

  pthread_t      inc_thread;
  pthread_t      dec_thread;
  pthread_t      disp_thread;

  struct sched_param low =  {  1 };
  struct sched_param high = { 99 };

  create(&inc_thread,  NULL, &incfun,  "inc_thread");
  pthread_setschedparam(inc_thread,  SCHED_RR, &high);
  create(&dec_thread,  NULL, &decfun,  "dec_thread");
  pthread_setschedparam(dec_thread,  SCHED_RR, &low);
  create(&disp_thread, NULL, &dispfun, "disp_thread");
  pthread_setschedparam(disp_thread, SCHED_RR, &low);

  join(inc_thread,  "inc_thread");
  join(dec_thread,  "dec thread");
  pthread_cancel(disp_thread);

  pthread_mutex_destroy(&mutex);

  return 0;
}


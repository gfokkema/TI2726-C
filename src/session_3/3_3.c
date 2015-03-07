#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>

#include <wiringPi.h>
#include <softPwm.h>

int *numbers;

void *minfun(void *params)
{
  int argc = *(int*)params;
  int min = INT_MAX;
  int max = INT_MIN;
  int total = 0;
  int i;
  for (i = 0; i < argc; i++)
  {
    if (numbers[i] < min) min = numbers[i];
    if (numbers[i] > max) max = numbers[i];
    total += numbers[i];
  }

  double *avg = malloc(sizeof(double));
  *avg = (double) total / argc;
  printf("The average value is %f\n", *avg);
  printf("The minimum value is %d\n", min);
  printf("The maximum value is %d\n", max);

  return (void*)avg;
}

int compare (const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}

void *medianfun(void *params)
{
  int argc = *(int*)params;
  int n[argc];

  for (int i = 0; i < argc; i++)
  {
    n[i] = numbers[i];
  }
  qsort(n, argc, sizeof(int), compare);
  printf("The median value is %f\n", argc % 2 ? n[argc / 2] : (n[argc / 2 - 1] + n[argc / 2]) / 2.f);

  return NULL;
}

typedef struct params
{
  int argc;
  double avg;
} params;

void *stddevfun(void *par)
{
  params *p = (params*)par;

  double stddev = 0;
  int i;
  for (i = 0; i < p->argc; i++)
  {
    double dev = numbers[i] - p->avg;
    stddev += dev * dev;
  }
  stddev /= p->argc;

  printf("The standard deviation is %f\n", sqrt(stddev));

  return NULL;
}

int create(pthread_t *thread, void *(threadfun)(void*), void *params, char* message)
{
  return 0;
}

int main(int argc, char **argv)
{
  int i;
  int idx = argc - 1;
  numbers = malloc(sizeof(int) * idx);

  if (idx < 1) goto err;
  for (i = 1; i <= idx; i++)
  {
    int n = strtol(argv[i], NULL, 10);
    if (n == 0 && *argv[i] != '0') goto err;
    numbers[i - 1] = n;
  }

  pthread_t    min_thread;
  pthread_t median_thread;
  pthread_t stddev_thread;
  double *retval;
  if (pthread_create(&min_thread, NULL, &minfun, &idx) != 0)         goto err;
  if (pthread_create(&median_thread, NULL, &medianfun, &idx) != 0)   goto err;

  /** We need to compute the average value before stddev */
  if (pthread_join(min_thread, (void**)&retval) != 0)                goto err;
  params p = { idx, *retval };
  if (pthread_create(&stddev_thread, NULL, &stddevfun, &p) != 0)     goto err;

  if (pthread_join(median_thread, NULL) != 0)                        goto err;
  if (pthread_join(stddev_thread, NULL) != 0)                        goto err;
  return 0;

  err:
  printf("Error!\n");
  free(numbers);
  return 1;
}


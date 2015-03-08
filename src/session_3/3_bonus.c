#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>

#include <wiringPi.h>
#include <softPwm.h>

int *numbers;

typedef struct listptr
{
  int* start;
  int len;
} listptr;

listptr* create_list(int len)
{
  listptr *list = malloc(sizeof(listptr));
  list->start = malloc(sizeof(int) * len);
  list->len = len;

  return list;
}

void delete_list(listptr* list)
{
  free(list->start);
  free(list);
}

void printlist(listptr* l)
{
  int i;
  for (i = 0; i < l->len; i++)
  {
    printf("[%d] -> %d\n", i, (l->start)[i]);
  }
}

listptr* merge(listptr* l, listptr* r)
{
  listptr *res = create_list(l->len + r->len);

  int idx = 0;
  int left = 0;
  int right = 0;
  while (left < l->len && right < r->len)
  {
    if ((l->start)[left] <= (r->start)[right])
    {
      res->start[idx++] = (l->start)[left++];
    }
    else
    {
      res->start[idx++] = (r->start)[right++];
    }
  }
  while (left < l->len)
  {
    res->start[idx++] = (l->start)[left++];
  }
  while (right < r->len)
  {
    res->start[idx++] = (r->start)[right++];
  }

  return res;
}

listptr* sort(listptr *list)
{
  if (list->len == 1)
  {
    return list;
  }

  listptr l     = { list->start, list->len / 2 };
  listptr r     = { list->start + list->len / 2, list->len - list->len / 2 };
  listptr *lres = sort(&l);
  listptr *rres = sort(&r);
  listptr *res  = merge(lres, rres);

  if (lres->len > 1) delete_list(lres);
  if (rres->len > 1) delete_list(rres);
  return res;
}

void *sortfun(void *params)
{
  listptr* list = (listptr*)params;
  listptr* res  = sort(list);
  printlist(res);
  free(res);

  return NULL;
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

  pthread_t sort_thread;
  listptr list = { numbers, idx };
  if (pthread_create(&sort_thread, NULL, &sortfun, &list) != 0) goto err;
  if (pthread_join(sort_thread, NULL) != 0)                     goto err;
  return 0;

  err:
  printf("Error!\n");
  free(numbers);
  return 1;
}


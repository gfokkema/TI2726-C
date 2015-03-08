#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define QSORTLIMIT 5

typedef struct listptr
{
  int* start;
  int len;
} listptr;

listptr *
create_list(int len)
{
  listptr * list = malloc(sizeof(listptr));
  list->start = malloc(sizeof(int) * len);
  list->len = len;

  return list;
}

void
delete_list(listptr * list)
{
  if (list != NULL)
  {
    if (list->start != NULL)
      free(list->start);
    free(list);
  }
}

void
printlist(listptr * l)
{
  for (int i = 0; i < l->len; i++)
  {
    printf("[%d] -> %d\n", i, (l->start)[i]);
  }
}

listptr *
merge(listptr * l, listptr * r)
{
  listptr * res = create_list(l->len + r->len);

  int idx = 0;
  int left = 0;
  int right = 0;
  while (left < l->len && right < r->len)
  {
    if ((l->start)[left] <= (r->start)[right])
      res->start[idx++] = (l->start)[left++];
    else
      res->start[idx++] = (r->start)[right++];
  }
  while (left < l->len)  res->start[idx++] = (l->start)[left++];
  while (right < r->len) res->start[idx++] = (r->start)[right++];

  return res;
}

int
compare (const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}

void *
sortfun(void * params)
{
  printf("new thread: %lu\n", pthread_self());
  listptr * list = (listptr*)params;
  listptr * leftres  = NULL;
  listptr * rightres = NULL;
  listptr * res      = NULL;;

  if (list->len < QSORTLIMIT)
  {
    qsort(list->start, list->len, sizeof(int), compare);
    res = list;
    goto clean;
  }

  int mid       = list->len / 2;
  listptr l     = { list->start, mid };
  listptr r     = { list->start + mid, list->len - mid };

  pthread_t left_thread, right_thread;
  if (pthread_create(&left_thread,  NULL, &sortfun, &l)  != 0) goto err;
  if (pthread_create(&right_thread, NULL, &sortfun, &r)  != 0) goto err;
  if (pthread_join  ( left_thread,  (void**)&leftres)    != 0) goto err;
  if (pthread_join  ( right_thread, (void**)&rightres)   != 0) goto err;
  res = merge(leftres, rightres);
  goto clean;

  err:
  printf("Error!\n"); res = NULL;

  clean:
  if (leftres  != NULL && leftres->len  >= QSORTLIMIT) delete_list(leftres);
  if (rightres != NULL && rightres->len >= QSORTLIMIT) delete_list(rightres);

  return res;
}

int
main(int argc, char **argv)
{
  int ret = 0;
  int idx = argc - 1;
  int * numbers = NULL;
  listptr * list = NULL;
  listptr * mergeres = NULL;
  pthread_t sort_thread;

  if (idx < 1) goto err;

  numbers = malloc(sizeof(int) * idx);
  for (int i = 1; i <= idx; i++)
  {
    int n = strtol(argv[i], NULL, 10);
    if (n == 0 && *argv[i] != '0') goto numerr;
    numbers[i - 1] = n;
  }

  list        = malloc(sizeof(listptr));
  list->start = numbers;
  list->len   = idx;
  if (pthread_create(&sort_thread, NULL, &sortfun, list) != 0) goto err;
  if (pthread_join  ( sort_thread, (void**)&mergeres)    != 0) goto err;

  printlist(mergeres);
  goto clean;

  numerr:
  free(numbers);
  err:
  printf("Error!\n"); ret = 1;

  clean:
  if (mergeres != list) delete_list(list);
  if (mergeres != NULL) delete_list(mergeres);

  return ret;
}


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

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
  if (list != NULL)
  {
    if (list->start != NULL)
      free(list->start);
    free(list);
  }
}

void printlist(listptr* l)
{
  for (int i = 0; i < l->len; i++)
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
      res->start[idx++] = (l->start)[left++];
    else
      res->start[idx++] = (r->start)[right++];
  }
  while (left < l->len)  res->start[idx++] = (l->start)[left++];
  while (right < r->len) res->start[idx++] = (r->start)[right++];

  return res;
}

void* mergefun(void *params)
{
  listptr* left  = ((listptr**)params)[0];
  listptr* right = ((listptr**)params)[1];

  return merge(left, right);
}

listptr* sort(listptr *list)
{
  if (list->len <= 1) return list;

  int mid       = list->len / 2;
  listptr l     = { list->start, mid };
  listptr r     = { list->start + mid, list->len - mid };
  listptr *lres = sort(&l);
  listptr *rres = sort(&r);
  listptr *res  = merge(lres, rres);

  if (lres->len > 1) delete_list(lres);
  if (rres->len > 1) delete_list(rres);
  return res;
}

void* sortfun(void *params)
{
  listptr* list = (listptr*)params;
  listptr* res  = sort(list);

  return res;
}

int main(int argc, char **argv)
{
  pthread_t left_thread;
  pthread_t right_thread;
  pthread_t merge_thread;

  listptr *leftres  = NULL;
  listptr *rightres = NULL;
  listptr *mergeres = NULL;

  int ret = 0;
  int idx = argc - 1;
  int *numbers = NULL;

  if (idx < 1) goto err;

  numbers = malloc(sizeof(int) * idx);
  for (int i = 1; i <= idx; i++)
  {
    int n = strtol(argv[i], NULL, 10);
    if (n == 0 && *argv[i] != '0') goto err;
    numbers[i - 1] = n;
  }

  if (idx < 4)
  {
    listptr list  = { numbers, idx };
    if (pthread_create(&left_thread,  NULL, &sortfun, &list)  != 0) goto err;
    if (pthread_join  ( left_thread,  (void**)&mergeres)  != 0)     goto err;
  }
  else
  {
    listptr left  = { numbers, idx / 2 };
    listptr right = { numbers + idx / 2, idx - idx / 2 };
    if (pthread_create(&left_thread,  NULL, &sortfun, &left)  != 0) goto err;
    if (pthread_create(&right_thread, NULL, &sortfun, &right) != 0) goto err;
    if (pthread_join  ( left_thread,  (void**)&leftres)  != 0)      goto err;
    if (pthread_join  ( right_thread, (void**)&rightres) != 0)      goto err;

    listptr *mergeparams[] = { leftres, rightres };
    if (pthread_create(&merge_thread, NULL, &mergefun, mergeparams ) != 0) goto err;
    if (pthread_join  ( merge_thread, (void**)&mergeres) != 0)             goto err;
  }

  printlist(mergeres);
  goto clean;

  err:
  printf("Error!\n"); ret = 1;

  clean:
  if (leftres  != NULL) delete_list(leftres);
  if (rightres != NULL) delete_list(rightres);
  if (mergeres != NULL) delete_list(mergeres);
  if (numbers  != NULL) free(numbers);

  return ret;
}


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define QSORTLIMIT 6

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

  // Iterate over l and r until we've reached the end of either l or r.
  // At each step, insert the smallest value in res and increment it's corresponding list index. 
  while (left < l->len && right < r->len)
  {
    if ((l->start)[left] <= (r->start)[right])
      res->start[idx++] = (l->start)[left++];
    else
      res->start[idx++] = (r->start)[right++];
  }
  // Iterate over any remaining list items and add them to res.
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
  listptr * list     = (listptr*)params;
  listptr * leftres  = NULL;
  listptr * rightres = NULL;
  listptr * res      = NULL;
  printf("new thread: %lu length: %d\n", pthread_self(), list->len);

  // Stop recursively descending when the list contains < QSORTLIMIT items.
  if (list->len < QSORTLIMIT)
  {
    qsort(list->start, list->len, sizeof(int), compare);
    res = list;
    goto clean;
  }

  // Split the list in a left and a right half.
  int mid       = list->len / 2;
  listptr l     = { list->start, mid };
  listptr r     = { list->start + mid, list->len - mid };

  // Recursively create a mergesort thread for the left and a right halves.
  pthread_t left_thread, right_thread;
  if (pthread_create(&left_thread,  NULL, &sortfun, &l)  != 0)
  { printf("Error creating left_thread!\n");  res = NULL; goto clean; }
  if (pthread_create(&right_thread, NULL, &sortfun, &r)  != 0)
  { printf("Error creating right_thread!\n"); res = NULL; goto clean; }

  // Block until the mergesort threads for the left and right halves join.
  if (pthread_join  ( left_thread,  (void**)&leftres)    != 0)
  { printf("Error joining left_thread!\n");   res = NULL; goto clean; }
  if (pthread_join  ( right_thread, (void**)&rightres)   != 0)
  { printf("Error joining right_thread!\n");  res = NULL; goto clean; }

  // Merge the sorted result from the left and the right halves.
  printf("merge thread: %lu length: %d\n", pthread_self(), list->len);
  res = merge(leftres, rightres);

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

  // Exit if no arguments are passed.
  if (idx < 1)
  {
    printf("Syntax: %s <num> <num> ... <num> <num>\n", argv[0]);

    return 1;
  }

  // Otherwise parse all arguments and store them in numbers.
  numbers = malloc(sizeof(int) * idx);
  for (int i = 1; i <= idx; i++)
  {
    // strtol returns zero when argv[i] is not a number.
    int n = strtol(argv[i], NULL, 10);
    if (n == 0 && *argv[i] != '0')
    {
      printf("Not a number: %s\n", argv[i]);

      free(numbers);
      return 1;
    }
    // if the number is valid, store it in our array
    numbers[i - 1] = n;
  }

  // Build a listptr from numbers
  list        = malloc(sizeof(listptr));
  list->start = numbers;
  list->len   = idx;

  // Create a sort_thread and pass it a listptr
  if (pthread_create(&sort_thread, NULL, &sortfun, list) != 0)
  { printf("Error creating thread\n"); ret = 1; goto clean; }

  // Wait for the sorting thread to complete and store the result in mergeres
  if (pthread_join  ( sort_thread, (void**)&mergeres)    != 0)
  { printf("Error joining thread!\n"); ret = 1; goto clean; }

  // Print the result
  printlist(mergeres);

  clean:
  if (mergeres != list) delete_list(list);
  if (mergeres != NULL) delete_list(mergeres);

  return ret;
}


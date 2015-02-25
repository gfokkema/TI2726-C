#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

int sudoku[9][9] =
{
  { 6, 2, 4, 5, 3, 9, 1, 8, 7 },
  { 5, 1, 9, 7, 2, 8, 6, 3, 4 },
  { 8, 3, 7, 6, 1, 4, 2, 9, 5 },
  { 1, 4, 3, 8, 6, 5, 7, 2, 9 },
  { 9, 5, 8, 2, 4, 7, 3, 6, 1 },
  { 7, 6, 2, 3, 9, 1, 4, 5, 8 },
  { 3, 7, 1, 9, 5, 6, 8, 4, 2 },
  { 4, 9, 6, 1, 8, 2, 5, 7, 3 },
  { 2, 8, 5, 4, 7, 3, 9, 1, 6 }
};
int result[27];

typedef void (*inc_t)(int *x, int *y);

typedef struct
{
  int id, row, col;
  inc_t inc;
} parameters_t;

void
row_inc(int *x, int *y)
{
  (*x)++;
}

void
col_inc(int *x, int *y)
{
  (*y)++;
}

void
block_inc(int *x, int *y)
{
  *x = *x + 1;
  if (*x % 3 == 0)
  {
    *x = *x - 3;
    *y = *y + 1;
  }
}

void *threadfun(void *paramsv)
{
  parameters_t *params =  paramsv;
  int values[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };

  int i;
  for (i = 0; i < 9; i++)
  {
    int v = *(*(sudoku + params->col) + params->row);
    if (v < 1 || v > 9 || values[v - 1] != 0) return NULL;

    values[v - 1] = v;
    (*(params->inc))(&params->row, &params->col);
  }
  result[params->id] = 1;
  
  return NULL;
}

parameters_t*
newparams(int id, int row, int col, inc_t inc)
{
  parameters_t *params = malloc(sizeof(parameters_t));
  params->id = id;
  params->row = row;
  params->col = col;
  params->inc = inc;

  return params;
}

int main()
{
  int i;
  int valid;
  pthread_t threads[9 * 3];
  parameters_t *params[9 * 3];

  for (i = 0; i < 9; i++)
  {
    params[i +  0] = newparams(i +  0, 0, i,                 &row_inc);
    params[i +  9] = newparams(i +  9, i, 0,                 &col_inc);
    params[i + 18] = newparams(i + 18, i / 3 * 3, i % 3 * 3, &block_inc);
  }

  for (i = 0; i < 27; i++)
  {
    if (pthread_create(&threads[i], NULL, &threadfun, params[i]) != 0)
    {
      printf("Error creating thread!\n");
      return 1;
    }
  }

  for (i = 0; i < 27; i++)
  {
    if (pthread_join(threads[i], NULL) != 0)
    {
      printf("Error joining thread!\n");
      return 1;
    }
  }

  valid = 1;
  for (i = 0; valid && i < 27; i++)
  {
    if (result[i] == 0) valid = 0;
  }

  if (valid) printf("valid!\n");
  else printf("invalid!\n");

  return 0;
}


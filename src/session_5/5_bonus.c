#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>

sem_t assist_mutex;
sem_t assist_semaphore;
sem_t assist_finished;

void *assistfun()
{
  while (1)
  {
    sem_wait(&assist_semaphore);
    sleep(10);                           /** Explaining takes a while... */

    printf("TA has finished explaining.\n");
    sem_post(&assist_mutex);             /** The TA is free for others to use */
    sem_post(&assist_finished);          /** The TA is finished explaining */
  }

  return NULL;
}

void *studentfun(void *args)
{
  while (1)
  {
    int queued;

    int work_time = 10 + (float)rand() / RAND_MAX * 20;
    printf("Student %lu starts programming for %d seconds.\n", pthread_self(), work_time);
    sleep(work_time);

    printf("Student %lu seeks help from TA.\n", pthread_self());
    sem_getvalue(&assist_semaphore, &queued);
    if (queued < 10)
    {
      printf("%d students are queued already.\n", queued);
      sem_post(&assist_semaphore);         /** Get the TA's attention */
      sem_wait(&assist_mutex);             /** Wait until he has noticed you */

      printf("TA starts explaining to %lu\n", pthread_self());
      sem_wait(&assist_finished);          /** Wait until the TA is done explaining */
    }
    else
    {
      printf("%d students are queued, student %lu will try again later.\n", queued, pthread_self());
    }
  }

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

int main(int argc, char **argv)
{
  pthread_t   assist_thread;
  pthread_t * student_threads;
  int         n_students;

  // Exit if no arguments are passed.
  if (argc != 2)
  {
    printf("Syntax: %s <num>\n", argv[0]);
    return 1;
  }

  n_students = strtol(argv[1], NULL, 10);
  if (n_students == 0 && *argv[1] != '0')
  {
    printf("Not a number: %s\n", argv[1]);
    return 1;
  }

  srand(time(NULL));
  sem_init(&assist_mutex, 0, 1);
  sem_init(&assist_semaphore, 0, 0);

  create(&assist_thread,  NULL, &assistfun,  "assist thread");
  student_threads = malloc(sizeof(pthread_t) * n_students);
  for (int i = 0; i < n_students; i++)
  {
    create(student_threads + i,  NULL, &studentfun,  "student thread");
  }
  for (int i = 0; i < n_students; i++)
  {
    join(student_threads[i],  "student thread");
  }
  join(assist_thread, "assist thread");

  sem_destroy(&assist_mutex);

  return 0;
}


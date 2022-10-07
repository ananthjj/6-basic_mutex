#include "simple_mutex.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX 1000000
#define NUM_THREADS 4
#define MAX_ITERATIONS 10

static int sum = 0;

static int mutex = 0;

void *thread_sum(void *arg)
{
  for (int i = 0; i < MAX; i++) {
    mutex_lock(&mutex);
    sum++;
    mutex_unlock(&mutex);
  }

  return NULL;
}

int main(int argc, char* argv[])
{
  pthread_t threads[NUM_THREADS];

  for (int i = 0; i < MAX_ITERATIONS; i++) {
    printf("Starting test %d of %d\n", i + 1, MAX_ITERATIONS);
    sum = 0;

    for (int j = 0; j < NUM_THREADS; j++) {
      if (pthread_create(&threads[j], NULL, thread_sum, (void *)NULL)) {
	printf("Error creating threads!\n");
	exit(1);
      }
    }

    for (int j = 0; j < NUM_THREADS; j++) {
      if (pthread_join(threads[j], NULL)) {
	printf("Error joining threads!\n");
	exit(1);
      }
    }

    if (sum != (MAX * NUM_THREADS))
      printf("Test %d failed!\n", i + 1);
    else
      printf("Test %d passed!\n", i + 1);
  }
 
  return 0;
}

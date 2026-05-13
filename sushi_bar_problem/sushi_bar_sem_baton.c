#include <assert.h>
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define SEATS_AVAILABLE 5
#define TOTAL_GUESTS 10

void *enter_the_restaurant(void *arg);
void *leave_the_restaurant(void *arg);
void *customer(void *arg);

void Pthread_create(pthread_t *tid, void *(*routine)(void *), void *arg);
void init_state();
void clean();
void lunch();

sem_t emptied;
sem_t mutex;
int dr;
int seatsAvailable;

int main() {
  pthread_t threads[TOTAL_GUESTS];
  init_state();

  for (int i = 0; i < TOTAL_GUESTS; i++) {
    int *id = malloc(sizeof(int));
    *id = i + 1;
    Pthread_create(&threads[i], customer, id);
  }

  for (int i = 0; i < TOTAL_GUESTS; i++) {
    int result = pthread_join(threads[i], NULL);
    assert(result == 0);
  }

  printf("All guests left. Closing the restaurant...\n");
  clean();
  return 0;
}

void *enter_the_restaurant(void *arg) {
  sem_wait(&mutex);
  if (seatsAvailable == 0 || dr > 0) {
    dr++;
    sem_post(&mutex);
    sem_wait(&emptied);
    dr--;
  }
  seatsAvailable--;
  if (dr > 0 && seatsAvailable > 0) {
    sem_post(&emptied);
  } else {
    sem_post(&mutex);
  }
  return NULL;
}

void *leave_the_restaurant(void *arg) {
  sem_wait(&mutex);
  seatsAvailable++;
  if (seatsAvailable == SEATS_AVAILABLE && dr > 0) {
    sem_post(&emptied);
  } else {
    sem_post(&mutex);
  }
  return NULL;
}

void *customer(void *arg) {
  int id = *(int*)arg;

  printf("Customer with id: %d just entered the restaurant\n", id);
  enter_the_restaurant(NULL);
  leave_the_restaurant(NULL);
  printf("Customer with id: %d just left the restaurant\n", id);

  return NULL;
}
void init_state() {
  dr = 0;
  seatsAvailable = SEATS_AVAILABLE;

  sem_init(&mutex, 0, 1);
  sem_init(&emptied, 0, 0);
}

void lunch() {
  printf("Taking a lunch\n");

  struct timespec ts = {
      .tv_sec = 1,
      .tv_nsec = 200 * 1000 * 1000,
  };

  nanosleep(&ts, NULL);

  printf("Finished a lunch\n");
}

void Pthread_create(pthread_t *tid, void *(*routine)(void *), void *arg) {
  int result = pthread_create(tid, NULL, routine, arg);
  assert(result == 0);
}

void clean() {
  sem_destroy(&mutex);
  sem_destroy(&emptied);
}

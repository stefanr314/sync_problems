#include <assert.h>
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define STATION_NUMBER 5
#define BUS_CAPACITY 30

typedef struct __station_info {
  int fromStationId;
  int toStationId;
} station_info;

void *bus_routine(void *args);
void *passenger_routine(void *args);
void init_routine();
void destroy_routine();
void Pthread_create(pthread_t *tid, const pthread_attr_t *attr,
                    void *(*routine)(void *), void *arg);
void Pthread_join(pthread_t tid, void **thread_return);
station_info *yield_stations();

sem_t mutex;
sem_t busToLeave;
sem_t passengersLeft;
sem_t destinationStation[STATION_NUMBER];
sem_t stationSem[STATION_NUMBER];
int atStation[STATION_NUMBER];
int toStation[STATION_NUMBER];
int inBuss;

int main() {
  pthread_t busThread;
  pthread_t passengerThread[100];

  init_routine();

  srand((unsigned int)time(NULL));

  for (int i = 0; i < 100; i++) {
    station_info *si = yield_stations();
    Pthread_create(&passengerThread[i], NULL, passenger_routine, (void *)si);
  }

  Pthread_create(&busThread, NULL, bus_routine, NULL);

  for (int i = 0; i < 100; i++) {
    Pthread_join(passengerThread[i], NULL);
  }

  int res = pthread_cancel(busThread);
  assert(res == 0);
  Pthread_join(busThread, NULL);
  destroy_routine();
}

void *bus_routine(void *args) {
  // int arrivedAtStation = (int)args;

  int i;
  struct timespec td = {.tv_sec = 0, .tv_nsec = 500 * 1000 * 1000};

  for (;;) {
    for (i = 0; i < STATION_NUMBER; i++) {
      sem_wait(&mutex);
      if (toStation[i] > 0) {
        sem_post(&destinationStation[i]);
        sem_wait(&passengersLeft);
        printf("Passengers left at the station %d\n", i);
      }
      if (atStation[i] > 0 && inBuss < BUS_CAPACITY) {
        sem_post(&stationSem[i]);
        sem_wait(&busToLeave);
        printf("Bus left the station %d\n", i);
      }
      printf("Currently in buss: %d\n", inBuss);
      sem_post(&mutex);
      // simulate driving
      printf("Driving started\n");
      nanosleep(&td, NULL);
    }
  }
  return NULL;
}

void *passenger_routine(void *arg) {
  station_info *st = (station_info *)arg;

  sem_wait(&mutex);
  atStation[st->fromStationId]++;
  sem_post(&mutex);
  sem_wait(&stationSem[st->fromStationId]);

  // do the action after the baton is passed, so all the code is done under
  // the bus mutex
  inBuss++;
  atStation[st->fromStationId]--;
  toStation[st->toStationId]++;
  if (inBuss < BUS_CAPACITY && atStation[st->fromStationId] > 0) {
    sem_post(&stationSem[st->fromStationId]);
  } else {
    sem_post(&busToLeave);
  }

  // driveAround();

  sem_wait(&destinationStation[st->toStationId]);
  inBuss--;
  toStation[st->toStationId]--;
  if (toStation[st->toStationId] > 0) {
    sem_post(&destinationStation[st->toStationId]);
  } else {
    sem_post(&passengersLeft);
  }

  free(st);
  return NULL;
}

void init_routine() {
  sem_init(&mutex, 0, 1);
  sem_init(&busToLeave, 0, 0);
  sem_init(&passengersLeft, 0, 0);
  for (int i = 0; i < STATION_NUMBER; i++) {

    sem_init(&stationSem[i], 0, 0);
    sem_init(&destinationStation[i], 0, 0);
    atStation[i] = 0;
    toStation[i] = 0;
  }

  inBuss = 0;
}

void destroy_routine() {
  sem_destroy(&mutex);
  sem_destroy(&busToLeave);
  sem_destroy(&passengersLeft);

  for (int i = 0; i < STATION_NUMBER; i++) {
    sem_destroy(&stationSem[i]);
    sem_destroy(&destinationStation[i]);
  }
}

void Pthread_create(pthread_t *tid, const pthread_attr_t *attr,
                    void *(*routine)(void *), void *arg) {
  int res = pthread_create(tid, attr, routine, arg);
  assert(res == 0);
}

void Pthread_join(pthread_t tid, void **thread_return) {
  int res = pthread_join(tid, thread_return);
  assert(res == 0);
}

station_info *yield_stations() {

  station_info *si = malloc(sizeof(station_info));
  int toStationId = rand() % 5;
  int fromStationId = rand() % 5;

  while (toStationId == fromStationId) {
    toStationId = rand() % 5;
  }

  si->fromStationId = fromStationId;
  si->toStationId = toStationId;

  return si;
}

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <semaphore.h>

#define N 3
#define MaxP 1 // maximum number of people that can stay inside the park
#define MaxB 1  // number of bicycles
#define MaxM 1  // number of scooters
#define V sem_wait
#define P sem_post

typedef struct {
	int free_spots;
	int free_bikes;
	sem_t S; // condition semaphore
	int free_scooters;
} park;

park data;

void sleep_ms(int ms) {
	struct timespec ts;
	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;  // ms ? ns
	nanosleep(&ts, NULL);
}

void* parkTour(void *id) {
	int user = (intptr_t) id;
	printf("\n%d: in line at the ticket office\n", user);
	bool has_ticket = false, has_vehicle = false;
	bool wants_bike = (1 + rand() % 10) % 2 == 0;  // Random vote between 1 and 10
	printf("\n%d: wants a bike %d\n", user, wants_bike);
	// waiting semaphore to avoid multiple accesses to shared data structure

	while (!has_ticket && !has_vehicle) {
		sem_wait(&data.S);
		printf("\n%d: talking to the clerk\n", user);
		if (!has_ticket && data.free_spots > 0) {
			data.free_spots--;
			printf("\n%d: got the ticket\n", user);
			has_ticket = true;
		}
		if (has_ticket && !has_vehicle && wants_bike && data.free_bikes > 0) {
			data.free_bikes--;
			printf("\n%d: got a bike\n", user);
			has_vehicle = true;
		}
		if (has_ticket && !has_vehicle && !wants_bike
				&& data.free_scooters > 0) {
			data.free_scooters--;
			printf("\n%d: got a scooter\n", user);
			has_vehicle = true;
		}
		sem_post(&data.S);
		sleep_ms(3000);
	};

	printf("\n%d: got everything, starting to visit the park\n", user);
	sleep_ms(3000);
	printf("\n%d: finished visiting the park\n", user);

	printf(
			"\n%d: waiting to talk to the clerk to return the vehicle\n",
			user);
	while (has_vehicle || has_ticket) {
		sem_wait(&data.S);
		printf("\n%d: talking to the clerk\n", user);
		if (has_ticket) {
			data.free_spots++;
			printf("\n%d: freed a spot in the park\n", user);
			has_ticket = false;
		}
		if (has_vehicle && wants_bike) {
			data.free_bikes++;
			has_vehicle = false;
			printf("\n%d: returned the bike\n", user);
		}
		if (has_vehicle && !wants_bike) {
			data.free_scooters++;
			has_vehicle = false;
			printf("\n%d: returned the scooter\n", user);
		}
		sem_post(&data.S);
	};
	printf("\n%d: end of the visit to the park\n", user);
	pthread_exit(NULL);  // Terminate the thread
}

int main() {
	pthread_t users[N];  // N users (threads)
	int rc;
	int status;
	
	data.free_spots = MaxP;
	data.free_bikes = MaxB;
	data.free_scooters = MaxM;
	sem_init(&data.S, 0, 1);

	// Thread creation (users)
	for (int t = 0; t < N; t++) {
		rc = pthread_create(&users[t], NULL, parkTour,
				(void*) (intptr_t) t);
		if (rc) {
			printf("ERROR creating thread %d\n", rc);
			exit(-1);
		}
	}

	// Wait for all threads to finish
	for (int t = 0; t < N; t++) {
		rc = pthread_join(users[t], NULL);
		if (rc) {
			printf("ERROR joining thread %d\n", rc);
			exit(-1);
		}
	}

	printf("\nFINISHED!:\n");
	return 0;
}

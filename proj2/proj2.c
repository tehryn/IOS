/*
 * Author: Jiri Matejka
 * Login: xmatej52
 * School: VUT FIT, Brno
 * Date: 02-05-2016
 * Module: IOS-PROJ2 proj2.c
 * gcc version 4.9.2 (Debian 4.9.2-10)
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <time.h>
#include <signal.h>
#include "proj2.h"

FILE *output; // output of passengers and car
	
pid_t process_pid; // pid of processes

/******************************************************************************/
/*============================================================================*/
int main(int argc, char **argv) {

	/* Check if arguments are valid */
	if (!check_arguments(argc, argv)) {
		fprintf(stderr,"Invalid arguments.\n");
		exit(1);
	}
	
	int P; // number of passengers
	int C; // capacity of car
	int PT; // maximum value of time [ms] to generate new process for passenger
	int RT; // maximum value of time [ms] to finish the roller cast track
	
	/* Save value of arguments into addresses and check if values are valid */
	if (!get_arguments_values(&P, &C, &PT, &RT, argv)) {
		fprintf(stderr, "Invalid values of arguments.\n");
		exit(1);
	}
	
	/* Set output */
	if ((output = fopen("proj2.out", "w")) == NULL) {
		fprintf(stderr, "Cannot open/create file 'proj2.out'\n");
		exit(2);
	}

	/* Allocate and initialize semaphores */
	if(!set_semaphores()) {
		fclose(output);
		exit(2);
	}
	
	/* Allocate and initialize shared variables */
	if(!set_shared_memory()) {
		unlink_semaphores();
		fclose(output);
		exit(2);
	}
	signal(SIGQUIT, kill_them_all); // receive signal in case of error	
	/* Create spawner of passengers */
	process_pid = fork();
	int id = 1; // ID of processes
	
	/* Spawn passengers and set their ID */
	if (process_pid == 0)
		spawn_passengers(&id, P, PT);
	
	if (process_pid == 0) {
		/* Starts the passengers, but they must wait for car */
		proceed_passengers(C, P, id);
		exit(0);
	}
	
	/* Starts Roller coaster and car */
	proceed_car(C, P, RT);	
	/* Car will not be exited yet, we use it to free all memory, unlink and
	   close all semaphores */
	
	/* This semaphore will be unlocked by last passenger so resources will be
	   freed after all processes end */
	sem_wait(semaphore_unboard);
	
	/* Free shared variables */
	free_shared_memory();
	
	/* Unlink and close all semaphores */
	unlink_semaphores();
	
	/* Close file */
	fclose(output);
	exit(0);
}
/*============================================================================*/
/******************************************************************************/


void passenger_board(int id, int C) {

	sem_wait(semaphore_write);
	fprintf(output, "%d\t: P %d\t: board\n",(*shared_counter)++, id);
	fflush(output);
	sem_post(semaphore_write);

	sem_wait(semaphore_write);
	
	/* Last passenger that can board */
	if (*shared_board == C) {
		fprintf(output, "%d\t: P %d\t: board last\n",(*shared_counter)++, id);
		fflush(output);
		*shared_board = 1; // reset the value
		sem_post(semaphore_board); // open the semaphore for car
	}
	else
		fprintf(output, "%d\t: P %d\t: board order %d\n",(*shared_counter)++, id, (*shared_board)++);
		fflush(output);
	sem_post(semaphore_write);
}

void passenger_unboard(int id, int C) {
	sem_wait(semaphore_write);
	fprintf(output, "%d\t: P %d\t: unboard\n",(*shared_counter)++, id);
	fflush(output);
	sem_post(semaphore_write);

	sem_wait(semaphore_write);
	
	/* Last passenger in car */
	if (*shared_board == C) {
		fprintf(output, "%d\t: P %d\t: unboard last\n",(*shared_counter)++, id);
		fflush(output);
		*shared_board = 1; // reset the value
		sem_post(semaphore_unboard); // open the semaphore for car
	}
	else
		fprintf(output, "%d\t: P %d\t: unboard order %d\n",(*shared_counter)++, id, (*shared_board)++);
		fflush(output);
	sem_post(semaphore_write);
}

void kill_them_all() {
	exit(2);
}

void spawn_passengers(int *id, int P, int PT) {
	for (; *id <= P; (*id)++) {
		process_pid = fork();
		if (process_pid == 0)
			return;
		if (process_pid == -1) {
			fprintf(stderr,"Fork failed, I will try to free memory and kill passengers\n");
			free_shared_memory();
			unlink_semaphores();
			fclose(output);
			kill(0, SIGQUIT);
			exit(2);
		}
		if (PT) {
			srandom(time(NULL));
			usleep((random() % PT) *1000);
		}
	}
	exit(0);
}

void proceed_passengers(int C, int P, int id) {
	/* Car will unlock this semaphore at the beginning, then every process will
	   unlock this semaphore after they use it */
	sem_wait(semaphore_write);
	fprintf(output,"%d\t: P %d\t: started\n", (*shared_counter)++, id);
	fflush(output);
	sem_post(semaphore_write);
	
	sem_wait(semaphore_load); // waiting until car will be ready to load
	passenger_board(id, C); // board the passengers
	
	sem_wait(semaphore_ride); // wait until car will be ready to run
	passenger_unboard(id, C);
	
	/* End of passengers, now they will wait until rest of processes */
	
	sem_wait(semaphore_write);
	(*shared_finished)++;
	if (*shared_finished == P+1) {
		sem_post(semaphore_end); //this id is last in queue
		*shared_finished = 0;
	}
	sem_post(semaphore_write);
	
	sem_wait(semaphore_end);
	sem_post(semaphore_end);
	sem_wait(semaphore_write);
	fprintf(output,"%d\t: P %d\t: finished\n", (*shared_counter)++, id);
	fflush(output);
	(*shared_finished)++;
	if (*shared_finished == P)
		sem_post(semaphore_unboard);
	sem_post(semaphore_write);
}

void proceed_car(int C, int P, int RT) {
	fprintf(output,"%d\t: C 1\t: started\n",(*shared_counter)++);
	fflush(output);
	sem_post(semaphore_write); // unlock write semaphore so first passenger can start
	
	/* Loop for the car */
	for (int i = 0; i < P/C; i++) {
		sem_wait(semaphore_write);
		fprintf(output,"%d\t: C 1\t: load\n",(*shared_counter)++);
		fflush(output);
		sem_post(semaphore_write);	
		
		/* Unlock semaphore for C (capacity of a car) passengers */	
		for (int i = 0; i < C; i++)
			sem_post(semaphore_load);

		sem_wait(semaphore_board); // wait until all passengers will board

		sem_wait(semaphore_write);
		fprintf(output,"%d\t: C 1\t: run\n",(*shared_counter)++);
		fflush(output);
		sem_post(semaphore_write);
		
		/* Sleep random time */
		if (RT != 0) {
			srandom(time(NULL));
			usleep((random() % RT) *1000);
		}
		
		sem_wait(semaphore_write);
		fprintf(output,"%d\t: C 1\t: unload\n",(*shared_counter)++);
		fflush(output);
		sem_post(semaphore_write);
		
		/* Unlock semaphore for C (capacity of a car) passengers */
		for (int i = 0; i < C; i++)
			sem_post(semaphore_ride);
			
		sem_wait(semaphore_unboard); // wait until all passengers leave the car
	}
	
	/* End of a car */
	sem_wait(semaphore_write);
	(*shared_finished)++;
	if (*shared_finished == P+1) {
		sem_post(semaphore_end);
		*shared_finished = 0;
	}
	sem_post(semaphore_write);
	
	sem_wait(semaphore_end);
	sem_post(semaphore_end);
	sem_wait(semaphore_write);
	fprintf(output,"%d\t: C 1\t: finished\n",(*shared_counter)++);
	fflush(output);
	sem_post(semaphore_write);
}



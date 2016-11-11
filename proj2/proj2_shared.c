/*
 * Author: Jiri Matejka
 * Login: xmatej52
 * School: VUT FIT, Brno
 * Date: 02-05-2016
 * Module: IOS-PROJ2 proj2_shared.c
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

sem_t *semaphore_ride,    // Locked while car runs ( passengers must wait)
	  *semaphore_board,   // Locked until all passengers are on board
	  *semaphore_write,   // Locked while using shared memory or printing
	  *semaphore_end,     // Locked until all processes ends
	  *semaphore_unboard, // Locked until all passengers leave car
	  *semaphore_load;    // Locked until car will be ready to accept new passengers
int *shared_counter = NULL,  // Incremented every action of car or passenger
	 shared_counter_id = 0,  // ID of shared_counter
	*shared_board = NULL,    // Number of boarded/unboarded passengers
	 shared_board_id = 0,    // ID of shared_board
	*shared_finished = NULL, // Incremented when some process ends
	 shared_finished_id = 0; // ID of shared_finished

/* 
 * Allocate and initialize all semaphores (semaphore_ride, semaphore_start,
 * semaphore_board, semaphore_write, semaphore_end, semaphore_unboard,
 * semaphore_load)
 */
int set_semaphores () {
	semaphore_ride = sem_open(SEM_RIDE, O_CREAT | O_EXCL, 0666, 0);
	if (semaphore_ride == SEM_FAILED) {
		fprintf(stderr, "Cannot creat semaphores (semaphore_ride failed)\n");
		return 0;
	}

	semaphore_board = sem_open(SEM_BOARD, O_CREAT | O_EXCL, 0666, 0);
	if (semaphore_board == SEM_FAILED) {
		fprintf(stderr, "Cannot creat semaphores (semaphore_board failed)\n");
		sem_close(semaphore_board);
		sem_unlink(SEM_BOARD);
		return 0;
	}

	semaphore_end = sem_open(SEM_END, O_CREAT | O_EXCL, 0666, 0);
	if (semaphore_end == SEM_FAILED) {
		fprintf(stderr, "Cannot creat semaphores (semaphore_end failed)\n");
		sem_close(semaphore_board);
		sem_close(semaphore_ride);
		sem_unlink(SEM_BOARD);
		sem_unlink(SEM_RIDE);
		return 0;
	}
	
	semaphore_write = sem_open(SEM_WRITE, O_CREAT | O_EXCL, 0666, 0);
	if (semaphore_write == SEM_FAILED) {
		fprintf(stderr, "Cannot creat semaphores (semaphore_write failed)\n");
		sem_close(semaphore_board);
		sem_close(semaphore_ride);
		sem_close(semaphore_end);
		sem_unlink(SEM_BOARD);
		sem_unlink(SEM_RIDE);
		sem_unlink(SEM_END);
		return 0;
	}
	
	semaphore_load = sem_open(SEM_LOAD, O_CREAT | O_EXCL, 0666, 0);
	if (semaphore_load == SEM_FAILED) {
		fprintf(stderr, "Cannot creat semaphores (semaphore_load failed)\n");
		sem_close(semaphore_board);
		sem_close(semaphore_ride);
		sem_close(semaphore_end);
		sem_close(semaphore_write);
		sem_unlink(SEM_BOARD);
		sem_unlink(SEM_RIDE);
		sem_unlink(SEM_END);
		sem_unlink(SEM_WRITE);
		return 0;
	}
	
	semaphore_unboard = sem_open(SEM_UNBOARD, O_CREAT | O_EXCL, 0666, 0);
	if (semaphore_unboard == SEM_FAILED) {
		fprintf(stderr, "Cannot creat semaphores (semaphore_unboard failed)\n");
		sem_close(semaphore_board);
		sem_close(semaphore_ride);
		sem_close(semaphore_end);
		sem_close(semaphore_write);
		sem_close(semaphore_load);
		sem_unlink(SEM_BOARD);
		sem_unlink(SEM_RIDE);
		sem_unlink(SEM_END);
		sem_unlink(SEM_WRITE);
		sem_unlink(SEM_LOAD);
		return 0;
	}
	return 1;
}

int set_shared_memory() {
	shared_counter_id = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
	if (shared_counter_id == -1) {
		fprintf(stderr, "Cannot se shared variable: shared_counter_id\n");
		return 0;
	}
	
	shared_counter = (int *) shmat(shared_counter_id, NULL, 0);
	if (shared_counter == NULL) {
		fprintf(stderr, "Cannot se shared variable: *shared_counter\n");
		shmctl(shared_counter_id, IPC_RMID, NULL);
		return 0;
	}
	
	shared_board_id = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
	if (shared_counter_id == -1) {
		fprintf(stderr, "Cannot se shared variable: shared_board_id\n");
		shmctl(shared_counter_id, IPC_RMID, NULL);
		return 0;
	}
	
	shared_board = (int *) shmat(shared_board_id, NULL, 0);
	if (shared_counter == NULL) {
		fprintf(stderr, "Cannot se shared variable: *shared_board\n");
		shmctl(shared_board_id, IPC_RMID, NULL);
		shmctl(shared_counter_id, IPC_RMID, NULL);
		return 0;
	}
	
	shared_finished_id = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
	if (shared_counter_id == -1) {
		fprintf(stderr, "Cannot se shared variable: shared_finished_id\n");
		shmctl(shared_board_id, IPC_RMID, NULL);
		shmctl(shared_counter_id, IPC_RMID, NULL);
		return 0;
	}
	
	shared_finished = (int *) shmat(shared_finished_id, NULL, 0);
	if (shared_counter == NULL) {
		fprintf(stderr, "Cannot se shared variable: *shared_finished\n");
		shmctl(shared_board_id, IPC_RMID, NULL);
		shmctl(shared_counter_id, IPC_RMID, NULL);
		shmctl(shared_finished_id, IPC_RMID, NULL);
		return 0;
	}
	
	/* Setting default values */
	*shared_board = 1;
	*shared_counter = 1;
	*shared_finished = 0;
	return 1;
}

/*
 * Unlink and close all semaphores (semaphore_ride, semaphore_board,     
 * semaphore_write, semaphore_end, semaphore_unboard, semaphore_load)
 */
void unlink_semaphores() {
	sem_close(semaphore_board);
	sem_close(semaphore_ride);
	sem_close(semaphore_end);
	sem_close(semaphore_write);
	sem_close(semaphore_load);
	sem_close(semaphore_unboard);
	sem_unlink(SEM_BOARD);
	sem_unlink(SEM_RIDE);
	sem_unlink(SEM_END);
	sem_unlink(SEM_WRITE);
	sem_unlink(SEM_LOAD);
	sem_unlink(SEM_UNBOARD);
}

/*
 * Free all shared variables (shared_counter, shared_board, shared_finished)
 */
void free_shared_memory() {
	shmctl(shared_counter_id, IPC_RMID, NULL);
	shmctl(shared_board_id, IPC_RMID, NULL);
	shmctl(shared_finished_id, IPC_RMID, NULL);
}


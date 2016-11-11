/*
 * Author: Jiri Matejka
 * Login: xmatej52
 * School: VUT FIT, Brno
 * Date: 02-05-2016
 * Module: IOS-PROJ2 proj2.h
 * gcc version 4.9.2 (Debian 4.9.2-10)
 */
/** 
 * @mainpage
 * @section s1 IOS-project-2
 * Author of this project is Jiri Matejka (login: xmatej52), student of VUT FIT
 * in Brno. Project was last modified 02-05-2016.
 * xmatej52@stud.fit.vutbr.cz
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

/**
 * @defgroup macros Macros
 * Listed macros are used for initialization semaphores
 * @{
 */

/**
 * Used for initialization semaphore_ride
 */
#define SEM_RIDE "/aaa-xmatej52-semphore_ride-proj2-IOS-Hik723zy89R5c"

/**
 * Used for initialization semaphore_board
 */
#define SEM_BOARD "/aaa-xmatej52-semphore_board-proj2-IOS-Hik723zy89R5c"

/**
 * Used for initialization semaphore_end
 */
#define SEM_END "/aaa-xmatej52-semphore_end-proj2-IOS-Hik723zy89R5c"

/**
 * Used for initialization semaphore_write
 */
#define SEM_WRITE "/aaa-xmatej52-semphore_write-proj2-IOS-Hik723zy89R5c"

/**
 * Used for initialization semaphore_load
 */
#define SEM_LOAD "/aaa-xmatej52-semphore_load-proj2-IOS-Hik723zy89R5c"

/**
 * Used for initialization semaphore_unboard
 */
#define SEM_UNBOARD "/aaa-xmatej52-semphore_unboard-proj2-IOS-Hik723zy89R5c"

/** @} */

/**
 * @defgroup global_variables Global variables
 * Functions that allocate or free memory
 * @{
 */

/**
 * @defgroup semaphores Semaphores
 * Semaphores variables
 * @{
 */

/**
 * Unlocked by car, passengers wait until ride is over
 */
extern sem_t *semaphore_ride;

/**
 * Unlocked by last boarded passenger, car waits until all passengers will be boarded
 */
extern sem_t *semaphore_board;

/**
 * Locked/unlocked by any process, that wants print or change value of shared variable
 */
extern sem_t *semaphore_write;

/**
 * Unlocked by last process, all processes (expect spawner) waits until rest of them finish
 */
extern sem_t *semaphore_end;

/**
 * Unlocked by last unboarded passenger, car waits until all passengers will be unboarded
 */
extern sem_t *semaphore_unboard;

/**
 * Unlocked by car, passengers waits until car will be ready to load new passengers
 */
extern sem_t *semaphore_load;

/** @} */

/**
 * @defgroup shared_variables Shared memory
 * Semaphores variables
 * @{
 */


/**
 * Shared variable that is incremented every action of car or passenger
 */
extern int *shared_counter;

/**
 * ID of shared_counter (used for allocation and free)
 */
extern int shared_counter_id;

/**
 * Shared variable used by passenger to control number of boarded/unboarded passengers
 */
extern int *shared_board;

/**
 * ID of shared_board (used for allocation and free)
 */
extern int shared_board_id;

/**
 * Shared variable used by both car and passenger and is incremented every time some of processed finish his action
 */
extern int *shared_finished;

/**
 * ID of shared_finished (used for allocation and free)
 */
extern int shared_finished_id;
/** @} */

/**
 * @defgroup other Other global variables
 * Rest of global variables
 * @{
 */

/**
 * Output of passengers and car
 */
extern FILE *output;

/**
 * Pid of processes
 */
extern pid_t process_pid;

/** @} */

/** @} */

/**
 * @defgroup Shared_memory Shared memory functions
 * Functions that allocate or free memory
 * @{
 */

/**
 * Allocate and initialize all semaphores (semaphore_ride, semaphore_start, semaphore_board, semaphore_write, semaphore_end, semaphore_unboard, semaphore_load)
 * @pre there are no semaphores set by macros SEM_START, SEM_RIDE, SEM_BOARD, SEM_END, SEM_WRITE, SEM_LOAD or SEM_UNBOARD
 * @return 1 if success, otherwise 0
 */
int set_semaphores();

/**
 * Allocate and initialize all shared variables (shared_counter, shared_board, shared_finished)
 * @return 1 if success, otherwise 0
 */
int set_shared_memory();

/**
 * Unlink and close all semaphores (semaphore_ride, semaphore_board, semaphore_write, semaphore_end, semaphore_unboard, semaphore_load)
 */
void unlink_semaphores();

/**
 * Free all shared variables (shared_counter, shared_board, shared_finished)
 */
void free_shared_memory();

/** @} */


/**
 * @defgroup argument_functions Functions for arguments
 * Functions that process arguments
 * @{
 */

/**
 * Check if arguments are valid.
 * @param argc Number of arguments
 * @param **argv array of arguments
 * @return 1 in case of valid arguments, otherwise 0
 */
int check_arguments(int argc, char **argv);

/**
 * Save value of arguments into addresses and check if values are valid.
 * @param *P Number of passengers
 * @param *C Capacity of car
 * @param *PT maximum waiting time for creating new passenger
 * @param *RT maximum length of ride
 * @param **argv
 * @pre There are at least 4 arguments
 * @pre addresses of P, C, PT and RT has been allocated
 * @return 1 in case values are valid, otherwise 0
 */
int get_arguments_values(int *P, int *C, int *PT, int *RT, char **argv);

/**
 * Check if string stands for unsigned number.
 * @param *string string, that will be checked
 * @pre *string is array of chars with zero character at the end
 * @pre address of string has been allocated
 * @return 1 in case string stands for unsigned integer, otherwise 0
 */
int is_uint(char *string);

/**
 * Convert string into integer. Doest not convert negative values.
 * @param *string string, that will be converted
 * @pre *string is array of chars with zero character ('\0') at the end
 * @pre string consists of numbers
 * @pre address of string has been allocated
 * @return integer value, that is represented by string or -1 in case of error
 */
int string_to_uint(char *string);

/** @} */

/**
 * @defgroup roller_coaster Roller coaster functions
 * Functions that work with processes
 * @{
 */
 
/**
 * Board passengers into car.
 * @param id ID of the passenger
 * @param C capacity of a car
 * @pre shared memory (shared_counter, shared_board) has been allocated and initialized
 * @pre semaphore semaphore_write has been allocated and initialized
 */
void passenger_board(int id, int C);

/**
 * Unboard passengers from car.
 * @param id ID of the passenger
 * @param C capacity of a car
 * @pre shared memory (shared_counter, shared_board) has been allocated and initialized
 * @pre semaphore semaphore_write has been allocated and initialized
 */
void passenger_unboard(int id, int C);

/**
 * Processes that receive signal (SIGQUIT) will ends here with exit code 2.
 */
void kill_them_all();

/**
 * Create new passengers (processes) and set their id, kill all passengers in case of error
 * @param *id id of passenger
 * @param P number of passengers that will be created
 * @param PT maximum value of time [ms] to generate new process for passenger
 */
void spawn_passengers(int *id, int P, int PT);

/**
 * Function for all passengers
 * @param C capacity of a car
 * @param P number of passengers
 * @param id id of passenger (every passenger has different id)
 * @pre shared memory (shared_counter, shared_board, shared_finished) has been allocated and initialized
 * @pre semaphore semaphore_write has been allocated and initialized
 */
void proceed_passengers(int C, int P, int id);

/**
 * Function for car
 * @param C capacity of a car
 * @param P number of passengers
 * @param RT maximum value of time [ms] to finish the roller cast track
 * @pre shared memory (shared_counter, shared_board, shared_finished) has been allocated and initialized
 * @pre semaphores (semaphore_ride, semaphore_board, semaphore_write, semaphore_end, semaphore_unboard, semaphore_load) have been allocated and initialized
 */
void proceed_car(int C, int P, int RT);
/** @} */

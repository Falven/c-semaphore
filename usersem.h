#ifndef USERSEM_H_
#define USERSEM_H_

#include <pthread.h>

//	Names will be smaller than 8 characters.
#define NAME_LENGTH 7
//	Assume the global semaphore table can handle 20 semaphores.
#define CAPACITY 20

#define SUCCESS 0
#define FAILURE -1

typedef struct
{
	pthread_cond_t cond;
	char name[NAME_LENGTH];
	int count;
	int ref_count;
	int initial_count;
} sem;

extern pthread_mutex_t mutex;
extern sem * semTable;
extern int nextIndex;
extern int numSems;

/**
 *	Called once by the parent process, it creates global semaphore table in shared memory.
 *	Returns success(0) or failure(-1).
 *	Assume the global semaphore table can handle 20 semaphores.
 */
int sem_init();

/**
 *	Returns a sem_id (non-negative integer) associated with the argument name.
 *	The sem_id is a local identifier used by the calling thread, much like a file descriptor from open().
 *	If the semaphore does not exist when called, the routine will create it.
 *	If the routine creates it, the count of the semaphore is set to the initial_value.
 *	If the semaphore exists, the initial_value is ignored.
 *	Names will be smaller than 8 characters.
 *	Error returns -1.
 */
int sem_open(char *name, int initial_value);

/**
 *	Releases the semid so that it is no longer in use by this thread.
 *	If no other threads reference the semaphore anymore, the semaphore is removed from the table.
 *	Returns success (0) or failure (-1).
 */
int sem_close(int semid);

/**
 *	If the referenced semaphore is not locked and there are no tasks already waiting for the semaphore, the function returns with the semaphore locked.
 *	If the referenced semaphore is locked, the process is added to the wait queue.
 *	Returns success (0) or failure (-1).
 */
int sem_wait(int semid);

/**
 *	If the referenced semaphore has waiting processes, the first one on the queue is restarted.
 *	If the referenced semaphore has no waiting processes, the semaphore is unlocked.
 *	Returns success (0) or failure (-1).
 */
int sem_signal(int semid);

/**
 *	Releases the global semaphore table.
 *	Returns success (0) or failure (-1).
 */
int sem_destroy();

/**
 *	Returns the depth of the queue of the semaphore.
 */
int sem_depth(int semid);
#endif

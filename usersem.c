#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "usersem.h"

pthread_mutex_t mutex;
sem* semTable;
int nextIndex;
int numSems; // TODO sizeof array

/**
 *	Called once by the parent process, it creates global semaphore table in shared memory.
 *	Returns success(0) or failure(-1).
 */
int sem_init()
{
	//	If the referenced semaphore is not locked
	pthread_mutex_lock(&mutex);

	nextIndex = 0;
	numSems = 0;
	semTable = (sem*) malloc(sizeof(sem) * CAPACITY);
	
	int i;
	for(i = 0; i < CAPACITY; i++)
	{
		sem* curSem = &semTable[i];
		pthread_cond_init(&(curSem->cond), NULL);
	}

	pthread_mutex_unlock(&mutex);
	return semTable && pthread_mutex_init(&mutex, NULL) ? SUCCESS : FAILURE;
}

/**
 *	Returns a sem_id (non-negative integer) associated with the argument name.
 *	The sem_id is a local identifier used by the calling thread, much like a file descriptor from open().
 *	If the semaphore does not exist when called, the routine will create it.
 *	If the routine creates it, the count of the semaphore is set to the initial_value.
 *	If the semaphore exists, the initial_value is ignored.
 *	Names will be smaller than 8 characters.
 *	Error returns -1.
 *	Use condition variables to do queuing.
 */
int sem_open(char* name, int initial_value)
{
	//	If the referenced semaphore is not locked
	pthread_mutex_lock(&mutex);

	// If the semaphore exists, the initial_value is ignored.
	int i;
	sem* curSem;
	for (i = 0; i < CAPACITY; i++)
	{
		curSem = &semTable[i];
		if (strcmp(curSem->name, name) == 0)
		{
			curSem->ref_count++;
			pthread_mutex_unlock(&mutex);
			return i;
		}
	}

	curSem = &semTable[nextIndex];
	//	If the semaphore does not exist when called, the routine will create it.
	strcpy(curSem->name, name);
	//	If the routine creates it, the count of the semaphore is set to the initial_value.
	curSem->count = initial_value;
	curSem->initial_count = initial_value;
	curSem->ref_count++;
	numSems++;

	pthread_mutex_unlock(&mutex);
	return nextIndex++;
}

/**
 *	Releases the semid so that it is no longer in use by this thread.
 *	If no other threads reference the semaphore anymore, the semaphore is removed from the table.
 *	Returns success (0) or failure (-1).
 */
int sem_close(int semid)
{
	//	If the referenced semaphore is not locked
	pthread_mutex_lock(&mutex);

	sem* curSem = &semTable[semid];
	
	// Release this threads reference.
	pthread_cond_signal(&(curSem->cond));
	curSem->ref_count--;

	// If there are no more references in that sem. Make it Empty.
	if (curSem->ref_count == 0)
	{
		//	Reset semaphore
		curSem->name[NAME_LENGTH];
		curSem->count = 0;
		curSem->ref_count = 0;
		pthread_cond_destroy(&(curSem->cond));
		pthread_cond_init(&(curSem->cond), NULL);
	}

	pthread_mutex_unlock(&mutex);
	return SUCCESS;
}

/**
 *	If the referenced semaphore is not locked and there are no tasks already waiting for the semaphore,
 *	the function returns with the semaphore locked.
 *	If the referenced semaphore is locked, the process is added to the wait queue.
 *	Returns success (0) or failure (-1).
 *	Use mutex to protect variables
 *	Must guarantee that no two processes can execute the wait() and signal() on the same semaphore at the same time.
 */
int sem_wait(int semid)
{
	//	If the referenced semaphore is not locked
	pthread_mutex_lock(&mutex);

	sem* curSem = &semTable[semid];
	//	If the referenced semaphore is locked, the process is added to the wait queue.
	if (curSem->count <= 0 && (curSem->count <= curSem->initial_count))
	{
		//	place the process invoking the operation on the appropriate waiting queue.
		pthread_cond_wait(&(curSem->cond), &mutex);
		curSem->count--;
	}

	pthread_mutex_unlock(&mutex);
	return SUCCESS;
}

/**
 *	If the referenced semaphore has waiting processes, the first one on the queue is restarted.
 *	If the referenced semaphore has no waiting processes, the semaphore is unlocked.
 *	Returns success (0) or failure (-1).
 *	Must guarantee that no two processes can execute the wait() and signal() on the same semaphore at the same time.
 */
int sem_signal(int semid)
{
	//	If the referenced semaphore is not locked
	pthread_mutex_lock(&mutex);

	sem* curSem = &semTable[semid];
	// If the referenced semaphore has waiting processes, the first one on the queue is restarted.
	if ((curSem->count > 0) && (curSem->count <= curSem->initial_count))
	{
		// remove one of processes in the waiting queue and place it in the ready queue
		pthread_cond_signal(&(curSem->cond));
		curSem->count++;
	}

	pthread_mutex_unlock(&mutex);
	return SUCCESS;
}

/**
 *	Releases the global semaphore table.
 *	Returns success (0) or failure (-1).
 */
int sem_destroy()
{
	//	If the referenced semaphore is not locked
	pthread_mutex_lock(&mutex);

	int i;
	for (i = 0; i < CAPACITY; i++)
	{
		sem* curSem = &semTable[i];
		if (!pthread_cond_destroy(&(curSem->cond)))
		{
			pthread_mutex_unlock(&mutex);
			return FAILURE;
		}
	}

	free(semTable);

	if (!pthread_mutex_destroy(&mutex))
	{
		pthread_mutex_unlock(&mutex);
		return FAILURE;
	}
	pthread_mutex_unlock(&mutex);
	return SUCCESS;
}

/**
 *	Returns the depth of the queue of the semaphore.
 */
int sem_depth(int semid)
{
	pthread_mutex_lock(&mutex);
	int refCount = semTable[semid].ref_count;
	pthread_mutex_unlock(&mutex);
	return refCount;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdbool.h>
#include "usersem.h"

#define MAIN_SLEEP_S 30
#define MAIN_SLEEP_INCREMENT_S 5
#define MIN_WRITERS 1
#define MAX_WRITERS 10
#define MIN_READERS 1
#define MAX_READERS 100

static bool done;
static bool verbose;
static bool dbg;
static int sleepVal;
static int readCount;
static int totalReadNum;
static int writeCount;
static int totalWriteNum;
//	'r' protects the reading operation.
static int r;
//	'w' protects the writing operation.
static int w;
//	mutex_1 protects the readCount variable.
static int mutex_1;
//	mutex_2 protects the writeCount variable.
static int mutex_2;
//	mutex_3 protects r/w operations.
static int mutex_3;

typedef struct
{
	int id;
} thread_data;

/*
 *	Prints the usage statement for this program.
 */
void printUsage()
{
	fprintf(stderr, "Usage: P3 -r < r > -w < w > -v\n");
}

/*
 *	Thread sleeps for the given number of milliseconds.
 */
void sleepMS(int milliseconds)
{
	int nanoseconds = milliseconds / 100000L;
	struct timespec tim, tim2;
	tim.tv_sec = 0;
	tim.tv_nsec = nanoseconds;
	nanosleep(&tim, &tim2);
}

/*
 *	Generate a random number between 0 and i - 1.
 */
int getRandNum(int i)
{
	struct timeval tim;
	gettimeofday(&tim, NULL);
	srand((unsigned int) tim.tv_usec);
	return rand() % i;
}

void* WriterThread(void* t)
{
	thread_data* pData = (thread_data*) t;
	int tid = pData->id;
	printf("Writer thread, ID = %d: started.\n", tid);

	while (!done)
	{
		//	Generate a random number between 100 and 1000
		int sleepTime = (100 + getRandNum(999));

		//	Sleep for that number of milliseconds.
		sleepMS(sleepTime);

		//	Generate another random number between 10 and 50.
		int rNum = (10 + getRandNum(49));

		/*	Perform the actions necessary to begin writing. */
		int result = -1;

		if (verbose)
		{
			//	If -v specified, print the depth of the queue of the semaphore protecting writecount before acquiring it and the value written. 
			int depth = sem_depth(mutex_2);
			printf("Depth of the queue of mutex_2 = %d.\n", depth);
		}

		if (dbg)
		{
			printf("Writer thread, ID = %d: sem_wait(mutex_2).\n", tid);
		}
		//		wait(mutex_2);
		result = sem_wait(mutex_2);

		//	Increment a global variable indicating the number of writes that have been done.
		//		writeCount := writeCount + 1;
		writeCount++;
		//		if writeCount = 1 then wait(r);
		if (writeCount == 1)
		{
			if (dbg)
			{
				printf("Writer thread, ID = %d: sem_wait(r).\n", tid);
			}
			result = sem_wait(r);
		}

		if (dbg)
		{
			printf("Writer thread, ID = %d: sem_signal(mutex_2).\n", tid);
		}
		//		signal(mutex_2);
		result = sem_signal(mutex_2);

		if (dbg)
		{
			printf("Writer thread, ID = %d: sem_wait(w).\n", tid);
		}
		//		wait(w);
		result = sem_wait(w);

		/*	Perform the actions necessary to begin writing. */

		/*	Perform Writing	*/

		//	Safely store that number in sleepVal.
		//	    writing is performed
		sleepVal = rNum;

		//	Increment a global variable indicating the number of writes that have been done.. 
		totalWriteNum++;

		//	Sleep for sleepVal number of milliseconds.
		sleepMS(sleepVal);

		/* Perform Writing */

		if (dbg)
		{
			printf("Writer thread, ID = %d: sem_signal(w).\n", tid);
		}
		/*	Perform the actions necessary to stop writing.	*/
		//		signal(w);
		result = sem_signal(w);

		if (dbg)
		{
			printf("Writer thread, ID = %d: sem_wait(mutex_2).\n", tid);
		}
		//		wait(mutex_2);
		result = sem_wait(mutex_2);

		//		writeCount := writeCount - 1;
		writeCount--;
		//		if writeCount = 0 then signal(r);
		if (writeCount == 0)
		{
			if (dbg)
			{
				printf("Writer thread, ID = %d: sem_signal(r).\n", tid);
			}
			result = sem_signal(r);
		}

		if (dbg)
		{
			printf("Writer thread, ID = %d: sem_signal(mutex_2).\n", tid);
		}
		//		signal(mutex_2);
		result = sem_signal(mutex_2);

		/*	Perform the actions necessary to stop writing.	*/
	}

	printf("Writer thread, ID = %d: finished.\n", tid);
	pthread_exit(NULL);
}

void* ReaderThread(void *t)
{
	thread_data* pData = (thread_data*) t;
	int tid = pData->id;
	printf("Reader thread, ID = %d: started.\n", tid);

	while (!done)
	{
		/*	Perform the actions necessary to begin reading. */
		int result = -1;
		if (dbg)
		{
			printf("Reader thread, ID = %d: sem_wait(mutex_3).\n", tid);
		}
		//		wait(mutex_3);
		result = sem_wait(mutex_3);

		if (dbg)
		{
			printf("Reader thread, ID = %d: sem_wait(r).\n", tid);
		}
		//		wait(r);
		result = sem_wait(r);

		if (verbose)
		{
			//	If -v specified, print the depth of the queue of the mutex semaphore protecting readCount before acquiring it and the value read.
			int depth = sem_depth(mutex_1);
			printf("Depth of the queue of mutex_1 = %d.\n", depth);
		}

		if (dbg)
		{
			printf("Reader thread, ID = %d: sem_wait(mutex_1).\n", tid);
		}
		//		wait(mutex_1);
		result = sem_wait(mutex_1);

		//	Increment a global variable indicating the number of reads performed.
		//		readCount := readCount + 1;
		readCount++;

		//		if readCount = 1 then wait(w);
		if (readCount == 1)
		{
			result = sem_wait(w);
		}

		if (dbg)
		{
			printf("Reader thread, ID = %d: sem_signal(mutex_1).\n", tid);
		}
		//		signal(mutex_1);
		result = sem_signal(mutex_1);

		if (dbg)
		{
			printf("Reader thread, ID = %d: sem_signal(r).\n", tid);
		}
		//		signal(r);
		result = sem_signal(r);

		if (dbg)
		{
			printf("Reader thread, ID = %d: sem_signal(mutex_3).\n", tid);
		}
		//		signal(mutex_3);
		result = sem_signal(mutex_3);

		/*	Perform the actions necessary to begin reading. */

		/* Reading is Performed */

		//	Safely read the value of sleepval.
		int sleep = sleepVal;

		//	Increment a global variable indicating the number of reads performed.
		totalReadNum++;

		//	Sleep for sleepVal milliseconds.
		sleepMS(sleepVal);

		/* Reading is Performed */

		/* Perform the actions necessary to end reading. */

		if (dbg)
		{
			printf("Reader thread, ID = %d: sem_wait(mutex_1).\n", tid);
		}
		//		wait(mutex_1);
		result = sem_wait(mutex_1);

		//		readCount := readCount - 1;
		readCount--;
		//		if readCount = 0 then signal(w);
		if (readCount == 0)
		{
			if (dbg)
			{
				printf("Reader thread, ID = %d: sem_signal(w).\n", tid);
			}
			result = sem_signal(w);
		}

		if (dbg)
		{
			printf("Reader thread, ID = %d: sem_signal(mutex_1).\n", tid);
		}
		//		signal(mutex_1);
		sem_signal(mutex_1);

		/* Perform the actions necessary to end reading. */

		//	Generate another random number between 100 and 500.
		int rNum = (100 + getRandNum(499));

		//	Sleep for that number of milliseconds.
		sleepMS(rNum);
	}
	printf("Reader thread, ID = %d: finished.\n", tid);
	pthread_exit(NULL);
}

int main(int argc, char**argv)
{
	//	-r defines the number of reader threads
	int rflag = 0;
	int rValue = 0;

	//	-w defines the number of writer threads
	int wflag = 0;
	int wValue = 0;

	//	-v optional. turns on verbose.
	verbose = false;

	//	-d optional. turns on debug.
	dbg = false;

	char * cvalue = NULL;
	int index;
	int c;

	while ((c = getopt(argc, argv, "r:w:v::d::")) != -1)
	{
		switch (c)
		{
			case 'r':
				rflag = 1;
				rValue = atoi(optarg);
				break;

			case 'w':
				wflag = 1;
				wValue = atoi(optarg);
				break;

			case 'v':
				verbose = true;
				break;

			case 'd':
				dbg = true;
				break;

			case '?':
				printUsage();
				return 1;

			default:
				abort();
		}
	}

	//	Your program should be able to handle up to 1 to 10 writers and 1 to 100 readers (all numbers inclusive).
	if (rValue < MIN_READERS || rValue > MAX_READERS)
	{
		fprintf(stderr,
				"Invalid number of Readers provided. -r must be between %d and %d inclusive.",
				MIN_READERS, MAX_READERS);
	}

	if (wValue < MIN_WRITERS || wValue > MAX_WRITERS)
	{
		fprintf(stderr,
				"Invalid number of Writers provided. -w must be between %d and %d inclusive.",
				MIN_WRITERS, MAX_WRITERS);
	}

	int threadCount = rValue + wValue;

	//	Your program will declare a global integer sleepVal that will be shared by all readers and writer threads
	//	sleepVal will be protected by whatever semaphores necessary from your library to implement concurrency.
	sleepVal = 10;
	readCount = 0;
	writeCount = 0;
	totalReadNum = 0;
	totalWriteNum = 0;

	printf("Main: Starting %d readers and %d writers.\n", rValue, wValue);

	sem_init();

	//	Semaphore that allows 1 reader to update readCount at a time.
	mutex_1 = sem_open("mutex_1", 1);

	//	Semaphore that allows 1 writer to update writeCount at a time.
	mutex_2 = sem_open("mutex_2", 1);

	//	Semaphore that allows 1 reader to read at a time.
	r = sem_open("r", 1);

	//	Semaphore that allows 1 writer to write at a time.
	w = sem_open("w", 1);

	//	Semaphore that protects r/w operations.
	mutex_3 = sem_open("mutex_3", 1);

	thread_data* pthreadData = malloc(threadCount * sizeof(thread_data));
	if (!pthreadData)
	{
		fprintf(stderr, "Could not allocate thread data.\n");
		return EXIT_FAILURE;
	}

	done = false;

	//	pthread array to hold all reader and writer threads.
	pthread_t thread[threadCount];

	//	Initialize and set thread attributes
	pthread_attr_t attr;
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	pthread_attr_init(&attr);

	//	Main should spawn the readers and writers (passing each the ordinal of its creation as a thread id)
	int id;
	for (id = 0; id < wValue; id++)
	{
		pthreadData[id].id = id;
		pthread_create(&thread[id], &attr, WriterThread,
				(void *) &pthreadData[id]);
	}

	for (id = wValue; id < threadCount; id++)
	{
		pthreadData[id].id = id;
		pthread_create(&thread[id], &attr, ReaderThread,
				(void *) &pthreadData[id]);
	}

	//	Every 5 seconds, main should print the number of reads and writes that have occurred.
	int nrw = 0;
	int sleepTime = MAIN_SLEEP_S;
	while (sleepTime > 0)
	{
		sleep(MAIN_SLEEP_INCREMENT_S);
		sleepTime -= MAIN_SLEEP_INCREMENT_S;

		printf("Main: At time %d s - Total Number of Reads = %d.\n",
				(nrw + 1) * MAIN_SLEEP_INCREMENT_S, totalReadNum);
		printf("Main: At time %d s - Total Number of Writes = %d.\n",
				(nrw + 1) * MAIN_SLEEP_INCREMENT_S, totalWriteNum);
		nrw++;
	}

	printf("Main: Printed Total Reads and Writes %d/%d = %d times.\n",
	MAIN_SLEEP_S, MAIN_SLEEP_INCREMENT_S, nrw);

	//	then wait for 30 seconds and signal the threads to finish.
	done = true;

	//	Free attribute and wait for the other threads
	void* status;
	for (id = 0; id < threadCount; id++)
	{
		pthread_join(thread[id], &status);
	}

	printf("Main: program exiting after joining threads.\n");

	//	Clean up and exit
	pthread_attr_destroy(&attr);
	sem_close(mutex_1);
	sem_close(mutex_2);
	sem_close(r);
	sem_close(w);
	sem_close(mutex_3);
	sem_destroy();
	free(pthreadData);

	printf("Main: Resources successfully freed.\nGoodbye.\n");
	return EXIT_SUCCESS;
}

# Readers and Writers
###Custom Semaphore with readers and writers test

04/25/2014


##main.c:

Contains the main routine of the application, PA3, which tests the semaphore
using ReaderThread and WriterThread routines. The main
routine checks for the right number of parameters and spawns the
readers and writers which read and write to a sleeptime variable
synchronized fashion, utilizing semaphores, wait and notify to synchronize
reading from and writing to the sleeptime, until they are told to finish
via deferred execution the main routine. Main also continues to print the
total number of reads and writes from each thread every 5 seconds.
Main then joins with such threads, prints an exit message, and finishes execution.
	
##usersem.h:

The declaration for a thread safe semaphore implementation with a capacity of 20 items.

##usersem.c:

The declaration for a thread safe semaphore implementation with a capacity of 20 items.
Allows users to initialize, open, close, wait, signal, destroy and get check the depth in the thread queue.
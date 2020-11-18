/* shmp1.cpp */

#include "registration.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <memory.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h> 		 // POSIX semaphore
#include <pthread.h>         // Threads

using namespace std;

CLASS myclass = { "4321", "082620", "Operating Systems", 20 };

#define NCHILD	3
#define SEMNAME "shmpSem"

int	shm_init( void * , sem_t *);
void	wait_and_wrap_up( int [], void *, int , sem_t *);
void	rpterror( char *, char * );

int main(int argc, char *argv[])
{
	int 	child[NCHILD], i, shmid;
	void	*shm_ptr;
	char	ascshmid[10], pname[14];
	sem_t *sem;

	shmid = shm_init(shm_ptr, sem);
	sprintf (ascshmid, "%d", shmid);

	for (i = 0; i < NCHILD; i++) {
		child[i] = fork();
		switch (child[i]) {
		case -1:
			sprintf (pname, "child%d", i+1);
rpterror ((char *)"fork failed", pname);
			exit(1);
		case 0:
			sprintf (pname, "shmc%d", i+1);
			execl("shmc1", pname, ascshmid, SEMNAME, (char *)0);
			rpterror ((char *)"execl failed", pname);
			exit (2);
		}
	}
	wait_and_wrap_up (child, shm_ptr, shmid, sem);
    return 0;
}

int shm_init(void *shm_ptr, sem_t *sem)
{
	int	shmid;

    // shmget will create a shared memory identifier.
    // First parameter is the key. The key will be created with ftok
    // Second parameter is the size in bytes of the shared memory
    // Last parameter are the flags. In this case, 0600 = the owner can read/write to the shared memory. IPC_CREAT creates the data structure.
	shmid = shmget(ftok(".",'u'), sizeof(CLASS), 0600 | IPC_CREAT);
	if (shmid == -1) {
		perror ("shmget failed");
		exit(3);
	}
    // shmat will attach the shared memory to the address space of this process.
    // First parameter is the id of the shared memory
    // Second parameter is the address at which the calling thread would like the shared memory attached
    // Last parameter are the flags. 0 = no flags.
	shm_ptr = shmat(shmid, (void * ) 0, 0);
	if (shm_ptr == (void *) -1) {
		perror ("shmat failed");
		exit(4);
	}

    // memcpy will copy the num bytes of the source memory block to the destination memory block
    // First parameter is the destination
    // Second parameter is the source
    // Last parameter is the num of bytes to copy.
	memcpy (shm_ptr, (void *) &myclass, sizeof(CLASS) );

	// sem_open opens a connection between a named semaphore and a process.
	// First parameter is the name of the semaphore. You can name this anything.
	// Second parameter are the flags. In this case O_CREAT will create a semaphore.
	// Third parameter is the file permission of the second parameter, in this case for the O_CREAT. 0600 = the owner can read/write the semaphore.
	// Last parameter is the initial value that will be set to the semaphore, in this case 1.
	sem = sem_open(SEMNAME, O_CREAT, 0600, 1);
    if(sem == SEM_FAILED) {
        perror("sem_open failed");
        exit(1);
    }


	return (shmid);
}

void wait_and_wrap_up(int child[], void *shm_ptr, int shmid, sem_t *sem)
{
	int wait_rtn, w, ch_active = NCHILD;

	while (ch_active > 0) {
		wait_rtn = wait( (int *)0 );
		for (w = 0; w < NCHILD; w++)
			if (child[w] == wait_rtn) {
				ch_active--;
				break;
			}
	}
	cout << "Parent removing shm" << endl;
    // shmdt detaches the shared memory segment from the process.
    // parameter: the address of the shared memory segment
	shmdt (shm_ptr);
    // shmctl performs  a variety of shared memory control operations
    // First parameter is the shared memory id
    // Second parameter are the commands. In this case IPC_RMID will remove the shared memory segment and data structure from the system.
    // Last parameter is the pointer to the shmid_ds structure.
	shmctl (shmid, IPC_RMID, (struct shmid_ds *) 0);

	// sem_unlink removes the semaphore from the system by name. Parameter = the name of the semaphore.
    sem_unlink(SEMNAME);
	// sem_close will close a named sempahore that was opened by sem_open. Parameter = the pointer to the semaphore.
    sem_close(sem);

	exit (0);
}

void rpterror(char *string, char *pname)
{
	char errline[50];

	sprintf (errline, "%s %s", string, pname);
	perror (errline);
}

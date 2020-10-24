/* shmp1.cpp */

#include "registration.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <memory.h>

using namespace std;

CLASS myclass = { "4321", "082620", "Operating Systems", 20 };

#define NCHILD	3

int	shm_init( void * , int &);
void	wait_and_wrap_up( int [], void *, int , int);
void	rpterror( char *, char * );

int main(int argc, char *argv[])
{
	int 	child[NCHILD], i, shmid, semid;
	void	*shm_ptr;
	char	ascshmid[10], pname[14], ascsemid[50];
	int mutex = 0;

	shmid = shm_init(shm_ptr, semid);
	sprintf (ascshmid, "%d", shmid);
	sprintf (ascsemid, "%d", semid);

	for (i = 0; i < NCHILD; i++) {
		child[i] = fork();
		switch (child[i]) {
		case -1:
			sprintf (pname, "child%d", i+1);
			rpterror ((char *)"fork failed", pname);
			exit(1);
		case 0:
			sprintf (pname, "shmc%d", i+1);
			execl("shmc1", pname, ascshmid, ascsemid, (char *)0);
			rpterror ((char *)"execl failed", pname);
			exit (2);
		}
	}
	wait_and_wrap_up (child, shm_ptr, shmid, semid);
    return 0;
}

int shm_init(void *shm_ptr, int &semid)
{
	int	shmid;

	key_t key = ftok(".",'u');

    // shmget will create a shared memory identifier.
    // First parameter is the key. The key will be created with ftok
    // Second parameter is the size in bytes of the shared memory
    // Last parameter are the flags. In this case, 0600 = the owner can read/write to the shared memory. IPC_CREAT creates the data structure.
	shmid = shmget(key, sizeof(CLASS), 0600 | IPC_CREAT);
	if (shmid == -1) {
		perror ("shmget failed");
		exit(3);
	}

	// setget will create a set of semaphores and return a semid
	// First parameter is the key generated with ftok.
	// Second parameter is the amount of semaphores to create
	// Last parameter are the flags. In this case, 0600 = the owner can read/write to the shared memory. IPC_CREAT creates the data structure.
	semid = semget(key, 3, IPC_CREAT | 0600);
	if(semid == -1)
	{
		perror("semget failed");
		exit(4);
	}

	// Set value for each semaphore
	for(int i = 0; i < NCHILD; i++) {
		// With semctl you can control the semaphore.
		// First parameter is the semid that's generated with semget.
		// Second parameter is the semaphore number
		// Third parameter is the control operation, in this case SETVAL. This will set the senum.val to whatever is the 4th parameter (in this case 1).
		// The last parameter is used for the control operation above.
		if(semctl(semid, i, SETVAL, 1) == -1) {
			perror("semctl failed");
			exit(4);
		}
	}

    // shmat will attach the shared memory to the address space of this process.
    // First parameter is the id of the shared memory
    // Second parameter is the address at which the calling thread would like the shared memory attached
    // Last parameter are the flags. 0 = no flags.
	shm_ptr = shmat(shmid, (void * ) 0, 0);
	if (shm_ptr == (void *) -1) {
		perror ("shmat failed");
		exit(5);
	}

    // memcpy will copy the num bytes of the source memory block to the destination memory block
    // First parameter is the destination
    // Second parameter is the source
    // Last parameter is the num of bytes to copy.
	memcpy (shm_ptr, (void *) &myclass, sizeof(CLASS) );
	return (shmid);
}

void wait_and_wrap_up(int child[], void *shm_ptr, int shmid, int semid)
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

    // shmctl performs a variety of shared memory control operations
    // First parameter is the shared memory id
    // Second parameter are the commands. In this case IPC_RMID will remove the shared memory segment and data structure from the system.
    // Last parameter is the pointer to the shmid_ds structure.
	shmctl (shmid, IPC_RMID, (struct shmid_ds *) 0);

	// With semctl you can control the semaphore.
	// First parameter is the semid that's generated with semget.
	// Second parameter is the semaphore number
	// Third parameter is the control operation, in this case IPC_RMID. This will remove the semaphore from the system.
	semctl(semid, 0, IPC_RMID);
	
	exit (0);
}

void rpterror(char *string, char *pname)
{
	char errline[50];

	sprintf (errline, "%s %s", string, pname);
	perror (errline);
}

/* shmc1.cpp */

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

CLASS *class_ptr;
void 	*memptr;
char	*pname;
int	shmid, ret;
void rpterror(char *), srand(), perror(), sleep();
void sell_seats();

sem_t *sem;
char *semName;

int main(int argc, char* argv[])
{
	if (argc < 2) {
		fprintf (stderr, "Usage:, %s shmid\n", argv[0]);
		exit(1);
	}

	pname = argv[0];
	sscanf (argv[1], "%d", &shmid);
	semName = argv[2];

    // shmat will attach the shared memory to the address space of this process.
    // First parameter is the id of the shared memory
    // Second parameter is the address at which the calling thread would like the shared memory attached
    // Last parameter are the flags. 0 = no flags.
	memptr = shmat (shmid, (void *)0, 0);
	if (memptr == (char *)-1 ) {
		rpterror ((char *)"shmat failed");
		exit(2);
	}

	class_ptr = (struct CLASS *)memptr;

	// sem_open opens a connection between a named semaphore and a process.
	// First parameter is the name of the semaphore. The process gets this from the parent.
	// Last parameter is the initial value that will be set to the semaphore, in this case 1.
    if ((sem = sem_open(semName, 1)) == SEM_FAILED) {
        perror("sem_open failed");
        exit(1);
    }

	sell_seats();

    // shmdt detaches the shared memory segment from the process.
    // parameter: the address of the shared memory segment
	ret = shmdt(memptr);
	exit(0);
    return 0;
}

void sell_seats() 
{
	int all_out = 0;

	srand ( (unsigned) getpid() );
	while ( !all_out) {   /* loop to sell all seats */

		// sem_wait decrements the value of the semaphore by one if its value is greater than 0. If the value is 0, the current thread will block until its greater than 0.
		// Parameter is the pointer to the semaphore.
        sem_wait(sem);

		if (class_ptr->seats_left > 0) {
			sleep ( (unsigned)rand()%5 + 1);
			class_ptr->seats_left--;
			sleep ( (unsigned)rand()%5 + 1);
			cout << pname << " SOLD SEAT -- " 
			     << class_ptr->seats_left << " left" << endl;
		}
		else {
			all_out++;
			cout << pname << " sees no seats left" << endl;
		}

		// sem_post unlocks the semaphore referenced by the sem pointer. It will increase its value from 0 to 1.
		// Parameter is the pointer to the semaphore.
        sem_post(sem);
		sleep ( (unsigned)rand()%10 + 1);
	}
}

void rpterror(char* string)
{
	char errline[50];

	sprintf (errline, "%s %s", string, pname);
	perror (errline);
}

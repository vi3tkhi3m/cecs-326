/* shmc1.cpp */

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

CLASS *class_ptr;
void 	*memptr;
char	*pname;
int	shmid, ret, semid;
struct sembuf oper0;
void rpterror(char *), srand(), perror(), sleep();
void sell_seats();

struct sembuf unlock = {
	0, 1, SEM_UNDO
};

struct sembuf lock = {
	0, -1, SEM_UNDO
};

int main(int argc, char* argv[])
{
	if (argc < 3) {
		fprintf (stderr, "Usage:, %s shmid\n", argv[0]);
		exit(1);
	}

	pname = argv[0];
	sscanf (argv[1], "%d", &shmid);
	sscanf (argv[2], "%d", &semid);
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
		// semop will perform operations on the sempahore set.
		// First parameter is the semid that's generated with semget from the parent.
		// Second parameter is the operation. In this case locking the semaphore.
		// Last parameter is the amount of semaphores operations. In this case 1.
		if (semop(semid, &lock, 1) == -1) {
			printf("error locking semaphore \n");
			exit(1);
		}

		if (class_ptr->seats_left > 0) {
			sleep ( (unsigned)rand()%1 + 1);
			
			class_ptr->seats_left--;

			sleep ( (unsigned)rand()%1 + 1);
			cout << pname << " SOLD SEAT -- " 
			     << class_ptr->seats_left << " left" << endl;
		}
		else {
			all_out++;
			cout << pname << " sees no seats left" << endl;
		}

		// semop will perform operations on the sempahore set.
		// First parameter is the semid that's generated with semget from the parent.
		// Second parameter is the operation. In this case unlocking the semaphore.
		// Last parameter is the amount of semaphores operations. In this case 1.
		if (semop(semid, &unlock, 1) == -1) {
			printf("error unlocking semaphore \n");
			exit(1);
		}
		sleep ( (unsigned)rand()%10 + 1);
	}
}

void rpterror(char* string)
{
	char errline[50];

	sprintf (errline, "%s %s", string, pname);
	perror (errline);
}

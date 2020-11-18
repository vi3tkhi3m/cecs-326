#include "accounts.h"
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h> 		 // POSIX semaphore
#include <pthread.h>         // Threads

void withdraw(int from, int amount);
void deposit(int to, int amount);

struct ACCOUNTS *accountsPtr;
sem_t *sem;
sem_t *sem1;
void 	*memptr;
int ret;

int main(int argc, char *argv[])
{
	if(argc<6) {
        printf("Please provide the correct amount of arguments!\n"); 
        printf("[shared_memory_name] [sem1_name] [sem2_name] [bank_from] [bank_to] [transfer_amount]\n"); 
    }  else {
        char* sharedMemorySegmentName = argv[0];
        char* semName1 = argv[1];
        char* semName2 = argv[2];
        int from = atoi(argv[3]);
        int to = atoi(argv[4]);
        int amount = atoi(argv[5]);

        // Give the file a size in bytes
        const int SIZE = 4096; 
    
        // shared memory file descriptor
        int shm_fd; 
    
        // shm_open creates or opens a POSIX shared memory object
        // First parameter is the name of the shared memory
        // Second parameter is the flags you can provide. YOu can provide multiple flags like creating, make it read only, read-write etc.
        // Last parameter sets the permissions of the shared memory object
        shm_fd = shm_open(sharedMemorySegmentName, O_RDWR, 0666); 
    
        // mmap establishes a mapping between an address space of a process and a file associated with the file descriptor (shm_fd)
        // The first parameter specifies the starting address of the memory region to be mapped.
        // The second parameter specifies the length
        // The third parameter specifies the access permissions, in this case read or write
        // The fourth parameter specifies the attributes of the mapped region. MAP_SHARED will make modifications to the mapped region visible to other processes that also mapped the same region.
        // The fifth parameter specifies the file descriptor
        // The last parameter specifies the file byte offset at which the mapping starts.
        accountsPtr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0); 

        // sem_open opens a connection between a named semaphore and a process.
        // First parameter is the name of the semaphore. You can name this anything.
        // Last parameter is the initial value that will be set to the semaphore, in this case 1.
        if ((sem = sem_open(semName1, 1)) == SEM_FAILED) {
            perror("sem_open failed");
            exit(1);
        }
        if ((sem1 = sem_open(semName2, 1)) == SEM_FAILED) {
            perror("sem_open failed");
            exit(1);
        }

        // sem_wait decrements the value of the semaphore by one if its value is greater than 0. If the value is 0, the current thread will block until its greater than 0.
		// Parameter is the pointer to the semaphore.
        sem_wait(sem);
        sem_wait(sem1);

        withdraw(from, amount);
        deposit(to, amount);

        // sem_post unlocks the semaphore referenced by the sem pointer. It will increase its value from 0 to 1.
		// Parameter is the pointer to the semaphore.
        sem_post(sem1);
        sem_post(sem);
        
        // Removes the mapped shared memory segment from the address space of the process
        // First parameter is the starting address of the region.
        // Second parameter is the length of the address range.
        if (munmap(accountsPtr, SIZE) == -1) {
            printf("cons: Unmap failed: %s\n", strerror(errno));
            exit(1);
        }

        // Closes the shared memory segment as if it was a file
        // Parameter: the file descriptor
        if (close(shm_fd) == -1) {
            printf("cons: Close failed: %s\n", strerror(errno));
            exit(1);
        }

        exit(0);

    }

    return 0;
}

void withdraw(int from, int amount) {
    accountsPtr->accounts[from-1] -= amount;
    printf("Withdraw $%d from Account %d\n", amount, from);
}
void deposit(int to, int amount) {
    accountsPtr->accounts[to-1] += amount;
    printf("Deposit $%d to Account %d\n", amount, to);
}
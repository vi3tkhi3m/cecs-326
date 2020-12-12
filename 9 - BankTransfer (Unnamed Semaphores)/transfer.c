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
#include <semaphore.h> 	     // POSIX semaphore
#include <pthread.h>         // Threads

void withdraw(int from, int amount);
void deposit(int to, int amount);

struct ACCOUNTS *accountsPtr;
void 	*memptr;
int ret;

int main(int argc, char *argv[])
{
	if(argc<4) {
        printf("Please provide the correct amount of arguments!\n"); 
        printf("[shared_memory_name] [bank_from] [bank_to] [transfer_amount]\n"); 
    }  else {
        char* sharedMemorySegmentName = argv[0];
        int from = atoi(argv[1]);
        int to = atoi(argv[2]);
        int amount = atoi(argv[3]);

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

        // sem_wait decrements the value of the semaphore by one if its value is greater than 0. If the value is 0, the current thread will block until its greater than 0.
	// Parameter is the pointer to the semaphore.
        printf("Process %d - Locking semaphore #%d\n", getpid(), from); 
        sem_wait(&(accountsPtr->semaphores[from]));
        printf("Process %d - #%d Semaphore locked\n", getpid(), from); 
        //printf("Process %d - Activating sleep for 5 sec\n", getpid()); 
        //sleep(5);
        printf("Process %d - Locking semaphore #%d\n", getpid(), to); 
        sem_wait(&(accountsPtr->semaphores[to]));
        printf("Process %d - #%d Semaphore locked\n", getpid(), to); 

        withdraw(from, amount);
        deposit(to, amount);

        // sem_post unlocks the semaphore referenced by the sem pointer. It will increase its value from 0 to 1.
	// Parameter is the pointer to the semaphore.
        printf("Process %d - Releasing lock semaphore #%d\n", getpid(), to); 
        sem_post(&(accountsPtr->semaphores[to]));
        printf("Process %d - Semaphore #%d lock released\n", getpid(), to); 
        printf("Process %d - Releasing #%d lock semaphore\n", getpid(), from); 
        sem_post(&(accountsPtr->semaphores[from]));
        printf("Process %d - Semaphore #%d lock released\n", getpid(), from); 

        int amountOfAccounts = accountsPtr->nAccounts;
        for(int i = 0; i < amountOfAccounts; i++) {
            // sem_destroy will remove the unnamed semaphore from the system
            // The given parameter is the location of the semaphore that needs to be removed.
            sem_destroy(&(accountsPtr->semaphores[i]));
        }
        
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

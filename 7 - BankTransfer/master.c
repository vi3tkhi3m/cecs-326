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

void initShm(char* shmName);
void initSem(char* semName);
void initAccountBalances(int *bank1, int *bank2);
void delShm(char* shmName);
void delSem(char* semName);
void printBalances();

int shmid;
int shm_fd;
const int SIZE = 4096;
struct ACCOUNTS *accountsPtr;
sem_t *sem;

int main(int argc, char *argv[])
{
	if(argc<6) {
        printf("Please provide the correct amount of arguments!\n"); 
        printf("[bank1_amount] [bank2_amount] [shared_memory_name] [sem1_name] [sem2_name]\n"); 
    }  else {
        int bankAmount1 = atoi(argv[1]);
        int bankAmount2 = atoi(argv[2]);
        char* sharedMemorySegmentName = argv[3];
        char* semName1 = argv[4];
        char* semName2 = argv[5];

        initShm(sharedMemorySegmentName);

        initSem(semName1);
        initSem(semName2);

        initAccountBalances(&bankAmount1, &bankAmount2);
        printBalances();

        int amount = 50;
        char amountChar[10];
        sprintf(amountChar, "%d", amount);

        int from = 1;
        char fromChar[10];
        sprintf(fromChar, "%d", from);

        int to = 2;
        char toChar[10];
        sprintf(toChar, "%d", to);

        pid_t child_pid, wpid;

        // fork will create a new child process.
        if ((child_pid = fork()) == 0) {
            char *args[] = {sharedMemorySegmentName, semName1,semName2, fromChar, toChar, amountChar, NULL};

            // Execute the transfer program and give it the above arguments.
            // First parameter is the path to the transfer program (compiled).
            // Second parameter is the arguments that will be passed to the transfer program.
            execv("./transfer", args);

            // Exit the child process.
            exit(0);
        }

        // waiting for children to terminate (wait(NULL))
        while(wait(NULL) != -1);

        printBalances();

        delShm(sharedMemorySegmentName);

        delSem(semName1);
        delSem(semName2);
    }

    return 0;
}

void initShm(char* shmName) {
        // shm_open creates or opens a POSIX shared memory object
        // First parameter is the name of the shared memory
        // Second parameter is the flags you can provide. YOu can provide multiple flags like creating, make it read only, read-write etc.
        // Last parameter sets the permissions of the shared memory object
        shm_fd = shm_open(shmName, O_CREAT | O_RDWR, 0666);
        if (shm_fd == -1) {
            printf("prod: Shared memory failed: %s\n", strerror(errno));
            // We exit 1 here because we indicate that something failed so we dont exit normally. Exit 0 will exit the program normally.
            exit(1);
        }
        
        // Configure the size of shared memory segment
        // First parameter is the file descriptor
        // Second parameter is the desired size of the file in bytes
        ftruncate(shm_fd, SIZE);

        // mmap establishes a mapping between an address space of a process and a file associated with the file descriptor (shm_fd)
        // The first parameter specifies the starting address of the memory region to be mapped.
        // The second parameter specifies the length
        // The third parameter specifies the access permissions, in this case read or write
        // The fourth parameter specifies the attributes of the mapped region. MAP_SHARED will make modifications to the mapped region visible to other processes that also mapped the same region.
        // The fifth parameter specifies the file descriptor
        // The last parameter specifies the file byte offset at which the mapping starts.
        accountsPtr = mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (accountsPtr == MAP_FAILED) {
            printf("prod: Map failed: %s\n", strerror(errno));
            // We exit 1 here because we indicate that something failed so we dont exit normally. Exit 0 will exit the program normally.
            exit(1);
        }
}

void initSem(char* semName) {
    // sem_open opens a connection between a named semaphore and a process.
	// First parameter is the name of the semaphore. You can name this anything.
	// Second parameter are the flags. In this case O_CREAT will create a semaphore.
	// Third parameter is the file permission of the second parameter, in this case for the O_CREAT. 0600 = the owner can read/write the semaphore.
	// Last parameter is the initial value that will be set to the semaphore, in this case 1.
	sem = sem_open(semName, O_CREAT, 0666, 1);
    if(sem == SEM_FAILED) {
        perror("sem_open failed");
        exit(1);
    }
}

void initAccountBalances(int *bank1, int *bank2) {
    accountsPtr->nAccounts = 2;
    accountsPtr->accounts[0] = *bank1;
    accountsPtr->accounts[1] = *bank2;
}

void delShm(char* shmName) {
    // Removes the mapped shared memory segment from the address space of the process
    // First parameter is the starting address of the region.
    // Second parameter is the length of the address range.
    if (munmap(accountsPtr, SIZE) == -1) {
        printf("cons: Unmap failed: %s\n", strerror(errno));
        // We exit 1 here because we indicate that something failed so we dont exit normally. Exit 0 will exit the program normally.
        exit(1);
    }

    // Closes the shared memory segment as if it was a file
    // Parameter: the file descriptor
    if (close(shm_fd) == -1) {
        printf("cons: Close failed: %s\n", strerror(errno));
        // We exit 1 here because we indicate that something failed so we dont exit normally. Exit 0 will exit the program normally.
        exit(1);
    }
}

void delSem(char* semName) {
	// sem_close will close a named sempahore that was opened by sem_open. Parameter = the pointer to the semaphore.
    sem_close(sem);
    // sem_unlink removes the semaphore from the system by name. Parameter = the name of the semaphore.
    sem_unlink(semName);
}

void printBalances() {
    int nAccounts = accountsPtr->nAccounts;

    printf("Number of accounts: %d\n", nAccounts); 
    for(int i = 0; i < nAccounts; i++) {
        printf("Account #%d balance: %d\n", i + 1, accountsPtr->accounts[i]); 
    }
}
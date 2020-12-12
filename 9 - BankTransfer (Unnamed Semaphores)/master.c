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
void initSem();
void initAccountBalances(int accountsArray[], int nAccounts);
void delShm(char* shmName);
void delSem();
void printBalances();

int shmid;
int shm_fd;
const int SIZE = 4096;
struct ACCOUNTS *accountsPtr;

int main(int argc, char *argv[])
{
	if(argc<4) {
        printf("Please provide the correct amount of arguments!\n"); 
        printf("[bank1_amount] [bank2_amount] [shared_memory_name]\n"); 
    }  else {

        int bankAccounts[50];
        int amountOfAccounts = argc-2;

        for(int i = 0; i < amountOfAccounts; i++) {
            bankAccounts[i] = atoi(argv[i+1]);
        }

        char* sharedMemorySegmentName = argv[argc-1];

        initShm(sharedMemorySegmentName);
        initAccountBalances(bankAccounts, amountOfAccounts);
        initSem();

        printBalances();

        int amount = 50;
        char amountChar[10];
        sprintf(amountChar, "%d", amount);

        int amount2 = 25;
        char amountChar2[10];
        sprintf(amountChar2, "%d", amount2);

        int from = 1;
        char fromChar[10];
        sprintf(fromChar, "%d", from);

        int to = 2;
        char toChar[10];
        sprintf(toChar, "%d", to);

        pid_t child_pid, wpid;

        // fork will create a new child process.
        if ((child_pid = fork()) == 0) {
            char *args[] = {sharedMemorySegmentName, fromChar, toChar, amountChar, NULL};

            // Execute the transfer program and give it the above arguments.
            // First parameter is the path to the transfer program (compiled).
            // Second parameter is the arguments that will be passed to the transfer program.
            execv("./transfer", args);

            // Exit the child process.
            exit(0);
        }

        // second child
        if ((child_pid = fork()) == 0) {
            char *args[] = {sharedMemorySegmentName, toChar, fromChar, amountChar2, NULL};

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
        delSem();
        delShm(sharedMemorySegmentName);
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

void initSem() {
    // sem_init opens a connection between a unnamed semaphore and a process.
	// First parameter is location of the semaphore to be initialized.
	// Second parameter determines whether the semaphore can be shared with other processes or not. In this case, 1 allows sharing.
	// Last parameter is the initial value that will be set to the semaphore, in this case 1.
    int amountOfAccounts = accountsPtr->nAccounts;
	for(int i = 0; i < amountOfAccounts; i++) {
        sem_init(&(accountsPtr->semaphores[i]), 1, 1);
    }
}

void initAccountBalances(int accountsArray[], int nAccounts) {
    accountsPtr->nAccounts = nAccounts;
    for(int i = 0; i < nAccounts; i++) {
        accountsPtr->accounts[i] = accountsArray[i];
    }
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

void delSem() {
	int amountOfAccounts = accountsPtr->nAccounts;
	for(int i = 0; i < amountOfAccounts; i++) {
        // sem_destroy will remove the unnamed semaphore from the system
        // The given parameter is the location of the semaphore that needs to be removed.
        sem_destroy(&(accountsPtr->semaphores[i]));
    }
}

void printBalances() {
    int nAccounts = accountsPtr->nAccounts;

    printf("Number of accounts: %d\n", nAccounts); 
    for(int i = 0; i < nAccounts; i++) {
        printf("Account #%d balance: %d\n", i + 1, accountsPtr->accounts[i]); 
    }
}
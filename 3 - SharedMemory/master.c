#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>

struct myShm {
    int index;
    int response[10];
};

int main(int argc, char* argv[]) {

    if(argc < 3) {
        printf("Please provide the correct amount of arguments. [n child processes] [shared memory segment name].");
    } else {
        printf("Master begins execution\n");

        char* numberOfChildren = argv[1];
        char* sharedMemorySegmentName = argv[2];

        // Give the file a size in bytes
        const int SIZE = 4096;

        // Struct base address, from mmap()
        struct myShm *shm_base;

        // File descriptor, from shm_open()
        int shm_fd;

        // shm_open creates or opens a POSIX shared memory object
        // First parameter is the name of the shared memory
        // Second parameter is the flags you can provide. YOu can provide multiple flags like creating, make it read only, read-write etc.
        // Last parameter sets the permissions of the shared memory object
        shm_fd = shm_open(sharedMemorySegmentName, O_CREAT | O_RDWR, 0666);
        if (shm_fd == -1) {
            printf("prod: Shared memory failed: %s\n", strerror(errno));
            // We exit 1 here because we indicate that something failed so we dont exit normally. Exit 0 will exit the program normally.
            exit(1);
        }

        if (shm_base == MAP_FAILED) {
            printf("prod: Map failed: %s\n", strerror(errno));/* close and shm_unlink */
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
        shm_base = mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

        printf("Master created a shared memory segment named %s.\n", sharedMemorySegmentName);

        // Convert the numberOfChildren to an int. This will be used in the for loop.
        int nChildren = atoi(numberOfChildren);

        for(int i = 0; i < nChildren; i++) {
            // Fork creates a new process.
            pid_t cpid = fork();
            // 0 = fork successful
            if (cpid == 0) { 
                char number_str[10];
                // Convert an integer to a char so that it can be passed as an argument.
                sprintf(number_str, "%d", i);

                // Arguments that will be passed to the child program. 
                char *args[] = {number_str, sharedMemorySegmentName, NULL};

                // Execute the slave program and give it the above arguments.
                // First parameter is the path to the slave program (compiled).
                // Second parameter is the arguments that will be passed to the slave program.
                execv("./slave", args);

                // Exit the child process normally.
                exit(0);
            }
        }

        printf("Master created %s child processes to execute slave.\n", numberOfChildren);

        printf("Master waits for all child processes to terminate.\n");
        // waiting for both children to terminate (wait(NULL))
        while(wait(NULL) != -1);
        
        printf("Master received termination signals from all %s child processes.\n", numberOfChildren);

        printf("Content of shared memory segment filled by child processes:\n");
        printf("--- content of shared memory ---\n");

        for(int i = 0; i < nChildren; i++) {
            printf("%d\n", shm_base -> response[i]);
        }

        // Removes the mapped shared memory segment from the address space of the process
        // First parameter is the starting address of the region.
        // Second parameter is the length of the address range.
        if (munmap(shm_base, SIZE) == -1) {
            printf("cons: Unmap failed: %s\n", strerror(errno));
            // We exit 1 here because we indicate that something failed so we dont exit normally. Exit 0 will exit the program normally.
            exit(1);
        }

        // Closes the shared memory segment as if it was a file
        if (close(shm_fd) == -1) {
            printf("cons: Close failed: %s\n", strerror(errno));
            // We exit 1 here because we indicate that something failed so we dont exit normally. Exit 0 will exit the program normally.
            exit(1);
        }

        // Removes the shared memory segment from the file system
        // Parameter: the shared memory segment name
        if (shm_unlink(sharedMemorySegmentName) == -1) {
            printf("cons: Error removing %s: %s\n", sharedMemorySegmentName, strerror(errno));
            // We exit 1 here because we indicate that something failed so we dont exit normally. Exit 0 will exit the program normally.
            exit(1);
        }

        printf("Master removed shared memory segment, and is exiting.\n");
    }
    // Exit 0 will exit the program normally.
    exit(0);
}
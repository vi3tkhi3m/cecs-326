#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h> 
#include <fcntl.h> 
#include <sys/shm.h> 
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>

struct myShm {
    int index;
    int response[10];
};

int main(int argc, char* argv[]) {

    if(argc < 2) {
        printf("Please provide the correct amount of arguments. [child number] [shared memory segment name].");
    } else {
        printf("Slave begins execution\n");
        
        char* childNumber = argv[0];
        // Convert a char to an int. 
        int childNumberInt = atoi(childNumber);

        char* sharedMemorySegmentName = argv[1];

        // Give the file a size in bytes
        const int SIZE = 4096;

        printf("I am child numer %s, received shared memory name %s.\n", childNumber, sharedMemorySegmentName);

        // File descriptor, from shm_open()
        int shm_fd;
        // Struct base address, from mmap()
        struct myShm *shm_base;

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

        // Configure the size of shared memory segment
        // First parameter is the file descriptor
        // Second parameter is the desired size of the file in bytes
        ftruncate(shm_fd,SIZE);
        
        // mmap establishes a mapping between an address space of a process and a file associated with the file descriptor (shm_fd)
        // The first parameter specifies the starting address of the memory region to be mapped.
        // The second parameter specifies the length
        // The third parameter specifies the access permissions, in this case read or write
        // The fourth parameter specifies the attributes of the mapped region. MAP_SHARED will make modifications to the mapped region visible to other processes that also mapped the same region.
        // The fifth parameter specifies the file descriptor
        // The last parameter specifies the file byte offset at which the mapping starts.
        shm_base = mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        
        if (shm_base == MAP_FAILED) {
            printf("prod: Map failed: %s\n", strerror(errno));/* close and shm_unlink */
            // We exit 1 here because we indicate that something failed so we dont exit normally. Exit 0 will exit the program normally.
            exit(1);
        }
        
        // Write to the shared memory region.
        shm_base -> index = childNumberInt;
        shm_base -> response[childNumberInt] = childNumberInt;
        printf("I have written my child number to shared memory.\n");
        
        // Removes the mapped shared memory segment from the address space of the process
        // First parameter is the starting address of the region.
        // Second parameter is the length of the address range.
        if (munmap(shm_base, SIZE) == -1) {
            printf("prod: Unmap failed: %s\n", strerror(errno));
            // We exit 1 here because we indicate that something failed so we dont exit normally. Exit 0 will exit the program normally.
            exit(1);
        }  

        // Closes the shared memory segment as if it was a file
        // Parameter: the file descriptor
        if (close(shm_fd) == -1) {
            printf("prod: Close failed: %s\n", strerror(errno));
            // We exit 1 here because we indicate that something failed so we dont exit normally. Exit 0 will exit the program normally.
            exit(1);
        }

        printf("Slave closed access to shared memory and terminates\n");
    }

    // Exit 0 will exit the program normally.
    exit(0);
}
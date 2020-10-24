#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h> 
#include <stdlib.h>

using namespace std;

int main(int argc,char* argv[]) {

    // Holds the number of children.
    int amountOfChildren = 0;

    // Defines a pair.
    const int pair = 2;

    // Checks if the given arguments are an even number.
    if((argc - 1) % pair == 0 ) {
        // Sets the amountOfChilderen based on the given arguments.
        amountOfChildren = argc / pair;
        printf("I have %d children.", amountOfChildren);
        // Flush internal buffer. This will prevent the above printf from printing afterwards.
        fflush(stdout);

        pid_t child_pid, wpid;
        char childNumber[3];

        for(int i = 0; i < amountOfChildren; i++) {
            // Create a new child process.
            if ((child_pid = fork()) == 0) {
                // Convert an int to a char. This is needed in order to pass the number to the arguments below.
                sprintf(childNumber, "%d", i);

                // Arguments that will be passed to the child program. 
                char *args[] = {childNumber, argv[i*2+1],argv[i*2+2], NULL};

                // Execute the child program and give it the above arguments.
                execv("./child", args);

                // Exit the child process.
                exit(0);
            }
        }
        
        int status = 0;

        // Wait for all child processes.
        while ((wpid = wait(&status)) > 0);

        printf("\nAll child processes terminated. Parent exits.\n");
    } else {
        // Odd pair error handling
        printf("Please provide pair(s) in arguments!\n");
    }

    return 0;
}
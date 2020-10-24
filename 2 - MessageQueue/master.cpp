#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

using namespace std;

int main() {

    printf("Master, PID %d, begins execution", getppid());

    // This key will be used for creating a message.
    key_t key;
    // Create a key. First parameter points to this file (master.cpp), second parameter can be anything.
    if(key=ftok(".",'u') == -1) {
        perror("key");
        exit(1);
    }

    // Creating a message id.
    // IPC_CREATE = Creates a message queue if the key specified does not already have an associated ID
    // 0600 is equal to cmod0600, which gives the owner the permission to read and write
    int qid = msgget(key, IPC_CREAT|0600);

    printf("\nMaster acquired a message queue, id %d", qid);
    // Flush internal buffer. This will prevent the above printf from printing afterwards.
    fflush(stdout);

    // This will hold the message id but in a char. This will later be used as a parameter for the sender & receiver program.
    char qidNumber[50];
    // Convert the qid (int) to the qidNumber holder above (char).
    sprintf(qidNumber, "%d", qid);

    // Create the first child (sender)
    pid_t cpid = fork();
    // 0 = fork successful
    if (cpid == 0) { 
        printf("\nMaster created a child process with PID %d to execute sender", getpid());
        // Arguments that will be passed to the child program. 
        char *args[] = {qidNumber, NULL};
        // Execute the child program and give it the above arguments.
        execv("./sender", args);

        // Exit the child process.
        exit(0);
    }
    // Create the second child (receiver)
    cpid = fork();
    if (cpid == 0) {
        printf("\nMaster created a child process with PID %d to execute receiver", getpid());
        // Arguments that will be passed to the child program. 
        char *args[] = {qidNumber, NULL};

        // Execute the child program and give it the above arguments.
        execv("./receiver", args);

        // Exit the child process.
        exit(0);
    }

    printf("\nMaster wait for both child processes to terminate");
    // waiting for both children to terminate (wait(NULL))
    while(wait(NULL) != -1);

    // This will destroy the message que, by giving it the qid and the destroy command (IPC_RMID), the third parameter is the buffer command.
    msgctl(qid, IPC_RMID, NULL); 
    
    printf("\nMaster received termination signals from both child processes, removed message queue, and terminates\n");
    exit(0);
}

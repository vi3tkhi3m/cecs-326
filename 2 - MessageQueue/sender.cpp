#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h> 
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <sys/msg.h>

using namespace std;

struct my_msgbuf {
    long mtype;
    char message[50];
};

int main(int argc,char* argv[]) {

    if(argc > 0) {
        int qid;
        // Convert the commandline argument (the message id) from a char to an int.
        sscanf(argv[0], "%d", &qid);

        my_msgbuf msg;
        // Define the message type. This can be anything, as long as the sender and reiver both have the same.
        msg.mtype = 114;
        // Calculate the message size
        int size = sizeof(msg)-sizeof(long);
        // The user input will be stored here.
        string message;

        // Pause the program, to make sure the receiver is up and running. Also, makes the output less messy.
        sleep(2);

        printf("Sender, PID %d, begins execution", getpid());
        printf("\nSender received message queue id %d through commandline parameter", qid);

        printf("\nPlease input your message: \n");
        // Get the user input
        getline (cin, message);
        // Put the message in msgbuf
        strcpy(msg.message, message.c_str());

        // Send the message
        // First parameter is the message queue id
        // Second parameter is the pointer to the msgbuf
        // Third patameter is the size of the message
        // The last parameter is the MessageFlag. This is set to 0. Here you can specify the action to be taken if the message cannot be sent.
        msgsnd(qid,(my_msgbuf *)&msg, size, 0);

        printf("\nSender sent message to message queue");
        printf("\nSender terminates\n");
    }

    return 0;
}
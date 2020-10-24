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
        printf("Receiver, PID %d, begins execution", getpid());

        // This will hold the message queue id that this program will receive as an argument. 
        // This int will later be used for receiving the message.
        int qid;
        // Convert the char argument (the message queue id) to an int.
        sscanf(argv[0], "%d", &qid);

        my_msgbuf msg;
        // Calculate the message size.
        int size = sizeof(msg)-sizeof(long);

        printf("\nReceiver received message queue id %d through commandline parameter", qid);

        // Receive the message.
        // First parameter is the message queue id
        // Second parameter is the pointer to the msgbuf
        // Third patameter is the size of the message
        // Fourth parameter is the messagetype number
        // The last parameter is the MessageFlag. This is set to 0. Here you can specify the action to be taken if the message cannot be sent.
        msgrcv(qid, (struct my_msgbuf *)&msg, size, 114, 0);

        printf("\nnReceiver: retrieved the following message from message queue: \n");
        cout << msg.message << endl;

        printf("\nReceiver terminates\n");
    }
    return 0;
}
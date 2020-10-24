#include <stdio.h> 

using namespace std;

int main(int argc,char* argv[]) {

    // Checks if there are 3 arguments (child #, gender, and name)
    if(argc=3) {
        printf("\n Child # %s: I am a %s, and my name is %s.", argv[0], argv[1], argv[2]);
    }

    return 0;
}
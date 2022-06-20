//Ariel Mantel 313450249
#include <stdio.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <wait.h>


void clientSignalHandler(){
    char myPidStr[50];
    int pidNum = getpid();
    sprintf(myPidStr, "%d", pidNum);
    char clientFileName[150] = "to_client_";
    strcat(clientFileName, myPidStr);
    int fdClientFile = open(clientFileName, O_RDONLY);
    char buffer[1000];
    memset(buffer,0,sizeof buffer);
    read(fdClientFile, buffer, 1000);
    printf("%s", buffer);
    close(fdClientFile);
    remove(clientFileName);
    while (wait(NULL)!=-1);
    signal(SIGCHLD, SIG_IGN);
    exit(0);
}

int main(int argc, char* argv[]) {

    signal(SIGUSR1, clientSignalHandler);

    //check 4 args
    int elementsInArgv = argc - 1;
    if(elementsInArgv != 4){
        printf("ERROR_FROM_EX4\n");
        exit(-1);
    }

    char myPidStr[50];
    int pidNum = getpid();
    sprintf(myPidStr, "%d", pidNum);
    char stringToFile[100];

    strcat(stringToFile,myPidStr);
    strcat(stringToFile," ");
    strcat(stringToFile, argv[2]);
    strcat(stringToFile, " ");
    strcat(stringToFile, argv[3]);
    strcat(stringToFile, " ");
    strcat(stringToFile, argv[4]);
    strcat(stringToFile, " ");

    //open to_srv file
    int timesDidntOpenFile = 0;
    int i = 0;
    for (i = 0; i < 10; ++i) {
        int fdOpenNewFile = open("to_srv", O_RDWR|O_CREAT|O_EXCL, 0777);
        if(fdOpenNewFile == -1){
            srand(time(NULL));
            int sleepFor = (rand() % 5) +1;
            sleep(sleepFor);
            timesDidntOpenFile++;
        } else {
            write(fdOpenNewFile, stringToFile, strlen(stringToFile));
            close(fdOpenNewFile);
            i = 11;
        }
    }
    if(i == 10){
        printf("Can not create 'to_srv' because it already exists\n");
        return 0;
    }

    // send signal to server
    pid_t serverPid = atoi(argv[1]);
    kill(serverPid, SIGUSR1);
    while (wait(NULL)!=-1);
    sleep(30);
    printf("Client closed because no response was received from the server for 30 seconds\n");
    return 0;
}

//Ariel Mantel 313450249
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <wait.h>

void serverAlarmHandler(){
    while (wait(NULL)!=-1);
    signal(SIGCHLD, SIG_IGN);

    printf("The server was closed because no service request was received from the last 60 seconds\n");
    exit(0);
}

void serverSignalHandler() {
    pid_t forkId = fork();
    if (forkId < 0){
        printf("ERROR_FROM_EX4\n");
        exit(-1);
    }
    if (forkId == 0) {
        //read the file.
        int fdOpenOldFile = open("to_srv", O_RDONLY);
        char buffer[1000];
        //getting components.
        char *clientId; char *first; char *action; char *second;
        read(fdOpenOldFile, buffer, 1000);

        clientId = strtok(buffer, " ");
//        if (clientId == NULL) {
//            printf("ERROR_FROM_EX4\n");
//            exit(-1);
//        }
        first = strtok(NULL, " ");
//        if (first == NULL) {
//            printf("ERROR_FROM_EX4\n");
//            exit(-1);
//        }
        action = strtok(NULL, " ");
//        if (action == NULL) {
//            printf("ERROR_FROM_EX4\n");
//            exit(-1);
//        }
        second = strtok(NULL, " ");
//        if (second == NULL) {
//            printf("ERROR_FROM_EX4\n");
//            exit(-1);
//        }
        close(fdOpenOldFile);
        remove("to_srv");//TODO: cancel the //

        //turning everything to int
        int firstNum = atoi(first);
        int secondNum = atoi(second);
        int actionNum = atoi(action);
        int devZeroFlag = 0;

        int sum = 0;
        // the sum of the equation
        if (actionNum == 1) {
            sum = firstNum + secondNum;
        } else if (actionNum == 2) {
            sum = firstNum - secondNum;
        } else if (actionNum == 3) {
            sum = firstNum * secondNum;
        } else if (actionNum == 4 && secondNum == 0) {
            devZeroFlag = 1;
        } else { //has to be divide no 0
            sum = firstNum / secondNum;
        }

        //name of file.
        char nameOfFile[50] = "to_client_";
        strcat(nameOfFile, clientId);

        //create file
        int fdOpenNewFile = open(nameOfFile, O_RDWR | O_CREAT | O_EXCL | O_TRUNC, 0777);
        if (fdOpenNewFile == -1){
            printf("ERROR_FROM_EX4\n");
        }
        if (devZeroFlag) {
            write(fdOpenNewFile, "CANNOT_DIVIDE_BY_ZERO\n", strlen("CANNOT_DIVIDE_BY_ZERO\n"));
        } else {
            char sumToStr[1000];
            sprintf(sumToStr, "%d", sum);
            write(fdOpenNewFile, sumToStr, strlen(sumToStr));
        }
        close(fdOpenNewFile);

        pid_t clientIdNum = atoi(clientId);
        kill(clientIdNum, SIGUSR1);
    }
    if(forkId > 0){
        signal(SIGCHLD, SIG_IGN);
        alarm(60);
        signal(SIGUSR1, serverSignalHandler);
    }
}

int main() {
    signal(SIGALRM, serverAlarmHandler);
    signal(SIGUSR1, serverSignalHandler);
    alarm(60);
    remove("to_srv");
    while (1);
    return 0;
}

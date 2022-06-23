#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <wait.h>
#include <string.h>
#include <dirent.h>

int compareOutputs(char* trueResults, char* studentResults){
    int stat;
    pid_t pid;
    char *toRun[] = {"./comp.out", trueResults, studentResults, NULL};
    pid = fork();
    if (pid==0){
        stat = execvp(toRun[0], toRun);
        if (stat == -1){
            write(1, "error in exec", strlen("error in exec"));
        }
    } else if(pid < 0){
        write(1, "fork error", strlen("fork error"));
    }
    return WEXITSTATUS(stat);
}

void execCompiled(){
    int stat;
    pid_t pid;
    char* toRun[] = {"./a.out", NULL};
    pid = fork();
    if(pid == 0){
        stat = execvp(toRun[0], toRun);
        if (stat == -1){
            write(1,"didnt manage to exec compiled", strlen("didnt manage to exec compiled"));
        }
    } else if(pid < 0){
        write(1,"error in fork", strlen("error in fork"));
    }
    if(waitpid(pid, &stat, 0)<0){
        write(1,"didnt manage to wait", strlen("didnt manage to wait"));
    }
    remove("a.out");
}

int tryCompile(char* cFilePath){
    pid_t pid;
    int stat;
    char *toCompile[] = {"gcc", cFilePath, NULL};
    pid = fork();
    if (pid == 0){
        stat = execvp(toCompile[0], toCompile);
        if (stat == -1){
            write(1, "error, didn't execute right.", strlen("error, didn't execute right."));
        }
    } else if(pid < 0){
        write(1,"fork error", strlen("fork error"));
    }
    if(waitpid(pid, &stat, 0) < 0){
        write(1, "error in wait", strlen("error in wait"));
    }
    return WEXITSTATUS(stat);
}

int main(int args, char* argv[]) {
    //FdsArr: 0) conf. 1) results csv.
    // 2) error. 3) ass output.
    int FdsArr[5];
    int openFdsNum = 0;

    //saving old dups.
    int oldInDup = dup(0);
    int oldOutDup = dup(1);

    //open conf file
    FdsArr[0] = open(argv[1], O_RDONLY);
    openFdsNum++;
    if(FdsArr[0] == -1){
        write(1, "Error in: open\n", strlen("Error in: open\n"));
        // typical closer
        for (int i = 0; i < openFdsNum; i++){
            close(FdsArr[i]);
        }
        exit(-1);
    }

    FdsArr[1] = open("results.csv", O_CREAT|O_WRONLY|O_TRUNC, 0644);
    openFdsNum++;
    if(FdsArr[1] == -1) {
        write(1, "Error in: open\n", strlen("Error in: open\n"));
        // typical closer
        for (int i = 0; i < openFdsNum; i++) {
            close(FdsArr[i]);
        }
        exit(-1);
    }
    FdsArr[2] = open("error.txt", O_CREAT|O_WRONLY|O_TRUNC, 0644);
    openFdsNum++;
    if(FdsArr[2] == -1) {
        write(1, "Error in: open\n", strlen("Error in: open\n"));
        // typical closer
        for (int i = 0; i < openFdsNum; i++) {
            close(FdsArr[i]);
        }
        exit(-1);
    }


    //reading conf file
    char buffer[500];
    char rootPath[150], input[150], output[150];
    read(FdsArr[0], buffer, 500);
    int index = 0;
    for (int j = 0, k = 0; j < 500 && index != 3; j++) {
        if (buffer[j] == '\n') {
            if (index == 0) {
                rootPath[k] = '\0';
                k = 0;
                ++index;
                j++;
            }else if (index == 1) {
                input[k] = '\0';
                ++index;
                k = 0;
                j++;
            }else if (index == 2) {
                output[k] = '\0';
                ++index;
                k = 0;
                j++;
            }
        }
        if (index == 0) {
            rootPath[k] = buffer[j];
            k++;
        }
        if (index == 1) {
            input[k] = buffer[j];
            k++;
        }
        if (index == 2) {
            output[k] = buffer[j];
            k++;
        }
    }

    // check if can access input and output.
    int ifFile = access(input, F_OK);
    if (ifFile == -1){
        write(1, "Input file does not exist", strlen("Input file does not exist"));
    }
    ifFile = access(output, F_OK);
    if (ifFile == -1){
        write(1, "Output file does not exist", strlen("Output file does not exist"));
    }

    // start working on root folder
    int ifDir = access(rootPath, F_OK);
    if (ifDir == -1){
        write(1, "error opening root dir", strlen("error opening main dir"));
    }
    DIR *rootDir = opendir(rootPath);
    struct dirent* studentDir;
    while ((studentDir = readdir(rootDir))){
        int shouldCon = 1;
        char* studentName = studentDir->d_name;
        if ((strcmp(studentName, ".") == 0 || strcmp(studentName, "..") == 0 ||studentDir->d_type != DT_DIR)){
            continue;
        }

        //path to student
        char studentFolderPath[151] = {};
        strcpy(studentFolderPath, rootPath);
        strcat(studentFolderPath, "/");
        strcat(studentFolderPath, studentName);

        DIR *studentDirIter = opendir(studentFolderPath);
        struct dirent *studentFile;
        //iterate through student files
        while ((studentFile = readdir(studentDirIter))){
            char* studentFileName = studentFile->d_name;
            //check if c file.
            if(strlen(studentFileName) >= 3 && strcmp(studentFileName + strlen(studentFileName) - 2, ".c") == 0){
                char studentFilePath[151] = {};
                strcpy(studentFilePath,studentFolderPath);
                strcat(studentFilePath,"/");
                strcat(studentFilePath,studentFileName);
                int stat = tryCompile(studentFilePath);
                //if not compiled
                if (stat != 0){
                    strcat(studentName,",10,COMPILATION_ERROR\n");
                    write(FdsArr[1], studentName, strlen(studentName));
                    shouldCon = 0;
                    continue;
                }

                FdsArr[3] = open("ass_output.txt", O_CREAT|O_WRONLY|O_TRUNC, 0644);
                openFdsNum++;
                if(FdsArr[3] == -1) {
                    write(1, "Error in: open\n", strlen("Error in: open\n"));
                    // typical closer
                    for (int i = 0; i < openFdsNum; i++) {
                        close(FdsArr[i]);
                    }
                    exit(-1);
                }
                int fdInput = open(input, O_WRONLY,0644);

                dup2(FdsArr[3],1);
                dup2(fdInput, 0);
                execCompiled();
                dup2(oldInDup, 0);
                dup2(oldOutDup, 1);

                int compareResults = compareOutputs(output, "ass_output.txt");
                if (compareResults == 1) {
                    strcat(studentName,",100,EXCELLENT\n");
                    write(FdsArr[1], studentName, strlen(studentName));
                    shouldCon = 0;
                    continue;
                } else if (compareResults == 3){
                    strcat(studentName,",75,SIMILAR\n");
                    write(FdsArr[1], studentName, strlen(studentName));
                    shouldCon = 0;
                    continue;
                } else{
                    strcat(studentName,",50,WRONG\n");
                    write(FdsArr[1], studentName, strlen(studentName));
                    shouldCon = 0;
                    continue;
                }
            }
        }
        if (shouldCon) {
            strcat(studentName, ",0,NO_C_FILE\n");
            write(FdsArr[1], studentName, strlen(studentName));
        }
    }

    return 0;
}


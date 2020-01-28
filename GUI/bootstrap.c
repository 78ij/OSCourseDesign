#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include "pv.h"

int keyshm,keyshm2,buf1f,buf1e,buf2f,buf2e;


int main(int argc,char **argv){


    keyshm = shmget(KEYSHM,101,IPC_CREAT|0666);
    keyshm2 = shmget(KEYSHM2,101,IPC_CREAT|0666);
    buf1e = semget(BUF1E,1,IPC_CREAT|0666);
    buf1f = semget(BUF1F,1,IPC_CREAT|0666);
    buf2e = semget(BUF2E,1,IPC_CREAT|0666);
    buf2f = semget(BUF2F,1,IPC_CREAT|0666);

    semctl(buf1e,0,SETVAL,1);
    semctl(buf1f,0,SETVAL,0);
    semctl(buf2e,0,SETVAL,1);
    semctl(buf2f,0,SETVAL,0);

    char *source = argv[1];
    char *dest = argv[2];

    if(argc != 3){
        printf("The count of arguments is not correct.\n");
        return 0;
    }
    else{
        if(access(source,F_OK | R_OK) != 0){
            printf("Source File not Exist or it can't be read.\n");
            return 0;
        }
        struct stat s_buf;
        stat(source,&s_buf);
        if(S_ISDIR(s_buf.st_mode)){
            printf("Source is a directory.\n");
            return 0;
        }
        if(access(dest,F_OK) == 0){
            printf("Dest file exists.\n");
            return 0;
        }

        // launch the child processes
        if(fork() == 0){
            execl("./first",source,dest,NULL);
        }
        else{
            if(fork() == 0){
                execl("./second",source,dest,NULL);
            }
            else{
                if(fork() == 0){
                    execl("./third",source,dest,NULL);
                }
                else{
                    wait();
                    wait();
                    wait();
                    shmctl(keyshm,IPC_RMID,NULL);
                    shmctl(keyshm2,IPC_RMID,NULL);
                    semctl(buf1e,1,IPC_RMID);
                    semctl(buf2e,1,IPC_RMID);
                    semctl(buf1f,1,IPC_RMID);
                    semctl(buf2f,1,IPC_RMID);
                    return 0;
                }
            }
        }
    }
    return 0;
}
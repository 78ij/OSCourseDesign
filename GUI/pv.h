#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>


#define KEYSHM 12450
#define KEYSHM2  12449
#define BUF1F  12451
#define BUF1E  12452
#define BUF2F  12453
#define BUF2E  12454


union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *arry;
};

void P(int semid){
    struct sembuf tempbuf;
    tempbuf.sem_num = 0;
    tempbuf.sem_op = -1;
    tempbuf.sem_flg = SEM_UNDO;
    semop(semid,&tempbuf,1);
}

void V(int semid){
    struct sembuf tempbuf;
    tempbuf.sem_num = 0;
    tempbuf.sem_op = 1;
    tempbuf.sem_flg = SEM_UNDO;
    semop(semid,&tempbuf,1);
}
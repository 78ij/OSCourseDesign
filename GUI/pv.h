#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>


const int keyshm = 12450;
const int keyshm2 = 12449;
const int buf1f = 12451;
const int buf1e = 12452;
const int buf2f = 12453;
const int buf2e = 12454;


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
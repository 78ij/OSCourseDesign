#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>



void printhelp(){
    printf("A simple program that simulates the behavior of cp\n");
    printf("Usage: copy [OPTIONS]... SOURCE DEST\n");
    printf("Options:\n");
    printf("-h: Print This help message.\n");
    printf("-f: Force to Write on the dest file.\n");
}
int main(int argc, char **argv){
    int op = 0;
    int force = 0;
    while((op = getopt(argc,argv,"hf"))!=-1){
        switch(op){
            case 'h':
                printhelp();
                return 0;
            case 'f':
                force = 1;
                break;
            case '?':
                printf("Execute copy -h to print help message.\n");
                return 0;
        }
    }
    if(argc - optind  != 2){
        printf("Command line arguments format not correct.\n");
        printf("Execute copy -h to print help message.\n");
        return 0;
    }
    char *source = argv[optind];
    char *dest = argv[optind+1];
    if(access(source,F_OK | R_OK) != 0){
        printf("Source File not Exist or it can't be read.\n");
        return 0;
    }
    if(access(dest,F_OK) == 0 && !force){
        printf("Dest file exists.\n");
        return 0;
    }
    return 0;
    
}
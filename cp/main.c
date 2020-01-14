#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>



void printhelp(){
    printf("A simple program that simulates the behavior of cp\n");
    printf("Usage: copy [OPTIONS]... SOURCE DEST\n");
    printf("Options:\n");
    printf("-h: Print This help message.\n");
    printf("-f: Force to write on existing dest file.\n");
}

int main(int argc, char **argv){
    int op = 0;
    int force = 0;
    // Get command line arguments
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

    // Is there source and dest?
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

    struct stat s_buf;
    stat(source,&s_buf);
    if(S_ISDIR(s_buf.st_mode)){
        printf("Source is a directory.\n");
        return 0;
    }

    if(access(dest,F_OK) == 0 && !force){
        printf("Dest file exists.\n");
        return 0;
    }
    else if(access(dest,F_OK) == 0){
        stat(dest,&s_buf);
        if(S_ISDIR(s_buf.st_mode)){
        printf("Dest is a directory.\n");
        return 0;
        }
    }

    // Copy Starts
    char buffer[100]; // buffer of 100 bytes
    int sourcefd = open(source,O_RDONLY);
    if(sourcefd == -1){
        printf("Open source file failed.\n");
        return 0;
    }
    int destfd = open(dest,O_RDWR|O_CREAT,0666);
    if(destfd == -1){
        printf("Open dest file failed.\n");
        return 0;
    }

    size_t size,writesize;
    while((size = read(sourcefd,buffer,100))!= 0){
        writesize = write(destfd,buffer,size);
        if(writesize != size) printf("Unknown Write Error.\n");
    }

    return 0;
    
}
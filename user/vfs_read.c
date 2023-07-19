#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define MSG_SIZE 4096

int main(){
    printf("Attempting to read the unique file\n");
    int fd = open("mount/the_file",O_RDONLY,O_CREAT);
    char buf[MSG_SIZE];
    if(fd < 0){
        printf("Error while opening the_file\n");
        printf("Errno: %d",errno);
        return 1;
    }
    while(1){
        int ret = read(fd,buf,MSG_SIZE-1);
        if(ret==-1){
            printf("Error during read\n");
            printf("Errno: %d",errno);
            return 1;
        }
        if(ret==0){
            printf("EOF reached\n");
            return 0;
        }
        printf("I have read %d bytes and message is %s\n",ret,buf);
    }
}
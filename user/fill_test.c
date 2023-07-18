#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int n_put_data;
int n_get_data;
int n_invalidate_data;

int put_data(char* source, size_t size){
    return syscall(n_put_data,source,size);
}

int get_data(int offset, char * destination, size_t size){
    return syscall(n_get_data,offset,destination,size);
}

int invalidate_data(int offset){
    return syscall(n_invalidate_data,offset);
}

int main(int argc, char **argv){
    if(argc!=4){
        printf("Usage: %s <#syscall put_data> <#syscall get_data> <#syscall invalidate_data>\n",argv[0]);
        exit(0);
    }
    n_put_data = atoi(argv[1]);
    n_get_data = atoi(argv[2]);
    n_invalidate_data = atoi(argv[3]);
    char msg[]={"Testing message"};
    int count=0;
    int res;
    char content[128];
    printf("Putting message %s until i get ENOMEM error\n",msg);
    while(1){
        res = put_data(msg,strlen(msg));
        if(res<0){
            if(errno==ENOMEM){
                printf("Device saturated with %d putting operations\n", count);
            }
            exit(0);
        }
        count++;
    }
}
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

int main(int argc, char** argv){
    if(argc!=4){
        printf("Usage: %s <#syscall put_data> <#syscall get_data> <#syscall invalidate_data>\n",argv[0]);
        exit(0);
    }
    int res = 0;
    n_put_data = atoi(argv[1]);
    n_get_data = atoi(argv[2]);
    n_invalidate_data = atoi(argv[3]);
    printf("Testing read operation followed by write operation\n");
    printf("Writing something in the file using put_data\n");
    char msg[1024];
    char resp[1024];
    strcpy(msg,"Hello World!\n");
    res = put_data(msg,strlen(msg)+1);
    if(res== -ENOMEM){
        printf("There is no space in this device\n");
        exit(0);
    }
    if(res<0){
        printf("An error has occured during write operation\n");
        exit(0);
    }
    printf("block number is: %d\n",res);
    printf("Writing operation completed! Written: %s\n",msg);
    printf("Sleeping...\n");
    sleep(60);
    printf("Reading with get_data\n");
    res = get_data(res,resp,strlen(msg)+1);
    if(res == -ENODATA){
        printf("No such data\n");
        exit(0);
    }
    printf("Read %d bytes : %s\n",res,resp);
}
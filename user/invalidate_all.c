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
    int n_block=0;
    printf("How many consecutive blocks do you want to invalidate?:");
    scanf("%d",&n_block);
    for(int i=0;i<n_block;i++){
        int res = invalidate_data(i);
        if(res<0){
            switch(errno){
                case -ENODATA:
                    printf("No data in %d block\n",i);
                    break;
                case -ENODEV:
                    printf("No device mounted\n");
                    return 0;
                default:
                    printf("Unexpected error:%d\n",errno);
                    return 0;
            }
        }
        else{
            printf("Block %d correctly invalidated\n");
        }
    }
}
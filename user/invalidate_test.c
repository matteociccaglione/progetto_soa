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
    char msg[128];
    int block_num;
    char content[128];

    printf("Insert a message to put\n");
    scanf("%s",msg);

    printf("Trying to put msg: %s\n",msg);
    res = put_data(msg,strlen(msg)+1);
    if (res<0){
        printf("An error has occurred during put operation\n");
        switch(res){
            case ENOMEM:
                printf("No space in device\n");
                break;
            case ENODEV:
                printf("Device not mounted\n");
                break;
            default:
                printf("Unhandled error\n");
        }
        exit(res);
    }
    printf("Operation completed, block written is: %d\n",res);
    printf("Reading from the device...\n");
    block_num=res;
    res = get_data(block_num,content,strlen(msg)+1);
    if(res<0){
        printf("An error has occurred during get operation\n");
        switch(errno){
            case ENODEV:
                printf("Device not mounted\n");
                break;
            case ENODATA:
                printf("No data available for block number: %d\n",block_num);
                break;
            default:
                printf("Unhandled error\n");
        }
        exit(res);
    }
    printf("Operation completed, message taken from device is: %s\n",content);
    printf("put a key to invalidate data\n");
    getchar();
    printf("Invalidating block number %d... \n",block_num);
    res = invalidate_data(block_num);
    if(res<0){
        printf("An error has occurred during invalidate operation\n");
        switch(errno){
            case ENODEV:
                printf("Device not mounted\n");
                break;
            case ENODATA:
                printf("No data available for block number: %d\n",block_num);
                break;
            default:
                printf("Unhandled error\n");
        }
        exit(res);
    }
    printf("Invalidate completed\n");
    printf("Reading block again, attempting to get -ENODATA error\n");
    res = get_data(block_num,content,strlen(msg));
    printf("the result is : %d\n", res);
    if(errno==ENODATA){
        printf("Test passed!\n");
    }
    else{
        printf("Test failed\n");
        exit(-2);
    }

    printf("Trying to re-invalidate the same block, attempting to get ENODATA error\n");
    res = invalidate_data(block_num);
    if (res == -1 && errno==ENODATA){
        printf("Test passed\n");
    }
    else{
        printf("Test failed, block was still valid or errno is different from ENODATA\n");
        exit(-2);
    }
}
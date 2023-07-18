#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>

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
    char msg[] = {"This message will not be inserted into the device\n"};
    res = put_data(msg,strlen(msg));
    if (res == -1 && errno==ENODEV){
        printf("Put data testing passed (no device mounted)");
    }
    else{
        printf("Put data test failed...\n");
        if (res==-1)
            printf("\tError code: %d\n",errno);
    }
    res = get_data(0,msg,1);
    if (res == -1 && errno==ENODEV){
        printf("Get data testing passed (no device mounted)");
    }
    else{
        printf("Get data test failed...\n");
        if (res==-1)
            printf("\tError code: %d\n",errno);
    }
    res = invalidate_data(0);
    if (res == -1 && errno==ENODEV){
        printf("Invalidate data testing passed (no device mounted)");
    }
    else{
        printf("Invalidate data test failed...\n");
        if (res==-1)
            printf("\tError code: %d\n",errno);
    }
    
}
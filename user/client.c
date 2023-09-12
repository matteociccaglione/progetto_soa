#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>

int n_put_data;
int n_get_data;
int n_invalidate_data;
#define MSG_SIZE 4096

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
    int choice;
    int res;
    int block_num;
    char msg[128];
    while(1){
        printf("Please select an option from the menu:\n\t 1) Put data;\n\t2) Get data;\n\t3) Invalidate data\n\t4) Read all\n\t5) exit\n");
        scanf("%d",&choice);
        switch(choice){
            case 1:
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
                else{
                    printf("Done\n");
                }
                break;
            case 2:
                printf("Insert a block number:");
                scanf("%d",&block_num);
                printf("Trying to read: %d\n",block_num);
                res = get_data(block_num,msg,128);
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
                else{
                    printf("Content is: %s\n",msg);
                }
                break;
            case 3:
                printf("Insert a block number:");
                scanf("%d",&block_num);
                printf("Trying to invalidate: %d\n",block_num);
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
                break;
            case 4:
                printf("Trying to read the file content\n");
                int fd = open("../mount/the_file",O_RDONLY,O_CREAT);
                char buf[MSG_SIZE];
                if(fd < 0){
                    printf("Error while opening the_file\n");
                    printf("Errno: %d",errno);
                    return 1;
                }
                while(1){
                    int ret = read(fd,buf,MSG_SIZE);
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
            default:
                return 0;
        }
    }
}
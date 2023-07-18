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

void * thread_f(void* the_arg){
    int the_thread = *(int*)the_arg;
    printf("Thread %d\n", the_thread);
    char msg[128];
    sprintf(msg,"%d",the_thread);
    int res = put_data(msg,strlen(msg));
    printf("Thread %d inserts in block %d\n", the_thread, res);
}


int main(int argc, char **argv){
    if(argc!=4){
        printf("Usage: %s <#syscall put_data> <#syscall get_data> <#syscall invalidate_data>\n",argv[0]);
        exit(0);
    }
    n_put_data = atoi(argv[1]);
    n_get_data = atoi(argv[2]);
    n_invalidate_data = atoi(argv[3]);
    int n_threads;
    int count=0;
    int res,i;
    int *t_num;
    pthread_attr_t attr;
    char content[128];
    printf("How much threads u want? :");
    scanf("%d",&n_threads);
    pthread_t *threads = malloc(sizeof(pthread_t) * n_threads);
    t_num= malloc(sizeof(int) * n_threads);
    for (i=0; i < n_threads; i++){
        t_num[i] = i;
        printf("Creating thread with num %d\n", t_num[i]);
        pthread_create(threads+i,NULL,thread_f,t_num+i);
    }
    void *result;
    for(i=0; i < n_threads; i++){
        pthread_join(threads[i],&result);
    }
}
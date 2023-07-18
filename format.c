#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "include/format.h"

/**
 * @brief This format.c will format the device writing the following
 * information onto the disk:
 *  - BLOCK 0, superblock;
 *  - BLOCK 1, inode of the unique file (the inode for root is volatile)
 *  - BLOCK 2, ..., N, inodes and datablocks for th messages.
*/

int main(int argc, char **argv){
    int fd, nbytes;
    ssize_t ret;
    struct dmsgs_sb_info sb_info;
    struct dmsgs_inode file_inode;
    struct dmsgs_dir_record dir_record;
    uint num_data_blocks;
    char *block_padding;
    struct stat st;
    off_t size;
    uint32_t i;

    if (argc != 2){
        printf("Usage: %s <device>\n", argv[0]);
        return -1;
    }

    //Open the device in read write mode

    fd = open(argv[1], O_RDWR);
    if (fd < 0){
        perror("Error opening the device\n");
        return -1;
    }

    fstat(fd, &st);
    size = st.st_size;

    // pack the superblock
    sb_info.version = 1;
    sb_info.magic = MAGIC;

    // write superblock on the device
    ret = write(fd, (char *)&sb_info, sizeof(sb_info));

    if(ret != DEFAULT_BLOCK_SIZE){
        printf("Written bytes [%d] are not equal to the default block size.\n", (int)ret);
        close(fd);
        return ret;
    }
    printf("Superblock written successfully\n");

    file_inode.mode = S_IFREG;
    file_inode.inode_no = SINGLEFILE_INODE_NUMBER;
    file_inode.file_size = size - (2 * DEFAULT_BLOCK_SIZE);

    //write the single file inode

    ret = write(fd, (char *)&file_inode, sizeof(file_inode));
    if (ret != sizeof(file_inode)){
        printf("The file inode was not properly written.\n");
        close(fd);
        return -1;
    }

    printf("File inode successfully written\n");

    //Write block padding to saturate the inode block

    nbytes = DEFAULT_BLOCK_SIZE - ret;
    block_padding = malloc(nbytes);
    ret = write(fd, block_padding, nbytes);
    if (ret != nbytes){
        printf("Padding for file inode block was not properly written.\n");
        close(fd);
        return -1;
    }
    printf("Padding for the block containing the file inode succesfully written.\n");


   
    num_data_blocks = file_inode.file_size / DEFAULT_BLOCK_SIZE;
   
   //nbytes is the maximum size of space for data

    nbytes = DEFAULT_BLOCK_SIZE - sizeof(uint32_t) - sizeof(uint32_t) - sizeof(long) -sizeof(unsigned char);

    //Fill remaining blocks. By default block 1 contains msg1 string and block 5 contains msg2 string.
    
    block_padding = calloc(nbytes, 1);
    char *msg1 = "Block number 1\n";
    char *msg2 = "Block number 5\n";
    for (i=0; i<num_data_blocks; i++){
        //Write the block index
        ret = write(fd, &i, sizeof(uint32_t));
        if (ret != sizeof(i)){
            printf("Error writing device block's metadata (index)\n");
            close(fd);
            return -1;
        }

        if (i == 1 || i==5){
            //Fill block 1 and 5 with the proper content and the proper metadata
            char *s;
            switch(i){
                case 1:
                    s = msg1; break;
                case 5:
                    s = msg2; break;
            }

            uint32_t valid_bytes = strlen(s) + 1;       
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            signed long long nsec = ts.tv_sec*1000 + ts.tv_nsec;
            unsigned char is_valid = BLK_VALID;

            
            ret = write(fd, &valid_bytes, sizeof(uint32_t));
            if (ret != sizeof(uint32_t)){
                printf("Error writing device block's metadata (valid bytes)\n");
                close(fd);
                return -1;
            }

            ret = write(fd, &nsec, sizeof(long));
            if (ret != sizeof(long)){
                printf("Error writing device block's metadata (nsec)\n");
                close(fd);
                return -1;
            }

            ret = write(fd, &is_valid, sizeof(unsigned char));
            if (ret != sizeof(unsigned char)){
                printf("Error writing device block's metadata (is_valid)\n");
                close(fd);
                return -1;
            }

            
            ret = write(fd, s, valid_bytes);
            if (ret != valid_bytes){
                printf("Error writing device block's metadata (valid bytes)\n");
                close(fd);
                return -1;
            }

            ret = write(fd, block_padding, nbytes - valid_bytes);
            if(ret != nbytes - valid_bytes){
                printf("Error initializing device block content: ret is %ld, should have been %d\n", ret, nbytes - valid_bytes);
                close(fd);
                return -1;
            }
            continue;
        }
        //For all other block just writes padding.
        ret = write(fd, block_padding, sizeof(uint32_t) + sizeof(long) + sizeof(unsigned char));
        if (ret != sizeof(uint32_t) + sizeof(long) + sizeof(unsigned char)){
            printf("Error writing device block's metadata (fields set to 0)\n");
            close(fd);
            return -1;
        }

        
        ret = write(fd, block_padding, nbytes);
        if(ret != nbytes){
            printf("Error initializing device block content\n");
            close(fd);
            return -1;
        }
    }

    printf("File system formatted correctly\n");
    close(fd);
    return 0;
}
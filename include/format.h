#pragma once
#ifndef __FORMAT_H
#define __FORMAT_H
#include <linux/fs.h>
#include <linux/types.h>


#define MOD_NAME "DEVICEMSGSYSTEM"
#define FS_NAME "dmsgsystem_fs"

#define MAGIC 0x42424242
#define DEFAULT_BLOCK_SIZE 4096

#ifndef NBLOCKS
    #define NBLOCKS 100
#endif

#define SB_BLOCK_NUMBER 0
#define FILENAME_MAX_LEN 255
#define UNIQUE_FILENAME_LEN 8
#define ROOT_INODE_NUMBER 2

#define BLK_INVALID (0)
#define BLK_VALID (BLK_INVALID + 1)
#define BLK_FREE BLK_VALID
#define BLK_NOT_FREE BLK_INVALID

#define UNIQUE_FILE_NAME "the_file"
#define INODES_BLOCK_NUMBER 1
#define SINGLEFILE_INODE_NUMBER 1

extern int last_block_written;




extern unsigned char device_mounted;
//inode definition
struct dmsgs_inode {
    mode_t mode;                                    //not exploited
    uint64_t inode_no;
    uint64_t data_block_number;                     //not exploited

    union {
        uint64_t file_size;
        uint64_t dir_children_count;
    };
};

//dir definition (how the dir data block is organized)
struct dmsgs_dir_record {
    char filename[FILENAME_MAX_LEN];
    uint64_t inode_no;
};

//super-block definition
struct dmsgs_sb_info {
    uint64_t version;
    uint64_t magic;

    //padding to fit into a single block
    char padding[ DEFAULT_BLOCK_SIZE - (2 * sizeof(uint64_t))];
};


// file.c
extern const struct inode_operations dmsgs_inode_ops;
extern const struct file_operations dmsgs_file_operations;

// dir.c
extern const struct file_operations dmsgs_dir_operations;
#endif
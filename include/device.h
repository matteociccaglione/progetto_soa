#pragma once
#ifndef __DMSGS_DEVICE_H__
#define __DMSGS_DEVICE_H__

#include <linux/ktime.h>
#include <linux/types.h>

// device's block metadata
typedef struct __attribute__((packed)) dmsgs_block{
    uint32_t ndx;                   // 32 bit
    uint32_t valid_bytes;           // 32 bit
    ktime_t nsec;                   // 64 bit   
    unsigned char is_valid;         // 8 bit    
}dmsgs_block;

#define METADATA_SIZE sizeof(dmsgs_block)

extern size_t md_array_size;
#endif
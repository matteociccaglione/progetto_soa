#pragma once
#ifndef __FLS_H_
#define __FLS_H_
#include <linux/fs.h>
#include <linux/blkdev.h>
#include <linux/buffer_head.h>
#include "format.h"
extern struct super_block* the_sb;
extern unsigned char block_status[NBLOCKS];
extern unsigned long n_blocks_handled;
extern struct srcu_struct ss;
#endif


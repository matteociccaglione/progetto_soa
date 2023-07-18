#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/timekeeping.h>
#include <linux/time.h>
#include <linux/buffer_head.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/string.h>

#include "include/filesystem.h"

/*
 * this function handles only the index of one out of 3 possible outcomes:
 * - 0: the directory "."
 * - 1: the directory ".."
 * - 2: the single file of the filesytem
 */
static int dmsgs_iterate(struct file *file, struct dir_context *ctx){
	if (ctx->pos >= (2 + 1)){
		return 0;
	}

	if (ctx->pos == 0){
		printk("[%s]: looking for directory .\n",MOD_NAME);
		if (!dir_emit(ctx, ".", 1, ROOT_INODE_NUMBER, DT_UNKNOWN)){
			return 0;
		}else{
			ctx->pos++;
		}
	}else if (ctx->pos == 1){
		printk("[%s]: looking for directory ..\n",MOD_NAME);
		if(!dir_emit(ctx,"..", 2, ROOT_INODE_NUMBER, DT_UNKNOWN)){
			return 0;
		}
		else{
			ctx->pos++;
		}
	}else{
		printk("[%s]: looking for the_file\n",MOD_NAME);
		if(!dir_emit(ctx, UNIQUE_FILE_NAME, UNIQUE_FILENAME_LEN, ROOT_INODE_NUMBER, DT_UNKNOWN)){
			return 0;
		}
		else{
			ctx->pos++;
		}
	}
	return 0;
}

const struct file_operations dmsgs_dir_operations = {
	.owner = THIS_MODULE,
	.iterate = dmsgs_iterate,
};

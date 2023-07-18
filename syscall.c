#define EXPORT_SYMTAB
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/kprobes.h>
#include <linux/mutex.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <asm/page.h>
#include <asm/cacheflush.h>
#include <asm/apic.h>
#include <asm/io.h>
#include <linux/syscalls.h>
#include "include/rcu.h"
#include "include/filesystem.h"
#include "include/syscall.h"

spinlock_t write_lock;
int last_block_written=-1;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0)
__SYSCALL_DEFINEx(3, _get_data, int, offset, char*, destination, size_t, size){
#else
asmlinkage long sys_get_data(int offset, char* destination, size_t size){
#endif
    if(!device_mounted){
        return -ENODEV;
    }
    int n_block = offset+2;
    int len = size;
    int ret;
    int byte_to_read;
    struct buffer_head *bh = NULL;
    rcu_elem *rcu_el;
    if(offset > NBLOCKS){
        return -ENOMEM;
    }
    printk("[%s]: looking for block with ndx = %d\n",MOD_NAME,offset);
    rcu_read_lock();
    list_for_each_entry_rcu(rcu_el,&valid_blk_list,node){
        if(rcu_el->ndx == offset){
            //Block found
            bh = sb_bread(the_sb, n_block);
            if(!bh){
		        rcu_read_unlock();
		        return -EIO;
	        }
            //read at most the size of a block
            if(len>rcu_el->valid_bytes){
                len=rcu_el->valid_bytes;
            }
            dmsgs_block block;
            //Read block metadata
            memcpy(&block,bh->b_data,sizeof(dmsgs_block));
            //check that block is not empty
            if(block.valid_bytes==0){
                return 0;
            }
            byte_to_read = size;
            if(byte_to_read>block.valid_bytes)
                byte_to_read=block.valid_bytes;
            //copy block data to user
            ret = copy_to_user(destination,bh->b_data+METADATA_SIZE,byte_to_read);
            brelse(bh);
            rcu_read_unlock();
            return block.valid_bytes-ret;
        }
    }
    //Block was not found in the rcu_list, so it is not valid
    rcu_read_unlock();
    printk("[%s]: attempting to return -ENODATA\n", MOD_NAME);
    return -ENODATA;
}


#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0)
__SYSCALL_DEFINEx(1, _invalidate_data, int, offset){
#else
asmlinkage long sys_invalidate_data(int offset){
#endif
    if(!device_mounted){
        return -ENODEV;
    }
    rcu_elem *el;
    struct buffer_head *bh;
    spin_lock(&write_lock);
    list_for_each_entry(el, &valid_blk_list, node){
        if (el->ndx == offset){
            // this is the element to be removed
            list_del_rcu(&el->node);
            bh = sb_bread(the_sb, offset);
            *(bh->b_data+METADATA_SIZE-sizeof(unsigned char))=0x0;
            mark_buffer_dirty(bh);
            brelse(bh);
            spin_unlock(&write_lock);
            #if FLUSH
                printk("[%s]: Sync dirty buffer\n", MOD_NAME);
                sync_dirty_buffer(bh);
            #else
                printk("[%s]: Mark buffer dirty\n", MOD_NAME);
            #endif
            // wait for the grace period and then free the removed element
            synchronize_rcu();
            printk("[%s]: End of grace period\n", MOD_NAME);
            kfree(el);
            return 0;
        }
    }
    spin_unlock(&write_lock);
    return -ENODATA;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0)
__SYSCALL_DEFINEx(2, _put_data, char*, source, size_t, size){
#else
asmlinkage long sys_put_data(char* source, size_t size){
#endif
    rcu_elem *rcu_el;
    dmsgs_block block;
    struct buffer_head *bh;
    int ndx=-1;
    int i;
    //constructs a byte array which is used to find the first invalid block
    unsigned char *indexes = kzalloc(sizeof(unsigned char)*NBLOCKS, GFP_KERNEL);
    if(!device_mounted){
        return -ENODEV;
    }
    printk("[%s]: Taking write lock\n", MOD_NAME);
    spin_lock(&write_lock);
    rcu_read_lock();
    if(!indexes){
        spin_unlock(&write_lock);
        return -ENOMEM;
    }
    list_for_each_entry_rcu(rcu_el, &valid_blk_list, node){
        indexes[rcu_el->ndx]=0x1;
    }
    rcu_read_unlock();
    i=last_block_written+1;
    if(i==md_array_size)
        i=0;
    //Iterate on array in circular behavior starting from last_block_written
    while(1){
        if(indexes[i] == 0x0){
            ndx=i;
            break;
        }
        i++;
        if (i==md_array_size){
            i = 0;
        }
        if(i==last_block_written){
            break;
        }
    }
    //There is no space
    if(ndx==-1){
        spin_unlock(&write_lock);
        kfree(indexes);
        return -ENOMEM;
    }
    //Write block data and metadata
    bh = sb_bread(the_sb, ndx + 2);
    block.is_valid=BLK_VALID;
    block.ndx = ndx;
    block.valid_bytes=size;
    block.nsec = ktime_get_real();
    memcpy(bh->b_data,&block,sizeof(dmsgs_block));
    copy_from_user(bh->b_data+sizeof(dmsgs_block),source,size);
    add_valid_block_lock_safe(ndx, size, block.nsec);
    mark_buffer_dirty(bh);
    brelse(bh);
    last_block_written = ndx;
    spin_unlock(&write_lock);
    #if FLUSH
        printk("[%s]: Sync dirty buffer\n", MOD_NAME);
        sync_dirty_buffer(bh);
    #else
        printk("[%s]: Mark buffer dirty\n", MOD_NAME);
    #endif
    kfree(indexes);
    return ndx;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,17,0)
long sys_get_data = (unsigned long) __x64_sys_get_data;
long sys_invalidate_data = (unsigned long) __x64_sys_invalidate_data;
long sys_put_data = (unsigned long) __x64_sys_put_data;    
#else
#endif

void register_syscall(unsigned long new_syscall[], int n_entries){
    new_syscall[1]=sys_get_data;
    new_syscall[0]=sys_put_data;
    new_syscall[2]=sys_invalidate_data;
}


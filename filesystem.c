#include <linux/fs.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/blkdev.h>
#include <linux/mutex.h>
#include <linux/buffer_head.h>
#include <linux/version.h>
#include <linux/timekeeping.h>
#include <linux/string.h>
#include "include/filesystem.h"
#include "include/device.h"
#include "include/rcu.h"
#include "include/syscall.h"


//This enum is used to handle possible concurrency during mount operation
typedef enum MountingState{
    FREE = 0,   //Device is free
    MOUNTING = 1,   //Someone is mounting the device
    MOUNTED = 2,    //Device is mounted
    UMOUNTING = 3   //Someone is unmounting the device
}MountingState;

unsigned char nomorercu=0x0;

MountingState the_state; //current state of the device

spinlock_t mount_lock;  //lock for mounting state r/w access

unsigned char block_status[NBLOCKS];
unsigned long n_blocks_handled=0;
struct srcu_struct ss;

unsigned long the_syscall_table = 0x0;

module_param(the_syscall_table,ulong,0660);

int put_data_number = 0;

module_param(put_data_number,int,0660);

int get_data_number = 0;

module_param(get_data_number,int,0660);

int invalidate_data_number = 0;

module_param(invalidate_data_number,int,0660);

unsigned long the_ni_syscall;

#define N_SYSCALL 3

int syscall_entries[N_SYSCALL];


unsigned long new_syscall[N_SYSCALL];

unsigned char device_mounted = 0;
struct super_block * the_sb;
size_t md_array_size;

unsigned long cr0;

static inline void
write_cr0_forced(unsigned long val)
{
    unsigned long __force_order;

    asm volatile(
        "mov %0, %%cr0"
        : "+r"(val), "+m"(__force_order));
}

static inline void
protect_memory(void)
{
    write_cr0_forced(cr0);
}

static inline void
unprotect_memory(void)
{
    write_cr0_forced(cr0 & ~X86_CR0_WP);
}

#define AUDIT if(1)
#define LEVEL3_AUDIT if(0)

#define MAX_ACQUIRES 4


//stuff for sys cal table hacking
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,3,0)
    #include <asm/switch_to.h>
#else
    #include <asm/system.h>
#endif
#ifndef X86_CR0_WP
#define X86_CR0_WP 0x00010000
#endif




int get_entries(int * entry_ids, int num_acquires, unsigned long sys_call_table, unsigned long *sys_ni_sys_call) {

        unsigned long * p;
        unsigned long addr;
        int i,j,z,k; //stuff to discover memory contents
        int ret = 0;
	int restore[MAX_ACQUIRES] = {[0 ... (MAX_ACQUIRES-1)] -1};


      
	if(num_acquires < 1){
       		
		 return -1;
	}
	if(num_acquires > MAX_ACQUIRES){
       		
		 return -1;
	}

	p = (unsigned long*)sys_call_table;

        j = -1;
        for (i=0; i<256; i++){
		for(z=i+1; z<256; z++){
			if(p[i] == p[z]){
				AUDIT{
                        	
				}
				addr = p[i];
                        	if(j < (num_acquires-1)){
				       	restore[++j] = i;
					ret++;
                        
				}
                        	if(j < (num_acquires-1)){
                        		restore[++j] = z;
					ret++;
                        	
				}
				for(k=z+1;k<256 && j < (num_acquires-1); k++){
					if(p[i] == p[k]){
                        		
                        			restore[++j] = k;
						ret++;
					}
				}
				if(ret == num_acquires){
					goto found_available_entries;
				}
				return -1;	
			}
                }
        }

        

	return -1;

found_available_entries:
        
	memcpy((char*)entry_ids,(char*)restore,ret*sizeof(int));
	*sys_ni_sys_call = addr;

	return ret;

}

/**
 * @brief  Returns 0 if they are equal, > 0 if lts is after rts, < 0 otherwise.
 */
static inline long timespec64_comp(struct timespec64 lts, struct timespec64 rts){
    return timespec64_sub(lts, rts).tv_nsec;
}

static inline long ktime_comp(ktime_t lkt, ktime_t rkt){
    return lkt - rkt;
}

static struct super_operations fs_super_ops = {
};

static struct dentry_operations fs_dentry_ops = {
};


int fs_fill_super(struct super_block *sb, void *data, int silent){

    struct inode *root_inode;
    struct dmsgs_inode *the_file_inode;
    struct buffer_head *bh;
    struct dmsgs_sb_info *sb_info;
    uint64_t magic;
    struct timespec64 curr_time;
    int i, nr_valid_blocks, ret;
    dmsgs_block metadata;
    
    the_sb = sb;

    // magic number that identifies the FS
    sb->s_magic = MAGIC;

    // read the superblock at index SB_BLOCK_NUMBER
    bh = sb_bread(sb, SB_BLOCK_NUMBER);
    if(!sb){
        return -EIO;
    }

    // read the magic number from the device
    sb_info = (struct dmsgs_sb_info *) bh->b_data;
    magic = sb_info->magic;
    brelse(bh);

    // check if the magic number corresponds to the expected one
    if (magic != sb->s_magic){
        return -EBADF;
    }

    sb->s_fs_info = NULL;
    sb->s_op = &fs_super_ops;

    root_inode = iget_locked(sb, 0);
    if (!root_inode){
        return -ENOMEM;
    }

    //Init root inode with specific operation
    root_inode->i_ino = ROOT_INODE_NUMBER;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
    inode_init_owner(sb->s_user_ns, root_inode, NULL, S_IFDIR);
#else
    inode_init_owner(root_inode, NULL, S_IFDIR);
#endif

    root_inode->i_sb = sb;

    root_inode->i_op = &dmsgs_inode_ops;
    root_inode->i_fop = &dmsgs_dir_operations;


    root_inode->i_mode = S_IFDIR | S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IXUSR | S_IXGRP | S_IXOTH;


    ktime_get_real_ts64(&curr_time);
    root_inode->i_atime = root_inode->i_mtime = root_inode->i_ctime = curr_time;

    //Set private data to NULL
    root_inode->i_private = NULL;


    sb->s_root = d_make_root(root_inode);
    if (!sb->s_root){
        return -ENOMEM;
    }
    

    sb->s_root->d_op = &fs_dentry_ops;


    unlock_new_inode(root_inode);



    bh = sb_bread(sb, INODES_BLOCK_NUMBER);
    if(!bh){
        return -EIO;
    }
    

    the_file_inode = (struct dmsgs_inode *) bh->b_data;
    brelse(bh);
    if (the_file_inode->file_size > NBLOCKS * DEFAULT_BLOCK_SIZE){
        return -E2BIG;
    }

    
    md_array_size = the_file_inode->file_size / DEFAULT_BLOCK_SIZE;
    n_blocks_handled = md_array_size;
    pr_info("%s: the device has %lu blocks\n", MOD_NAME, md_array_size);
    nr_valid_blocks = 0;
    rcu_init();
    init_srcu_struct(&ss);
    for (i = 0; i < md_array_size; i++){
        bh = sb_bread(sb, i + 2);
        if (!bh){
            return -EIO;
        }
        //read block metadata
        memcpy(&metadata,bh->b_data,sizeof(dmsgs_block));
        brelse(bh);
        block_status[i]=0x0;
        // if it's a valid block, also insert it into the initial RCU list
        if (metadata.is_valid == BLK_VALID){
            pr_info("%s: Block of index %u is valid - it has timestamp %lld\n", MOD_NAME, metadata.ndx, metadata.nsec);
            nr_valid_blocks++;
            //Add block in order with respect to nsec field
            ret = add_blk_in_order_lock_safe(metadata.ndx, metadata.valid_bytes, metadata.nsec);
            if (ret < 0){
                return ret;
            }
            block_status[i]=0x1;
        }
    }
    // signal that the device (with the file system) has been mounted
    device_mounted = 1;

    return 0;
}



static void fs_kill_sb(struct super_block *sb){
    spin_lock(&mount_lock);
    //If the current state is not mounted return -ENODEV error
    if(the_state!=MOUNTED){
        spin_unlock(&mount_lock);
        return;
    }
    else{
        //Change state to UMOUNTING
        the_state = UMOUNTING;
        spin_unlock(&mount_lock);
        kill_block_super(sb);
        the_sb=NULL;
        device_mounted = 0;
        printk(KERN_INFO "%s: file system unmount successful\n", MOD_NAME);
        //File system unmount succesful, move to FREE state
        spin_lock(&mount_lock);
        the_state = FREE;
        spin_unlock(&mount_lock);
    }
    return;
}

//Mount operation will be executed only if the device is in state FREE

struct dentry *fs_mount(struct file_system_type *fs_type, int flags, const char *dev_name, void *data){
    struct dentry *ret;
    spin_lock(&mount_lock);
    if (the_state == FREE){
        the_state = MOUNTING;
        spin_unlock(&mount_lock);
        ret = mount_bdev(fs_type, flags, dev_name, data, fs_fill_super);
        if (unlikely(IS_ERR(ret))){
            printk("%s: error mounting the file system\n", MOD_NAME);
            spin_lock(&mount_lock);
            the_state = FREE;
            spin_unlock(&mount_lock);
        }
        else{
            printk("%s: file system correctly mounted on from device %s\n", MOD_NAME, dev_name);
            spin_lock(&mount_lock);
            the_state = MOUNTED;
            spin_unlock(&mount_lock);
        }
    }
    else{
        //Device is busy. It is in MOUNTING, UNMOUNTING or MOUNTED state
        spin_unlock(&mount_lock);
        return NULL;
    }
    return ret;
}


static struct file_system_type dmsgs_fs_type = {
    .owner      = THIS_MODULE,
    .name       = FS_NAME,
    .mount      = fs_mount,
    .kill_sb    = fs_kill_sb
};

static int __init dmsgs_init(void){
    int ret;
    int i;
    printk("[%s]: the_syscall_table addr: %lu\n",MOD_NAME,the_syscall_table);
    //Use get_entries function to take 3 entries
    ret = get_entries(syscall_entries,3,the_syscall_table,&the_ni_syscall);
    printk("[%s]: free entries retrieved\n",MOD_NAME);
    if(ret!=3){
        printk("Unable to install 3 syscalls\n");
        return -1;
    }
    //This function save syscall addresses in new_syscall
    register_syscall(new_syscall,3);
    put_data_number = syscall_entries[0];
    get_data_number = syscall_entries[1];
    invalidate_data_number = syscall_entries[2];
    printk("[%s]: syscall registered succesfully\n",MOD_NAME);
    cr0 = read_cr0();
    unprotect_memory();
    printk("[%s]: inserting syscall inside kernel table\n", MOD_NAME);
    for(i=0; i < N_SYSCALL; i++){
        printk("[%s]: %d° entry : %d\n",MOD_NAME,i+1,syscall_entries[i]);
         ((unsigned long *)the_syscall_table)[syscall_entries[i]] = new_syscall[i];
    }
    printk("[%s]: operation ok!\n",MOD_NAME);
    protect_memory();
    printk("[%s]: registering filesystem\n",MOD_NAME);
    ret = register_filesystem(&dmsgs_fs_type);
    if (likely(ret == 0))
        printk("%s: successfully registered %s\n", MOD_NAME, dmsgs_fs_type.name);
    else
        printk("%s: failed to register %s - error %d\n", MOD_NAME, dmsgs_fs_type.name, ret);

    return ret;
}

static void __exit dmsgs_exit(void){
    int ret;
    int i;
    cr0 = read_cr0();
    unprotect_memory();
    for(i=0;i < N_SYSCALL; i++){
        ((unsigned long *)the_syscall_table)[syscall_entries[i]] = the_ni_syscall;
    }
    protect_memory();
    ret = unregister_filesystem(&dmsgs_fs_type);

    if (likely(ret == 0))
        printk("%s: sucessfully unregistered %s driver\n",MOD_NAME, dmsgs_fs_type.name);
    else
        printk("%s: failed to unregister %s driver - error %d", MOD_NAME, dmsgs_fs_type.name, ret);
    
    nomorercu=0x1;
    synchronize_rcu();
    synchronize_srcu(&ss);
    cleanup_srcu_struct(&ss);
}


module_init(dmsgs_init);
module_exit(dmsgs_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Matteo Ciccaglione <matteociccaglione@gmail.com>");
MODULE_DESCRIPTION("DMSGS: Device for messaging system");

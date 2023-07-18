#include "include/rcu.h"
#include <linux/slab.h>


LIST_HEAD(valid_blk_list);
spinlock_t rcu_write_lock;



int add_valid_block(uint32_t ndx, uint32_t valid_bytes, ktime_t nsec){
    rcu_elem *el;
    el = kzalloc(sizeof(rcu_elem), GFP_KERNEL);
    if (!el)
        return -ENOMEM;


    el->valid_bytes = valid_bytes;
    el->nsec = nsec;

    spin_lock(&rcu_write_lock);
    el->ndx = ndx;
    list_add_tail_rcu(&el->node, &valid_blk_list);
    spin_unlock(&rcu_write_lock);
    return 0;    
}

int add_valid_block_lock_safe(uint32_t ndx, uint32_t valid_bytes, ktime_t nsec){
    rcu_elem *el;
    el = kzalloc(sizeof(rcu_elem), GFP_KERNEL);
    if (!el)
        return -ENOMEM;

    el->ndx = ndx;
    el->valid_bytes = valid_bytes;
    el->nsec = nsec;

    
    list_add_tail_rcu(&el->node, &valid_blk_list);
   
    return 0;    
}

int remove_valid_block(uint32_t ndx){
    rcu_elem *el;

   
    spin_lock(&rcu_write_lock);
    list_for_each_entry(el, &valid_blk_list, node){
        if (el->ndx == ndx){
            
            list_del_rcu(&el->node);

           
            spin_unlock(&rcu_write_lock);

           
            synchronize_rcu();
            kfree(el);
            return 0;
        }
    }

    spin_unlock(&rcu_write_lock);
    return -ENODATA;
}

int remove_valid_block_lock_safe(uint32_t ndx){
    rcu_elem *el;
    list_for_each_entry(el, &valid_blk_list, node){
        if (el->ndx == ndx){
           
            list_del_rcu(&el->node);
            
            synchronize_rcu();
            kfree(el);
            return 0;
        }
    }
    return -ENODATA;
}

inline void rcu_init(void){
    spin_lock_init(&rcu_write_lock);
    INIT_LIST_HEAD_RCU(&valid_blk_list);
}

int add_blk_in_order_lock_safe(uint32_t ndx, uint32_t valid_bytes, ktime_t nsec){
    rcu_elem *el;
    rcu_elem * new = kzalloc(sizeof(rcu_elem), GFP_KERNEL);
    if(!new){
        return -ENOMEM;
    }
    new->ndx = ndx;
    new->valid_bytes = valid_bytes;
    new->nsec = nsec;
    rcu_elem *prev=NULL;
    list_for_each_entry(el, &valid_blk_list, node){
        if(el->nsec > nsec){
            //Update rcu
            if(prev==NULL){
                list_add_rcu(&new->node,&valid_blk_list);
            }
            else
                list_add_rcu(&new->node,&prev->node);
            return 0;
        }
        prev = el;
    }
    list_add_tail_rcu(&new->node,&valid_blk_list);
    return 0;
}

#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif


static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xf5c99cbc, "inode_init_owner" },
	{ 0x30d49723, "iget_locked" },
	{ 0xa0378bea, "try_module_get" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0x82c87431, "param_ops_ulong" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0x8d522714, "__rcu_read_lock" },
	{ 0xc9352d52, "unregister_filesystem" },
	{ 0xe140de5f, "mark_buffer_dirty" },
	{ 0x4a42aca9, "d_make_root" },
	{ 0x37a0cba, "kfree" },
	{ 0x4b366f21, "iput" },
	{ 0xa1bce3f0, "synchronize_srcu" },
	{ 0x63cc217e, "register_filesystem" },
	{ 0xba8fbd64, "_raw_spin_lock" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x65487097, "__x86_indirect_thunk_rax" },
	{ 0xda31cf92, "kill_block_super" },
	{ 0xa3488088, "unlock_new_inode" },
	{ 0x92997ed8, "_printk" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0x62d33f31, "module_put" },
	{ 0x735450a5, "__brelse" },
	{ 0x68f31cbd, "__list_add_valid" },
	{ 0x8675270c, "init_srcu_struct" },
	{ 0x6091797f, "synchronize_rcu" },
	{ 0xbbbb18b0, "__srcu_read_lock" },
	{ 0x2469810f, "__rcu_read_unlock" },
	{ 0x6a6bcd33, "set_nlink" },
	{ 0x9ec6ca96, "ktime_get_real_ts64" },
	{ 0x885dd2d3, "cleanup_srcu_struct" },
	{ 0xe1537255, "__list_del_entry_valid" },
	{ 0x78eb5e1d, "__bread_gfp" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0xe2d5255a, "strcmp" },
	{ 0xf8fea1c8, "pv_ops" },
	{ 0xb02d6f45, "d_add" },
	{ 0x7b0ab578, "mount_bdev" },
	{ 0xd9b85ef6, "lockref_get" },
	{ 0x9f2486aa, "__srcu_read_unlock" },
	{ 0xeaa1fdc0, "kmalloc_trace" },
	{ 0x7e662233, "param_ops_int" },
	{ 0xb5b54b34, "_raw_spin_unlock" },
	{ 0xc4f0da12, "ktime_get_with_offset" },
	{ 0x4ca346e5, "kmalloc_caches" },
	{ 0xb83992f2, "module_layout" },
};

MODULE_INFO(depends, "");


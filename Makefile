obj-m += the_dms.o
the_dms-objs += filesystem.o file_ops.o dir_ops.o rcu.o syscall.o


DEVICE_TYPE := "dmsgsystem_fs"
BLOCK_SIZE := 4096
NBLOCKS := 1000
NR_BLOCKS_FORMAT := 102
FLUSH ?= 0
KCPPFLAGS := '-DNBLOCKS=$(NBLOCKS) -DFLUSH=$(FLUSH)'
SYSTAB_ADDR = $(shell cat /sys/module/the_usctm/parameters/sys_call_table_address)

all:
	gcc	format.c -lrt -o format -DNBLOCKS=$(NBLOCKS)
	KCPPFLAGS=$(KCPPFLAGS) make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm format
	rmdir mount

create-fs:
	dd bs=$(BLOCK_SIZE) count=$(NR_BLOCKS_FORMAT) if=/dev/zero of=image
	./format image
	mkdir mount

mount-fs:
	mount -o loop -t $(DEVICE_TYPE) image ./mount/

umount-fs:
	umount ./mount

insmod:
	insmod the_dms.ko the_syscall_table=$(SYSTAB_ADDR)

rmmod:
	rmmod the_dms

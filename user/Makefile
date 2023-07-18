put_data = $(shell cat /sys/module/the_dms/parameters/put_data_number)
get_data = $(shell cat /sys/module/the_dms/parameters/get_data_number)
invalidate_data = $(shell cat /sys/module/the_dms/parameters/invalidate_data_number)

compile-all:
	echo "put_data_num: "
	echo $(put_data)
	echo "get_data_num: "
	echo ${get_data}
	echo "invalidate_data_num: "
	echo ${invalidate_data}
	mkdir executables
	gcc concurrency_test.c -o executables/concurrency_test -lpthread
	gcc fill_test.c -o executables/fill_test
	gcc invalidate_test.c -o executables/invalidate_test
	gcc nodev_mounted_test.c -o executables/nodev_mounted_test
	gcc vfs_read.c -o executables/vfs_read
clear:
	rm -r executables

run-all:
	echo "running concurrency_test\n"
	executables/concurrency_test ${put_data} ${get_data} ${invalidate_data}
	echo "running invalidate_test\n"
	executables/invalidate_test ${put_data} ${get_data} ${invalidate_data}
	echo "running fill_test\n"
	executables/fill_test ${put_data} ${get_data} ${invalidate_data}

run-nodev:
	echo "running nodev_mounted_test\n"
	executables/nodev_mounted_test ${put_data} ${get_data} ${invalidate_data}

run-concurrency_test:
	echo "running concurrency_test\n"
	executables/concurrency_test ${put_data} ${get_data} ${invalidate_data}

run-invalidate_test:
	echo "running invalidate_test\n"
	executables/invalidate_test ${put_data} ${get_data} ${invalidate_data}

run-fill_test:
	echo "running fill_test\n"
	executables/fill_test ${put_data} ${get_data} ${invalidate_data}
run-vfs_read:
	echo "running vfs_read\n"
	executables/vfs_read

	

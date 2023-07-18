# ADVANCED OPERATIVE SYSTEM PROJECT

### Student: Matteo Ciccaglione 0315944

To compile kernel level module and user-level software for device formatting use the provided Makefile, especially:
>1. Use the all directive to compile. You can specify compile time choice like this:
>>1. To provide a value for maximum number of blocks that can be handled, specify a value for the variable NBLOCKS;
>>2. To provide a value for device number of blocks, specify a value for the variable NR_FORMAT_BLOCKS;
>>3.  To decide whether to use the synchronous flush function, supply the value 1 for the FLUSH variable.
>2. Use the create-fs directive to format device file and create all required directories.
>3. Use the insmod directive to execute insmod command. Don't forget to execute usctm module first;
>4. Use mount-fs directive to execute mount command;
>5. Use umount-fs directive to execute umout command;
>6. Use rmmmod directive to execute rmmod command.

To use provided user-level tests, use the Makefile in user directory. Must be used as super user, since it requires permission to access dmsgsystem_fs module parameters that contain the syscall numbers.
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>

extern struct file * file_open(char * location){
	return filp_open(location, O_RDWR|O_SYNC , 0644);
}

extern int file_close(struct file * oFile){
	return filp_close(oFile, NULL);
}

extern int file_write(struct file * oFile,char* buffer, int count){
	loff_t pos = 0;

	int ret= vfs_write(oFile, buffer, count, pos);
//	fput(oFile);
	return ret;
}

/*extern int file_read(char*){
		vfs_read(file, data, block_size, &pos);
}*/

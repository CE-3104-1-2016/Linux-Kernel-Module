#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <asm/uaccess.h>

/**
 * @brief Method to open the file in a location
 * @param location a char with the location of the file
 */
extern struct file * file_open(char * location){
	return filp_open(location, O_RDWR , 0644);
}

/**
 * @brief Method to open the file in a location
 * @param oFile a struct with the file to read the buffer
 */
extern int file_close(struct file * oFile){
	return filp_close(oFile, NULL);
}

/**
 * @brief Method to open the file in a location
 * @param oFile a struct with the file to read the buffer
 * @param buffer a pointer to char with the data will be store
 * @param count the int that represent the size of the buffer
 */
extern int file_write(struct file * oFile,char* buffer, int count){
	loff_t pos = 0;
	int ret= vfs_write(oFile, buffer, count, pos);
//	fput(oFile);
	return ret;
}

/**
 * @brief Method to open the file in a location
 * @param oFile a struct with the file to read the buffer
 * @param buffer a pointer to char where the data will be store
 * @param count the int that represent the size of the buffer
extern int file_read(struct file * oFile,char* buffer, int count){
		return vfs_read(file, data, block_size, &pos);
}*/

/*
 *  hello-1.c - The simplest kernel module.
 */
#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include "fileHandler.c"
int init_module(void)
{
	printk(KERN_INFO "Hello world 1.\n");
  struct file * Afile = file_open("/home/malcolm/Documentos/Linux-Kernel-Module/Program/Malcolm.txt");
		printk(KERN_INFO "OPEN/WRITING. %d\n", Afile);
  file_write(Afile, "HOla archivo\n", 13);
		printk(KERN_INFO "writed/closing.\n");
  file_close(Afile);
	/*
	 * A non 0 return means init_module failed; module can't be loaded.
	 */
	return 0;
}

void cleanup_module(void)
{
	printk(KERN_INFO "Goodbye world 1.\n");
}

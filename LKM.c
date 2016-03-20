
/**
 * @file LKM.c
 * @authors Arturo Chinchilla, Malcolm Davis
 * @date 20 March 2016
 * @version 0.1
 * @abrief As part of the course Languages, Compilers and Interpreters, will be 
 * created a LKM for Linux 3.x which will be written in pure C and interact with 
 * an Arduino device 
 */

#include <linux/init.h>             // Macros used to mark up functions e.g., __init __exit
#include <linux/module.h>           // Core header for loading LKMs into the kernel
#include <linux/kernel.h>           // Contains types, macros, functions for the kernel

MODULE_LICENSE("GPL");              ///< The license type -- this affects runtime behavior
MODULE_AUTHOR("Arturo Chinchilla");  ///< The author -- visible when you use modinfo
MODULE_AUTHOR("Malcolm Davis"); 
MODULE_DESCRIPTION("LKM for Linux 3.x which will be written in pure C and interact with an Arduino device.");  ///< The description -- see modinfo
MODULE_VERSION("0.1");  


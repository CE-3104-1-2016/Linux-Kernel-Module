/**
 * @file   ArduinoLKM.c
 * @author Malcolm Davis
 * @author Arturo Chinchilla
 * @date   March 23 2015
 * @brief  A kernel module for controlling a arduino led project
 * has 2 modes, flash or burst, (suports interruptions)
 * and can be modified on linux user space with echo.
 * @see https://github.com/LKMInvestigation/Linux-Kernel-Module
*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include "fileHandler.c"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Malcolm Davis");
MODULE_AUTHOR("Arturo Chinchilla");
MODULE_DESCRIPTION("LKM for Linux 3.x which will be written in pure C and interact with an Arduino device.");
MODULE_VERSION("0.5");

static int buttonStats = 0;
static int burstRep = 1;
static bool ledStatus = 0;
static bool ledMode = 0;
static char arduinoPort[8] = "ttyACM0";
static const char arduinoLocation[13] = "/dev/ttyACM0";
static struct file *arduinoFile;


/**
 * @brief Function that returns the number that represents the times the button have been pressed.
 * @param kernelObject object that repressent the devices of the sysfs File System.
 * @param attribute the attribute of the object
 * @param buffer the buffer that will have the total number.
 * @return return the number of characters without null.
 */
static ssize_t buttonStats_show(struct kobject *kernelObject, struct kobj_attribute *attribute, char *buffer){
   return sprintf(buffer, "%d\n", buttonStats);
}

/**
 * @brief Function to store the button press number.
 * @param kernelObject object that repressent the devices of the sysfs File System.
 * @param attribute the attribute of the object
 * @param buffer the buffer that will have the total number.
 * @param count the number characters of the buffer.
 * @return return the number of buffer characters used.
 */
static ssize_t buttonStats_store(struct kobject *kernelObject, struct kobj_attribute *attribute,
 const char *buffer, size_t count){
   sscanf(buffer, "%du", &buttonStats);
   return count;
}

/**
 * @brief Shows the status of the led (on or off).
 * @param kernelObject represents a kernel object device that appears in the sysfs filesystem
 * @param attribute the attribute of the object.
 * @param buffer the buffer that will have the button state.
*/
static ssize_t ledStatus_show(struct kobject *kernelObject, struct kobj_attribute *attribute, char *buffer){
   int tmp = (int)ledStatus;
   return sprintf(buffer, "%d\n", tmp);
}

/**
 * @brief Shows the mode that the led will function on (ON-OFF or BURST)-(0 or 1).
 * @param kernelObject represents a kernel object device that appears in the sysfs filesystem
 * @param attribute the attribute of the object.
 * @param buffer the buffer that will have the button mode.
*/
 static ssize_t ledMode_show(struct kobject *kernelObject, struct kobj_attribute *attribute, char *buffer){
   return sprintf(buffer, "%d\n", ledMode);
}

/**
 * @brief Function to store the led mode (ON-OFF or BURST)-(0 or 1).
 * @param kernelObject object that repressent the devices of the sysfs File System.
 * @param attribute the attribute of the object
 * @param buffer the buffer that will have Led Mode(0 or 1).
 * @param count the number characters of the buffer.
 * @return return the number of buffer characters used.
 */
 static ssize_t ledMode_store(struct kobject *kernelObject, struct kobj_attribute *attribute,
  char *buffer, size_t count){
    sscanf(buffer, "%du", &ledMode);
    printk("ARduinoLKM: Buffer Count: %d\n",count);
    printk("ARduinoLKM: Buffer: %s\n", *buffer);
    int ret = file_write(arduinoFile, buffer, count);
    if(ret != count){
      printk( "ARduinoLKM: Did not write, error: %d\n", ERR_PTR(ret));
    }else{
      printk( "ARduinoLKM: Write, code: %d\n", ERR_PTR(ret));
    }
    return count;
}

/**
 * @brief Shows the repetition number of the burst function.
 * @param kernelObject represents a kernel object device that appears in the sysfs filesystem
 * @param attribute the attribute of the object.
 * @param buffer the buffer that will have repetition number.
*/
 static ssize_t burstRep_show(struct kobject *kernelObject, struct kobj_attribute *attribute, char *buffer){
   return sprintf(buffer, "%d\n", burstRep);
}

/**
 * @brief Function to store the repetition number of the burst function.
 * @param kernelObject object that repressent the devices of the sysfs File System.
 * @param attribute the attribute of the object
 * @param buffer the buffer that will have the repetition number of the burst function(0...n).
 * @param count the number characters of the buffer.
 * @return return the number of buffer characters used.
 */
 static ssize_t burstRep_store(struct kobject *kernelObject, struct kobj_attribute *attribute,
 const char *buffer, size_t count){
   sscanf(buffer, "%du", &burstRep);
   return count;
}

/**
 * Helper macros to handle the attributes of the LKM.
 * pressCount for the button press times, to show or to store a different value.
 * ledMode for the mode that the arduino will use the leds.
 * burstRep fot the repetition of the burst mode.
 * ledStatus(read only attribute) the status of the led on or off.
 */
static struct kobj_attribute pressCount_attr = __ATTR(buttonStats, 0666, buttonStats_show,
   buttonStats_store);
static struct kobj_attribute ledMode_attr = __ATTR(ledMode, 0666, ledMode_show, ledMode_store);
static struct kobj_attribute busrtRep_attr = __ATTR(burst, 0666, burstRep_show, burstRep_store);
static struct kobj_attribute ledStatus_attr = __ATTR_RO(ledStatus);


/**
 * struct that list the attributes of the arduino LKM.
 */
static struct attribute *arduino_attrs[] = {
      &pressCount_attr.attr,
      &ledStatus_attr.attr,
      &ledMode_attr.attr,
      &busrtRep_attr.attr,
      NULL,
};

/**
 * A attribute group that will have the arduino port name and the previous defined attribute array
 * arduino_attrs[].
 */
static struct attribute_group attr_group = {
      .name  = arduinoPort,
      .attrs = arduino_attrs,
};

static struct kobject *arduino_kObject;


/**
 * @brief The LKM initialization function
 * @return returns 0 if successful
 */
static int __init arduinoLKM_init(void){
   int result = 0;
   arduinoFile= file_open(arduinoLocation);
   if(!arduinoFile){
      printk("ArduinoLKM: cannot open the arduino.%d\n", PTR_ERR(arduinoFile));
      return -ENOMEM;
   }
   else{
      printk( "ArduinoLKM:  The arduino has been opened. %d\n", PTR_ERR(arduinoFile));
   }
   printk(KERN_INFO "ArduinoLKM: Initializing the Arduino LKM.\n");
   // create the kobject sysfs entry at /sys/Arduino -- probably not an ideal location!
   arduino_kObject = kobject_create_and_add("Arduino", kernel_kobj->parent);
   if(!arduino_kObject){
      printk(KERN_ALERT "ArduinoLKM: cannot create the kobject.\n");
      return -ENOMEM;
   }
   // add the attributes to /sys/Arduino/
   result = sysfs_create_group(arduino_kObject, &attr_group);
   if(result) {
      printk(KERN_INFO "ArduinoLKM: cannot create the sysfs group\n");
      kobject_put(arduino_kObject);
      return result;
   }
   ledStatus = false;

   //Just for test
   printk(KERN_INFO "ArduinoLKM: Init Done.\n");
   return result;
}

/**
 * @brief The arduinoLKM cleanup function
 */
static void __exit arduinoLKM_exit(void){
   printk(KERN_INFO "ArduinoLKM: The button has been pressed %d times\n", buttonStats);
   kobject_put(arduino_kObject);                   // clean up -- remove the kobject sysfs entry
   printk(KERN_INFO "ArduinoLKM: Closing device!\n");
   file_close(arduinoFile);
   printk(KERN_INFO "ArduinoLKM: Goodbye from the Arduino LKM!\n");
}

module_init(arduinoLKM_init);
module_exit(arduinoLKM_exit);

/**
 * @file   ArduinoLKM.c
 * @author Malcolm Davis
 * @author Arturo Chinchilla
 * @date   March 23 2015
 * @brief  A kernel module for controlling a arduino led project
 * has 2 modes, flash or burst, suports interruptions
 * and can be modified on linux user space with echo.
 * @see https://github.com/LKMInvestigation/Linux-Kernel-Module
*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>  
#include <linux/interrupt.h> 
#include <linux/kobject.h>  

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Malcolm Davis");
MODULE_AUTHOR("Arturo Chinchilla");
MODULE_DESCRIPTION("LKM for Linux 3.x which will be written in pure C and interact with an Arduino device.");
MODULE_VERSION("0.1");

static int interruptionNumber;
static int buttonStats = 0;
static int burstRep = 1;
static bool ledStatus = 0;
static bool ledMode = 0;
static char arduinoPort[8] = "ttyACM0"; 

/**
 * @brief Custom function that handles the interruptions of the arduino.
 * @param interruptNumber the IRQ number that is associated with the arduino.
 * @param device_id the a pointer to a device identification to know wich device is the one that triggers the interruption.
 * @param registers   hardware specific register values.
 * return returns IRQ_HANDLED if successful or IRQ_NONE if not.
 */
 static irq_handler_t  arduino_irq_handler(unsigned int interruptNumber, void *device_id, struct pt_regs *registers);

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
   return sprintf(buffer, "%d\n", ledStatus);
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
 const char *buffer, size_t count){
   sscanf(buffer, "%du", &ledMode);
   return count;
}

/**
 * @brief Shows the repetition number of the burst function.
 * @param kernelObject represents a kernel object device that appears in the sysfs filesystem
 * @param attribute the attribute of the object.
 * @param buffer the buffer that will have repetition number.
*/
 static ssize_t burstRep_show(struct kobject *kernelObject, struct kobj_attribute *attribute, char *buffer){
   return sprintf(buffer, "%d\n", burst);
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
   sscanf(buffer, "%du", &burst);
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

static struct kobject arduino_kObject;

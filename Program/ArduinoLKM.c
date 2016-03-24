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
#include <unistd.h> 

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
   int tmp = (int)ledMode;
   return sprintf(buffer, "%d\n", tmp);
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
   int tmp = (int)ledMode;
   sscanf(buffer, "%du", &tmp);
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
   
   /***********************/
   //unsigned long irqFlags = IRQF_TRIGGER_RISING;
   /************************/

   printk(KERN_INFO "ArduinoLKM: Initializing the Arduino LKM\n");
   // create the kobject sysfs entry at /sys/Arduino -- probably not an ideal location!
   arduino_kObject = kobject_create_and_add("Arduino", kernel_kobj->parent); 
   if(!arduino_kObject){
      printk(KERN_ALERT "ArduinoLKM: cannot create the kobject\n");
      return -ENOMEM;
   }
   // add the attributes to /sys/Arduino/
   result = sysfs_create_group(arduino_kObject, &attr_group);
   if(result) {
      printk(KERN_ALERT "ArduinoLKM: cannot create the sysfs group\n");
      kobject_put(arduino_kObject);
      return result;
   }
   ledStatus = false;

/*  // Perform a quick test to see that the button is working as expected on LKM load
   printk(KERN_INFO "ArduinoLKM: The button state is currently: %d\n", gpio_get_value(gpioButton));
 
   /// Maps the serial to the IRQ
   irqNumber = gpio_to_irq(gpioButton);
   printk(KERN_INFO "ArduinoLKM: The button is mapped to IRQ: %d\n", irqNumber);
 
   if(!isRising){
      IRQflags = IRQF_TRIGGER_FALLING;
   }

   // Calls the Interruption
   result = request_irq(interruptionNumber, (irq_handler_t) arduino_irq_handler, irqFlags,
   "Arduino_Button_Handler", NULL);
   */
   result=0;//Just for test
   return result;
}

/** 
 * @brief The arduinoLKM cleanup function
 */
static void __exit arduinoLKM_exit(void){
   printk(KERN_INFO "ArduinoLKM: The button has been pressed %d times\n", buttonStats);
   kobject_put(arduino_kObject);                   // clean up -- remove the kobject sysfs entry
   free_irq(interruptionNumber, NULL);               // Free the IRQ number, no *dev_id required in this case
   printk(KERN_INFO "ArduinoLKM: Goodbye from the Arduino LKM!\n");
}
 
/** 
 * @brief The arduino event Handler function
 * @param irq    the IRQ number that is associated.
 * @param dev_id the identificator of the device.
 * @param regs registers bound to the hardware.
 * return returns IRQ_HANDLED if successful or IRQ_NONE if not.
 */
static irq_handler_t ebbgpio_irq_handler(unsigned int irq, void *dev_id, struct pt_regs *regs){
   ledStatus = !ledStatus;                      // Invert the LED state on each button press
   //printk(KERN_INFO "ArduinoLKM: The button state is currently: %d\n", gpio_get_value(gpioButton));
   buttonStats++;                     // Global counter, will be outputted when the module is unloaded
   return (irq_handler_t) IRQ_HANDLED;  // Announce that the IRQ has been handled correctly
}

module_init(arduinoLKM_init);
module_exit(arduinoLKM_exit);
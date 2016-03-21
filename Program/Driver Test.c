#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>

//probe function
static int pen_probe(struct usb_interface* iterface, const struct usb_device_id* id)
{
	printk(KERN_INFO "[*] SolidusCode Pen Drive(%04x:%04x) plugged\n", id->idVendor, id->idProduct);
	return 0;
};
static void pen_disconnect(struct usb_interface * interface)
{
	|printk(KERN_INFO "[*] Malcolm Pen Drive removed\n");
};
//usb_device_id
static struct usb_device_id pen_table[] =
{
	//2341:0043, 0951:1694
	{USB_DEVICE(0x0951, 0x1694) },
	{}
};
MODULE_DEVICE_TABLE(usb, pen_table);

//usb_driver
static struct usb_driver pen_driver = {
	.name = "Malcolm-Usb Stick-driver",
	.id_table =pen_table,
	.probe = pen_probe,
	.disconnect = pen_disconnect
};

static int __init pen_init(void){
	int ret=-1;
	printk(KERN_INFO "[*]Malcolm Constructor of driver");
	printk(KERN_INFO "\tRegistering Driver with kernel");
	ret = usb_register(&pen_driver);
	printk(KERN_INFO "\tRegistration is complete")
	return ret;
}

static void __exit pen_exit(void){
	//deregister
	printk(KERN_INFO "[*]Malcolm destructor of driver");
	ret = usb_deregister(&pen_driver);
	printk(KERN_INFO "\tUnregistration complete");
}

module_init(pen_init);
module(pen_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Malcolm");
MODULE_DESCRIPTION("Usb pen registration");
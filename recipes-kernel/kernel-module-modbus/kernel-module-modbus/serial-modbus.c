//--------------------------------------------------------------------------------------------------------------------//
//! @file serial-modbus.c
//!
//! @brief Entry point into example application using Quantum Framework.
//! @section DESCRIPTION
//! The following file is responsible for initializing the framework, starting the application state machines, and 
//! defining external framework dependencies. Finally, control is given to the framework to manage execution of the 
//! application.
//! @copyright     Hach Confidential
//!                Copyright(c) (2017)
//!                Hach
//!                All Rights Reserved
//! This product is protected by copyright and distributed under
//! Hach Software License, Version 1.0 (See accompanying file LICENSE_1.0.txt)
//--------------------------------------------------------------------------------------------------------------------//

#include <linux/module.h>

#include <linux/cdev.h>
#include <linux/fs.h>                               // File operations.
#include <linux/device.h>                           // class_create(), device_create()
#include <linux/slab.h>                             // kmalloc(), kfree()

// The following class and device names result in the creation of a device that appears in the file system
// at "/sys/class/modbus_class/modbus_dev".
static const char DEVICE_NAME[] = "modbus_dev";
static const char CLASS_NAME[] = "modbus_class";

static struct cdev* my_dev;
static dev_t firstAssignedDevNum;                   // First device number being requested, major and minor.
static const unsigned int firstOfMinorNum = 0;  
static const unsigned int countOfMinorNum = 1;      // Range of contiguous minor numbers associated with this driver's 
                                                    // major.
static struct class*  modbusClass = NULL;           // class struct pointer.
static struct device* modbusDevice = NULL;          // device struct pointer.

// The following structure is used to contain data specific to opened instances of this device driver and maintainted
// within the private_data field of the file structure.
struct private_data {
    int tempPlaceholder;
};

//--------------------------------------------------------------------------------------------------------------------//
//! This callback function is executed by the kernel when a user-application makes a system call to open the file, or
//! device node, associated with this module within the virtual file system.
//
//! @retval int - 0: success
//
static int modbus_dev_open(struct inode* inode,     //!< [in] inode structure represent the file system item associated
                                                    //!< with this device node. It will contain the minor number to
                                                    //!< differentiate when more than one device is managed by this
                                                    //!< driver. There is only one inode structure pointing to a given
                                                    //!< device node, regardless of how many times it is opened.
                           struct file* file)       //!< [in] A unique file structure is supplied for each opened device
                                                    //!< node. Inside is private data field where information specific
                                                    //!< to this device node can be stored. When you operate on the file
                                                    //!< structure in other entry points, you can access the same private
                                                    //!< data associated with that device node.
{
    struct private_data* myPrivateData;

    // Allocate memory for private data and associate into file structure specific to this open instance.
    myPrivateData = kzalloc(sizeof(struct private_data), GFP_KERNEL);
    if(!myPrivateData) {
        pr_err("Failed to allocate memory for private data.\n");
        return -ENOMEM;
    }
    file->private_data = myPrivateData;

	pr_info("modbus_dev_open() is called.\n");
	return 0;
}

//--------------------------------------------------------------------------------------------------------------------//
//! This callback function is executed by the kernel when a user-application makes a system call to close the file, or
//! device node, associated with this module within the virtual file system.
//
//! @retval int - 0: success
//
static int modbus_dev_close(struct inode* inode,    //!< [in] inode structure represent the file system item associated
                                                    //!< with this device node. It will contain the minor number to
                                                    //!< differentiate when more than one device is managed by this
                                                    //!< driver.
                            struct file* file)      //!< [in] A unique file structure is supplied for each opened device
                                                    //!< node. Inside is private data field where information specific
                                                    //!< to this device node can be stored. When you operate on the file
                                                    //!< structure in other entry points, you can access the same private
                                                    //!< data associated with that device node.
{
    struct private_data* myPrivateData;
    
    // Access private data associated with the file structure specific to this open instance.
    myPrivateData = (struct private_data*)file->private_data;
    if(!myPrivateData) {
        pr_err("Failed to access memory for private data.\n");
        return -ENOMEM;
    }

    // Free resources.
    kfree(myPrivateData);
   
	pr_info("modbus_dev_close() is called.\n");
	return 0;
}

//--------------------------------------------------------------------------------------------------------------------//
//! This function provides 'out-of-bad' I/O control between user-space and this driver and is executed by the kernel 
//! when a user-space application invokes ioctl() on this device node and the system call then calls this entry point.
//! It is used to issue device-specific commands.
//! @retval int - 0: success
//
static long modbus_dev_ioctl(struct file* file, 
                             unsigned int cmd, 
                             unsigned long arg)
{
    struct private_data* myPrivateData;
    
    // Access private data associated with the file structure specific to this open instance.
    myPrivateData = (struct private_data*)file->private_data;
    if(!myPrivateData) {
        pr_err("Failed to access memory for private data.\n");
        return -ENOMEM;
    }

	pr_info("modbus_dev_ioctl() is called. cmd = %d, arg = %ld\n", cmd, arg);
	return 0;
}

//--------------------------------------------------------------------------------------------------------------------//
//! This function reads from the device, returning the number of bytes read.
//
//! @retval int - <0: error, 0: may mean end of device and not an error, >0: number of bytes read
//
static ssize_t modbus_dev_read(struct file* file,   //!< [in] Open instance of a file. Can be useful to take advantage
                                                    //!< of the private data field to access information specific to this
                                                    //!< minor number.
                               char __user* buf,    //!< [in] Buffer to user space containing data to be written. 
                               size_t lbuf,         //!< [in] Size of the user space buffer.
                               loff_t *ppos)        //!< [in, out] Position in buffer,
{
    struct private_data* myPrivateData;
    
    // Access private data associated with the file structure specific to this open instance.
    myPrivateData = (struct private_data*)file->private_data;
    if(!myPrivateData) {
        pr_err("Failed to access memory for private data.\n");
        return -ENOMEM;
    }

	pr_info("modbus_dev_read() is called.\n");
    return 0;
}

//--------------------------------------------------------------------------------------------------------------------//
//! This function writes to the device, returning the number of bytes witten.
//
//! @retval int - <0: error, 0: may mean end of device and not an error, >0: number of bytes written.
//
static ssize_t
modbus_dev_write(struct file* file,                 //!< [in] Open instance of a file. Can be useful to take advantage
                                                    //!< of the private data field to access information specific to this
                                                    //!< minor number.
                 const char __user* buf,            //!< [in] Buffer to user space containing data to be written. 
                 size_t lbuf,                       //!< [in] Size of the user space buffer.
                 loff_t *ppos)                      //!< [in, out] Position in buffer,
{
    struct private_data* myPrivateData;
    
    // Access private data associated with the file structure specific to this open instance.
    myPrivateData = (struct private_data*)file->private_data;
    if(!myPrivateData) {
        pr_err("Failed to access memory for private data.\n");
        return -ENOMEM;
    }

	pr_info("modbus_write_read() is called.\n");
    return 0;
}

// Define the entry points, or callback functions, implemented by this driver when a system call is made from user-space.
static const struct file_operations modbus_dev_fops = {
    .owner = THIS_MODULE,                           // When this device driver is opened, the kernel will poke the owner
                                                    // field and increment the modules' reference count, so the module 
                                                    // doesn't unload when using it.
    .read = modbus_dev_read,
    .write = modbus_dev_write,    
    .open = modbus_dev_open,
	.release = modbus_dev_close,
	.unlocked_ioctl = modbus_dev_ioctl,
};

static int __init modbus_dev_init(void)
{
    int ret;

    pr_info("modbus_dev_init() is called.\n");

	// Automatically allocate an available major and the range of minor numbers to go with it, return the first in
    // firstAssignedDevNum.
	ret = alloc_chrdev_region(&firstAssignedDevNum, firstOfMinorNum, countOfMinorNum, DEVICE_NAME); 
    if (ret < 0){ 
        pr_err("Failed to allocate device region.\n"); 
        return ret; 
    } 

    // Allocate memory for the structure, initialize it, and sets the owner and ops fields to point to the current
    // module and the proper file_operations table.
    my_dev = cdev_alloc();
    if(!my_dev) {
        pr_err("Failed to allocate memory for device structure.\n");
        return -1;
    }
	cdev_init(my_dev, &modbus_dev_fops);

    // Make the driver live in the system.
	ret= cdev_add(my_dev, firstAssignedDevNum, countOfMinorNum);
	if(ret < 0) {
		pr_err("Unable to add cdev\n");
        cdev_del(my_dev);
		unregister_chrdev_region(firstAssignedDevNum, countOfMinorNum);
		return ret;
	}

    // Register the device class.
	modbusClass = class_create(THIS_MODULE, CLASS_NAME);
	if(IS_ERR(modbusClass)) {
	    pr_err("Failed to register device class.\n");
        cdev_del(my_dev);
		unregister_chrdev_region(firstAssignedDevNum, countOfMinorNum);
	    return PTR_ERR(modbusClass);
	}

    // Create the device and register a node in sysfs.
	modbusDevice = device_create(modbusClass, NULL, firstAssignedDevNum, NULL, "%s%d", DEVICE_NAME,
                                                                                       MINOR(firstAssignedDevNum));
	if(IS_ERR(modbusDevice)) {
	    pr_info("Failed to create the device.\n");
	    class_destroy(modbusClass);
        cdev_del(my_dev);
		unregister_chrdev_region(firstAssignedDevNum, countOfMinorNum);
	    return PTR_ERR(modbusDevice);
	}
   
    dev_info(modbusDevice, "Succeeded in registering character device %s, Major=%d, Minor=%d.\n", DEVICE_NAME,
                                                                                                  MAJOR(firstAssignedDevNum),
                                                                                                  MINOR(firstAssignedDevNum));

    return 0;
}

static void __exit modbus_dev_exit(void)
{
    dev_info(modbusDevice, "Device unregistered.\n");

    // Remove the device from the system.
    if(my_dev) {
        cdev_del(my_dev);
    }
    device_destroy(modbusClass, firstAssignedDevNum);

    // Unregister the device class.
	class_unregister(modbusClass);

    // Remove the device class.
	class_destroy(modbusClass);
    
    // Unregister the device numbers and remove the association with device numbers.
    unregister_chrdev_region(firstAssignedDevNum, countOfMinorNum);
}

module_init(modbus_dev_init);
module_exit(modbus_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tim Higgins");
MODULE_DESCRIPTION("Modbus RTU implementation.");

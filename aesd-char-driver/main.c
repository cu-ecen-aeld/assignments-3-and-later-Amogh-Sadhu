/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h> 
#include <linux/fs.h> // file_operations
#include <linux/slab.h>    // kmalloc/kfree/krealloc
#include <linux/kernel.h>  // container_of
#include <linux/mutex.h>   // mutex
#include "aesdchar.h"
int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("AmoghSadhu"); /** TODO: fill in your name **/
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;

int aesd_open(struct inode *inode, struct file *filp)
{
    struct aesd_dev *dev;
    PDEBUG("open");

    dev = container_of(inode->i_cdev, struct aesd_dev, cdev);

    filp->private_data = dev;
    return 0;
}

int aesd_release(struct inode *inode, struct file *filp)
{
    PDEBUG("release");
    /**
     * TODO: handle release
     */
    return 0;
}

ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
    PDEBUG("read %zu bytes with offset %lld",count,*f_pos);

    struct aesd_dev *dev = filp->private_data;
    struct aesd_buffer_entry *entry;
    size_t entry_offset_byte = 0;
    ssize_t retval = 0;

    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;


    entry = aesd_circular_buffer_find_entry_offset_for_fpos(&dev->buffer, *f_pos, &entry_offset_byte);

    if (entry) {
        // check how many bytes are left in the entry
        size_t available = entry->size - entry_offset_byte;
        size_t bytes_to_copy = min(count, available);

        // Send the data
        if (copy_to_user(buf, entry->buffptr + entry_offset_byte, bytes_to_copy)) {
            retval = -EFAULT;
        } else {
            retval = bytes_to_copy;
            *f_pos += bytes_to_copy; // updating the pos.
        }
    }

    mutex_unlock(&dev->lock);
    /**
     * TODO: handle read
     */
    return retval;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
    struct aesd_dev *dev = filp->private_data;
    ssize_t retval = -ENOMEM;

    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;

    // Get the memory from kernel if already have it and want to expand it used krealloc    
    // used temp_ptr cause if krealloc fails it would set our original pointer to NULL
    // and it will be lost
    char *tmp_ptr = krealloc(dev->working_entry.buffptr, 
                             dev->working_entry.size + count, 
                             GFP_KERNEL);
    if (!tmp_ptr) {
        // We don't overwrite the original buffptr, so it's still safe in memory
        goto out; 
    }
    //after successfull allocation set our pointer 
    dev->working_entry.buffptr = tmp_ptr;

    // copy the data from user with size offset to not overwrite the data
    // if it is a recurring send and we havent yet got \n
    if (copy_from_user((char *)dev->working_entry.buffptr + dev->working_entry.size, 
                       buf, count)) {
        retval = -EFAULT;
        goto out;

    }

    // Update the size of our temporary accumulator
    dev->working_entry.size += count;
    retval = count;

    // look for \n character and if found update the circular buffer
    if (memchr(dev->working_entry.buffptr, '\n', dev->working_entry.size)) {
        // if buffer is full and we have to overwrite it
        // First free the memory on that old location
        if (dev->buffer.full) {
            void *old_ptr = (void *)dev->buffer.entry[dev->buffer.in_offs].buffptr;
            kfree(old_ptr);
        }

        // this will add the new entry to the buffer overwritting the old pointer
        aesd_circular_buffer_add_entry(&dev->buffer, &dev->working_entry);

        dev->working_entry.buffptr = NULL;
        dev->working_entry.size = 0;
    }

out:
    mutex_unlock(&dev->lock);
    return retval;
}

struct file_operations aesd_fops = {
    .owner =    THIS_MODULE,
    .read =     aesd_read,
    .write =    aesd_write,
    .open =     aesd_open,
    .release =  aesd_release,
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
    int err, devno = MKDEV(aesd_major, aesd_minor);

    cdev_init(&dev->cdev, &aesd_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &aesd_fops;
    err = cdev_add (&dev->cdev, devno, 1);
    if (err) {
        printk(KERN_ERR "Error %d adding aesd cdev", err);
    }
    return err;
}


int aesd_init_module(void)
{
    dev_t dev = 0;
    int result;
    result = alloc_chrdev_region(&dev, aesd_minor, 1,
            "aesdchar");
    aesd_major = MAJOR(dev);
    if (result < 0) {
        printk(KERN_WARNING "Can't get major %d\n", aesd_major);
        return result;
    }
    memset(&aesd_device,0,sizeof(struct aesd_dev));

    mutex_init(&aesd_device.lock);

    aesd_circular_buffer_init(&aesd_device.buffer);

    /**
     * TODO: initialize the AESD specific portion of the device
     */

    result = aesd_setup_cdev(&aesd_device);

    if( result ) {
        unregister_chrdev_region(dev, 1);
    }
    return result;

}

void aesd_cleanup_module(void)
{
    struct aesd_buffer_entry *entry;
    uint8_t index;

    dev_t devno = MKDEV(aesd_major, aesd_minor);

    cdev_del(&aesd_device.cdev);

    if(aesd_device.working_entry.buffptr){
        kfree((void *)aesd_device.working_entry.buffptr);   
    }
    
    AESD_CIRCULAR_BUFFER_FOREACH(entry, &aesd_device.buffer, index){
    if (entry->buffptr) {
            kfree((void *)entry->buffptr);
        }
    }

    mutex_destroy(&aesd_device.lock);
    /**
     * TODO: cleanup AESD specific poritions here as necessary
     */

    unregister_chrdev_region(devno, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);

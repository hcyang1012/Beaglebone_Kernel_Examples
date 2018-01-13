#include <linux/module.h>       
#include <linux/kernel.h>       
#include <linux/init.h>        
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/input.h>

#define MODULE_NAME "Virtual Keyboard"
#define NODE_NAME "virt_kbd"
#define LOG(MSG) MODULE_NAME " : " MSG "\n"

static u8 keyPressed = 0u;

static struct input_dev *virtKeyabord_dev = NULL;

static int virtKeyboard_open(struct inode *inode, struct file *file){
    pr_info("virtKeyboard_open\n");
    return 0;
}

static int virtKeyboard_close(struct inode *inodep, struct file *filep){
    pr_info("virtKeyboard_close\n");
    return 0;
}

static ssize_t virtKeyboard_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos){
    u32 keycode = 0u;
    u8 *kernel_buf = (u8*)kzalloc(len,GFP_KERNEL);
    int ret = 0;
    pr_info(LOG("Length : %d"),len);
    if(kernel_buf == NULL){
        pr_err(LOG("Not enough memory"));
        return -ENOMEM;
    }
    
    ret = copy_from_user(kernel_buf,buf,len);
    if (ret == 0u){
        ret = sscanf(kernel_buf,"%x",&keycode);
        if(ret != 1u){
            pr_err(LOG("Failed to read keycode"));
            kfree(kernel_buf);
            return -ENOMEM;
        }
        
        
        pr_info(LOG("Keycode input : %x"),keycode);
        kfree(kernel_buf);        
        
        keyPressed = (~keyPressed);
        input_report_key(virtKeyabord_dev, KEY_A,keyPressed);
        
        input_sync(virtKeyabord_dev);
        
        
        return len;
    }else{
        pr_err(LOG("copy_from_user error : %d"),ret);
        kfree(kernel_buf);
        return -1;
    }
    

}

static const struct file_operations virtKeyboard_fops = {
    .owner = THIS_MODULE,
    .write = virtKeyboard_write,
    .open = virtKeyboard_open,
    .release = virtKeyboard_close,
    .llseek = no_llseek,
};


struct miscdevice sample_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = NODE_NAME,
    .fops = &virtKeyboard_fops,
};

static int init_input_driver(void){
    int error;
    virtKeyabord_dev = input_allocate_device();
    if (!virtKeyabord_dev) {
            pr_err(KERN_ERR LOG("Not enough memory"));
            error = -ENOMEM;
            return error;
    }
    
    virtKeyabord_dev->evbit[0] = BIT_MASK(EV_KEY);
    set_bit(KEY_A, virtKeyabord_dev->keybit);
    keyPressed = 0;
    input_report_key(virtKeyabord_dev, KEY_A,keyPressed); 
    input_sync(virtKeyabord_dev);
    
    

    error = input_register_device(virtKeyabord_dev);
    if (error) {
           pr_err(LOG("Failed to register keyboard driver"));
           return -1;
    }    
		
    
    return 0;
}

static int __init init(void)
{
    int error;

    error = misc_register(&sample_device);
    if(error){
        pr_err("can't misc_register \n");
        return error;
    }    
    
    if((error = init_input_driver()) != 0u){
        pr_err(LOG("Failed to initialize input driver"));
        return error;
    }

    
    printk(KERN_INFO LOG("Module loaded"));
    return 0;
}

static void __exit release(void)
{
    input_unregister_device(virtKeyabord_dev);
    misc_deregister(&sample_device);
    pr_info("misc exit success\n");    
    printk(KERN_INFO LOG("Module released"));
}

module_init(init);
module_exit(release);

MODULE_AUTHOR("Heecheol Yang");
MODULE_DESCRIPTION(MODULE_NAME);
MODULE_LICENSE("GPL");

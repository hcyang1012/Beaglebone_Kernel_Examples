#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_platform.h>

static int sample_open(struct inode *inode, struct file *file){
    pr_info("sample_open\n");
    return 0;
}

static int sample_close(struct inode *inodep, struct file *filep){
    pr_info("sample_close\n");
    return 0;
}

static ssize_t sample_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos){
    pr_info("sample_write\n");
    return len;
}

static const struct file_operations sample_fops = {
    .owner = THIS_MODULE,
    .write = sample_write,
    .open = sample_open,
    .release = sample_close,
    .llseek = no_llseek,
};


struct miscdevice sample_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "simple misc",
    .fops = &sample_fops,
};

#if defined (CONFIG_OF)
    static const struct of_device_id my_device_of_match[] = {
        { .compatible = "my_dev"},
        {},
    };
    MODULE_DEVICE_TABLE(of, my_device_of_match);
#endif

static int my_device_probe(struct platform_device *pdev){
    pr_info("probe success\n");
    return 0;
}

static int my_device_remove(struct platform_device *pdev){
    pr_info("remove success\n");
    return 0;
}

static struct platform_driver my_device_driver = {
    .probe = my_device_probe,
    .remove = my_device_remove,
    .driver = {
            .name = "MY_DRIVER",
            .of_match_table = of_match_ptr(my_device_of_match),
    },
};


static int __init misc_init(void){
    int error;

    error = misc_register(&sample_device);
    if(error){
        pr_err("can't misc_register \n");
        return error;
    }

    error = platform_driver_register(&my_device_driver);
    if(error){
        pr_err("Can't register platform driver\n");
        return error;
    }
    pr_info("misc init success\n");
    return 0;

}


static void __exit misc_exit(void){
    platform_driver_unregister(&my_device_driver);
    misc_deregister(&sample_device);
    pr_info("misc exit success\n");
}

module_init(misc_init);
module_exit(misc_exit);

MODULE_DESCRIPTION("Simple Misc Driver");
MODULE_AUTHOR("Heecheol, Yang (heecheol.yang@outlook.com)");
MODULE_LICENSE("GPL");

#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_platform.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>

struct i2c_client *i2c_client_rtc = NULL;

static int sample_open(struct inode *inode, struct file *file){
	struct i2c_msg msg[2];
	int error;
	unsigned char reg;
	unsigned char buf;

	if(i2c_client_rtc == NULL){
		pr_err("I2C Client for RTC is NULL\n");
		return -1;
	}

	reg = 0x02;
	msg[0].addr = i2c_client_rtc->addr;
	msg[0].flags = 0;
	msg[0].len = 1;
	msg[0].buf = &reg;

	msg[1].addr = i2c_client_rtc->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = 1;
	msg[1].buf = &buf;

	error = i2c_transfer(i2c_client_rtc->adapter,msg,2);
	if(error != 2){
		pr_err("i2c_transfer error\n");
		return -1;
	}
	pr_info("Current seconds : %d sec\n",buf);
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

static const struct i2c_device_id my_device_id[] = {
	{"my_dev",0},
	{}
};
MODULE_DEVICE_TABLE(i2c,my_device_id);
#if defined (CONFIG_OF)
static const struct of_device_id my_device_of_match[] = {
	{ .compatible = "my_dev",},
	{},
};
MODULE_DEVICE_TABLE(of, my_device_of_match);
#endif

/* Note that the prototype of ->probe() function is changed suitable for I2C infrastructure */
static int my_device_probe(struct i2c_client *client, const struct i2c_device_id *id){
	i2c_set_clientdata(client,NULL);
	i2c_client_rtc = client;
	pr_info("probe success\n");
	return 0;
}

static int my_device_remove(struct i2c_client *client ){
	pr_info("remove success\n");
	return 0;
}

static struct i2c_driver  my_device_driver = {
	.probe = my_device_probe,
	.remove = my_device_remove,
	.id_table = my_device_id,
	.driver = {
		.name = "MY_DRIVER",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(my_device_of_match),
	},
};



static int __init misc_init(void){
	int error;

	error = i2c_add_driver(&my_device_driver);
	if(error){
		pr_err("Probe error : %d\n",error);
		return error;
	}
	error = misc_register(&sample_device);
	if(error){
		pr_err("can't misc_register \n");
		return error;
	}

	pr_info("misc init success\n");
	return 0;

}

static void __exit misc_exit(void){
	i2c_del_driver(&my_device_driver);
	misc_deregister(&sample_device);
	pr_info("misc exit success\n");
}
module_init(misc_init);
module_exit(misc_exit);

MODULE_DESCRIPTION("Simple Misc Driver");
MODULE_AUTHOR("Heecheol, Yang (heecheol.yang@outlook.com)");
MODULE_LICENSE("GPL");

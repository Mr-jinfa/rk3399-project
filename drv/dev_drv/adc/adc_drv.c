#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/ioport.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/iio/iio.h>
#include <linux/iio/machine.h>
#include <linux/iio/driver.h>
#include <linux/iio/consumer.h>
struct iio_channel *chan = NULL;

ssize_t gec3399_adc_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
	int ret,val;
	ret = iio_read_channel_raw(chan,&val);
	if(ret < 0)
	{
		printk("%s() have not read adc value\n", __FUNCTION__);
	}
	printk("val = %d\n",val);
	ret = copy_to_user(buf,&val,4);
	if(ret < 0)
	{
		printk("copy_to_user errot\n");
		return ret;
	}
	return 0;
}

static int gec3399_adc_release(struct inode *inode, struct file *file)
{
	return 0;
}

static const struct file_operations gec3399_adc_fops = {
	.owner = THIS_MODULE,
	.read = gec3399_adc_read,
	.release = gec3399_adc_release,
}; //文件操作集结构体
static struct miscdevice gec3399_adc_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &gec3399_adc_fops,
	.name = "adc_drv",
}; //混杂设备结构体定义和初始化

static int gec3399_adc_probe(struct platform_device *pdev)
{
	int ret;
	printk("gpio-adc math succee\n");
	ret = misc_register(&gec3399_adc_misc); //注册字符设备
	if(ret < 0){
		printk("%s misc register error\n",__FUNCTION__);
		goto err_misc_register;
	}
	chan = iio_channel_get(&(pdev->dev), NULL); //从设备树获取通道
	if (IS_ERR(chan))
	{
		chan = NULL;
		printk("%s() have not get chan\n", __FUNCTION__);
		ret = -1;
		goto err_channel_get;
	}
	printk("adc dirve install succee\n");
	return 0;
err_channel_get:
	misc_deregister(&gec3399_adc_misc);
err_misc_register:
	return ret;
}

static int gec3399_adc_remove(struct platform_device *pdev)
{
	iio_channel_release(chan);
	misc_deregister(&gec3399_adc_misc);
	printk("adc dirve rmove succee\n");
	return 0;
}
static const struct of_device_id of_gec_buzer_match[] = {
{ .compatible = "adc-adres", }, //compatible 兼容属性名，需与设备树节点的属性一致
{},
};
static struct platform_driver gec3399_adc_driver = {
	.probe = gec3399_adc_probe, //驱动探测
	.remove = gec3399_adc_remove, //驱动移除
	.driver = {
	.name ="adc-adres",
	.owner = THIS_MODULE,
	.of_match_table = of_gec_buzer_match, //设备树设备匹配
	},
};

module_platform_driver(gec3399_adc_driver);
MODULE_AUTHOR("bobeyfeng@163.com");
MODULE_DESCRIPTION("adc driver for GEC6818");
MODULE_LICENSE("GPL"); //符合GPL 协议
MODULE_VERSION("V1.0");

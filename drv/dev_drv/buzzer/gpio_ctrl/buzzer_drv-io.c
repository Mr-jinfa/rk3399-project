#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>

#include <linux/device.h>
#include <linux/ioport.h>
#include <linux/ioctl.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#define BUZZER_START _IO('B',0)
#define BUZZER_STOP _IO('B',1)

int gpio_buz;
static long gec3399_buzzer_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch(cmd)
	{
		case BUZZER_START:
			gpio_set_value(gpio_buz,1);
		break;
		case BUZZER_STOP :
			gpio_set_value(gpio_buz,0);
		break;
	}
return 0;
}

static int gec3399_buzzer_release(struct inode *inode, struct file *file)
{
	gpio_set_value(gpio_buz,0); //应用程序退出时，关闭蜂鸣器
	return 0;
}

static const struct file_operations gec3399_buzzer_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = gec3399_buzzer_ioctl,
	.release = gec3399_buzzer_release,
}; //文件操作集结构体

static struct miscdevice gec3399_buzzer_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &gec3399_buzzer_fops,
	.name = "buzzer_drv",
}; //混杂设备结构体定义和初始化

static int gec3399_buzzer_probe(struct platform_device *pdev)
{
	int ret;
	struct device_node *buz_node = pdev->dev.of_node;
	enum of_gpio_flags flag;
	printk("gpio-buzzer math succee\n");
	ret = misc_register(&gec3399_buzzer_misc); //注册字符设备
	if(ret < 0){
		printk("misc register error\n");
		goto err_misc_register;
	}
	gpio_buz = of_get_named_gpio_flags(buz_node,"gpio-buzzer", 0, &flag); //从设备树获取GPIO 号
	if (!gpio_is_valid(gpio_buz)) {
		printk("gpio-buzzer: %d is invalid\n",gpio_buz);
		ret = -ENODEV;
		goto err_get_gpio;
	}
	printk("gpio-buzzer = %d\n",gpio_buz);
	ret = gpio_request(gpio_buz,"ak8963c_DYDR"); //申请gpio_buz 引脚为GPIO模式
	if(ret < 0){
		printk("gpio_request gpio = ak8963c_DYDR error\n");
		goto err_get_gpio;
	}
	ret = gpio_direction_output(gpio_buz,0); //初始划蜂鸣器为关闭状态
	if(ret < 0){
		printk("gpio direction input gpio = ak8963c_DYDR error\n");
		goto err_gpio_direction;
	}
	printk("buzzer dirve install succee\n");
	return 0;
err_gpio_direction:
	gpio_free(gpio_buz);
err_get_gpio:
	misc_deregister(&gec3399_buzzer_misc);
err_misc_register:
	return ret;
}

static int gec3399_buzzer_remove(struct platform_device *pdev)
{
	gpio_free(gpio_buz);
	misc_deregister(&gec3399_buzzer_misc);
	printk("buzzer dirve rmove succee\n");
	return 0;
}

static const struct of_device_id of_gec_buzer_match[] = {
{ .compatible = "gpio-buzzer", }, //compatible 兼容属性名，需与设备树节点的属性一致
{},
};

static struct platform_driver gec3399_buzzer_driver = {
	.probe = gec3399_buzzer_probe, //驱动探测
	.remove = gec3399_buzzer_remove, //驱动移除
	.driver = {
	.name ="gpio-buzzer",
	.owner = THIS_MODULE,
	.of_match_table = of_gec_buzer_match, //设备树设备匹配
	},
};

module_platform_driver(gec3399_buzzer_driver);
//module 的描述，不是必需的。#modinfo buzzer_drv.ko
MODULE_AUTHOR("bobeyfeng@163.com");
MODULE_DESCRIPTION("buzzer driver for GEC6818");
MODULE_LICENSE("GPL"); //符合GPL 协议
MODULE_VERSION("V1.0");

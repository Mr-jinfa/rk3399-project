#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/ioport.h>
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/init.h>
#include <linux/pwm.h>
#include <linux/slab.h>

#define PWM1_BUZZER_ENABLE _IO('P',0)
#define PWM1_BUZZER_CONFIG _IO('P',1)
#define PWM1_BUZZER_DISABLE _IO('P',2)
#define buzzer_gpio (32*4 + 8*2 + 6)
struct pwm_cfg
{
	int duty_ns;
	int period_ns;
};
static struct pwm_device *pwm1buzzer; //保存申请的PWM 结构体指针

static const struct of_device_id of_match_buzzer[] = {
	{ .compatible = "pwm-buzzer", .data = NULL },
	{ /* sentinel */ }
};


static long gec3399_buzzer_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
int ret;
void *tmp;
struct pwm_cfg pwm1;
printk(KERN_INFO "gec3399_buzzer_ioctl\n");
switch(cmd)
{
	case PWM1_BUZZER_ENABLE:
		pwm_enable(pwm1buzzer); 
	break; //使能PWM
	case PWM1_BUZZER_DISABLE:
		pwm_disable(pwm1buzzer);
	break; //停止PWM
	
	case PWM1_BUZZER_CONFIG: //配置PWM
		memcpy(&tmp,&arg,sizeof(arg)); //防止编译器报错
		ret = copy_from_user(&pwm1,tmp,sizeof(pwm1));
		if(ret < 0){
			printk("copy_from_user error PWM1_BUZZER_CONFIG \n");
			return ret;
		}
		printk("duty_ns = %d\nperiod_ns = %d\n",pwm1.duty_ns,pwm1.period_ns);
		pwm_disable(pwm1buzzer);
		ret = pwm_config(pwm1buzzer,pwm1.duty_ns,pwm1.period_ns);
		if(ret < 0)
		{
			printk("pwm_config error PWM1_BUZZER_CONFIG \n");
			return ret;
		}
		pwm_enable(pwm1buzzer);
		break;
	}
	return 0;
}

static const struct file_operations gec3399_buzzer_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = gec3399_buzzer_ioctl,
};
static struct miscdevice miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "buzzer_drv",
	.fops = &gec3399_buzzer_fops,
};

static int buzzer_probe(struct platform_device *pdev)
{
	int ret, pwm_probe=0;
	printk("gec3399 buzzer probe\n");
	ret = misc_register(&miscdev); //注册混杂设备
	if(ret < 0){
		printk("misc driver register error\n");
		goto break0;
	}
	of_property_read_s32(pdev->dev.of_node, "pwm-buzzer", &pwm_probe);
	if (!pwm_probe) 
	{
		printk("can not get pin for pwm\n");
		goto break1;
	}
	
	pwm1buzzer = pwm_request(pwm_probe, "buzzer_pwm"); //申请PWM1
	if (IS_ERR(pwm1buzzer)) {
		printk("request pwm %d for %s failed\n", pwm_probe,"buzzer_pwm");
		ret = -ENODEV;
		goto break1;
	}
	pwm_disable(pwm1buzzer);
	return 0;

break1:
	misc_deregister(&miscdev);

break0:
	return ret; //返回错误的原因
}

static int buzzer_remove(struct platform_device *pdev)
{
	printk(KERN_INFO "platform driver buzzer exit\n");
	pwm_free(pwm1buzzer);
	misc_deregister(&miscdev);
	return 0;
}


struct platform_driver buzzer_drv = {
	.probe		= buzzer_probe,
	.remove		= buzzer_remove,
	.driver		= {
		.name	= "buzzer_drv",
		.of_match_table = of_match_buzzer,
	}
};


//驱动的安装函数
static int __init gec3399_buzzer_init(void)
{
	platform_driver_register(&buzzer_drv);
	return 0;
}

//驱动的卸载函数
static void __exit gec3399_buzzer_exit(void)
{
	platform_driver_unregister(&buzzer_drv);
	printk(KERN_INFO "gec3399_buzzer_exit\n");
}
module_init(gec3399_buzzer_init);//入口函数
module_exit(gec3399_buzzer_exit);//出口函数
MODULE_AUTHOR("bobeyfeng@163.com");
MODULE_DESCRIPTION("buzzer driver for GEC3399");
MODULE_LICENSE("GPL"); //符合GPL 协议
MODULE_VERSION("V1.0");

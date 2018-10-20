/* include/linux/sensor-dev.h - sensor header file
 *
 * Copyright (C) 2012-2015 ROCKCHIP.
 * Author: luowei <lw@rock-chips.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/miscdevice.h>
#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif

#include <dt-bindings/sensor-dev.h>

#define SENSOR_ON		1
#define SENSOR_OFF		0
#define SENSOR_UNKNOW_DATA	-1

#define GPIO_HIGH 1
#define GPIO_LOW 0

enum sensor_id {
	ID_INVALID = 0,

	ANGLE_ID_ALL,
	ANGLE_ID_KXTIK,
	ANGLE_ID_LIS3DH,

	ACCEL_ID_ALL,
	ACCEL_ID_LIS331,
	ACCEL_ID_LSM303DLX,
	ACCEL_ID_LIS3DH,
	ACCEL_ID_KXSD9,
	ACCEL_ID_KXTF9,
	ACCEL_ID_KXTIK,
	ACCEL_ID_KXTJ9,
	ACCEL_ID_BMA150,
	ACCEL_ID_BMA222,
	ACCEL_ID_BMA250,
	ACCEL_ID_ADXL34X,
	ACCEL_ID_MMA8450,
	ACCEL_ID_MMA845X,
	ACCEL_ID_MMA7660,
	ACCEL_ID_MPU6050,
	ACCEL_ID_MXC6225,
	ACCEL_ID_DMARD10,
	ACCEL_ID_LSM303D,
	ACCEL_ID_MC3230,
	ACCEL_ID_MPU6880,
	ACCEL_ID_MPU6500,
	ACCEL_ID_LSM330,
	ACCEL_ID_BMA2XX,
	ACCEL_ID_STK8BAXX,
	COMPASS_ID_ALL,
	COMPASS_ID_AK8975,
	COMPASS_ID_AK8963,
	COMPASS_ID_AK09911,
	COMPASS_ID_AK8972,
	COMPASS_ID_AMI30X,
	COMPASS_ID_AMI306,
	COMPASS_ID_YAS529,
	COMPASS_ID_YAS530,
	COMPASS_ID_HMC5883,
	COMPASS_ID_LSM303DLH,
	COMPASS_ID_LSM303DLM,
	COMPASS_ID_MMC314X,
	COMPASS_ID_HSCDTD002B,
	COMPASS_ID_HSCDTD004A,

	GYRO_ID_ALL,
	GYRO_ID_L3G4200D,
	GYRO_ID_L3G20D,
	GYRO_ID_EWTSA,
	GYRO_ID_K3G,
	GYRO_ID_MPU6500,
	GYRO_ID_MPU6880,
	GYRO_ID_LSM330,
	LIGHT_ID_ALL,
	LIGHT_ID_CM3217,
	LIGHT_ID_CM3218,
	LIGHT_ID_CM3232,
	LIGHT_ID_AL3006,
	LIGHT_ID_STK3171,
	LIGHT_ID_ISL29023,
	LIGHT_ID_AP321XX,
	LIGHT_ID_PHOTORESISTOR,
	LIGHT_ID_US5152,
	LIGHT_ID_STK3410,

	PROXIMITY_ID_ALL,
	PROXIMITY_ID_AL3006,
	PROXIMITY_ID_STK3171,
	PROXIMITY_ID_AP321XX,
	PROXIMITY_ID_STK3410,

	TEMPERATURE_ID_ALL,
	TEMPERATURE_ID_MS5607,

	PRESSURE_ID_ALL,
	PRESSURE_ID_BMA085,
	PRESSURE_ID_MS5607,

	HALL_ID_ALL,
	HALL_ID_OCH165T,

	SENSOR_NUM_ID,
};

struct sensor_axis {
	int x;
	int y;
	int z;
};

/*
*sensor处在状态
*/
struct sensor_flag {
	atomic_t a_flag;	//加速度计
	atomic_t m_flag;	//陀螺仪
	atomic_t mv_flag;	//电磁传感器
	atomic_t open_flag;	//用来标记电子罗盘(陀螺仪)是否开启,=1开启
	atomic_t debug_flag;
	long long delay;	//用来给应用层设置延时工作的延时ms.
	wait_queue_head_t open_wq;	//这个目前在电子罗盘上使用,用来阻塞获取compass的 open_flag值.
};

//这个operate被每个具体sensor使用..可以说这个结构体出的数据都会被struct sensor_private_data、struct sensor_platform_data使用.
struct sensor_operate {
	char *name;			//芯片名称,由芯片驱动自身填写(如:struct sensor_operate compass_akm09911_ops .name字段)
	int type;			//sensor类型具体含义由dt-bindings/sensor-dev.h定义,,它在rk sensor core probe时用到.
	int id_i2c;			//在注册进sensor core时该字段会和本文件的 enum sensor_id做验证
	int range[2];		//上报值的范围
	int brightness[2];	//这个应该是lsensor用的亮度字段
	int read_reg;		//提供一个sensor器件内部寄存器,让系统读一下看能不能工作.
	int read_len;		//读取数据长度(byte)
	int id_reg;			//读CHIP_ID寄存器的地址
	int id_data;		//正确的CHIP_ID值,用于判断设备是否存在.
	int precision;		//数据的精度
	int ctrl_reg;		//sensor控制寄存器的地址,一般用于开启和关闭设备、初始化等.
	int ctrl_data;		//往控制寄存器写入的值
	int int_ctrl_reg;	//器件的中断控制寄存器地址,不需要填-1.
	int int_status_reg;	//中断状态寄存器,用于清中断,不需要填-1.
	int trig;			//中断触发方式,如:IRQF_TRIGGER_LOW|IRQF_ONESHOT,对于电平类中断请填IRQF_ONESHOT
	int (*active)(struct i2c_client *client, int enable, int rate);	// 开关sensor钩子
	int (*init)(struct i2c_client *client);	//sensor芯片钩子
	int (*report)(struct i2c_client *client);	//芯片读数据和上报数据钩子
	int (*suspend)(struct i2c_client *client);	//休眠操作接口
	int (*resume)(struct i2c_client *client);	//唤醒操作接口
	struct miscdevice *misc_dev;				//私用misc设备,sensor-dev.c驱动会检查该字段,如填写就不会替那个sensor注册misc dev
};

/* Platform data for the sensor  同种类型sensor private data (由sensor_probe 1954行知)*/
struct sensor_private_data {
	int type;						//sensor类型,struct sensor_platform_data *pdata;合进来.
	struct i2c_client *client;		//i2c客户端
	struct input_dev *input_dev;	//输入设备
	int stop_work;					//调度延时工作标记;=0且没开启硬件中断就允许调度(这里的延时工作类似于应用层的线程)
	struct delayed_work delaywork;	//延时工作
	struct sensor_axis axis;		//存放sensor 三维数值x,y,z
	char sensor_data[40];			//缓存sensor读到的数据
	atomic_t is_factory;			//校准标记,=1已经校准.注意了,没有校准是不能操作sensor的(及掉ioctl操作sensor时会阻塞).
	wait_queue_head_t is_factory_ok;//上面说的阻塞,即会阻塞在这个等待队列上.
	struct mutex data_mutex;		//获取sensor_data数据锁
	struct mutex operation_mutex;	//IOCTL操作互斥锁
	struct mutex sensor_mutex;		//操作sensor互斥锁
	struct mutex i2c_mutex;			//操作i2c互斥锁
	int status_cur;					//sensor开关状态
	int start_count;				//sensor开启次数
	int devid;						//读取到的CHIP_ID
	struct sensor_flag flags;		//sensor处在状态
	struct i2c_device_id *i2c_id;	//i2c_id table它由struct sensor_operate中的name、id_i2c合进来.
	struct sensor_platform_data *pdata;//板级sensor数据结构,等下详细分析.
	struct sensor_operate *ops;		//芯片驱动填充的一系列钩子、sensor相关信息.
	struct file_operations fops;	//misc设备操作方法.
	struct miscdevice miscdev;		//芯片驱动或者sensor_dev.c给注册的misc,一般用于给应用层提供ioctl接口.
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend early_suspend;//一级休眠
#endif
};

struct sensor_platform_data {
	int type;			//sensor类型,由具体芯片的设备树合进来.
	int irq;			//中断号,这个由sensor_probe-->sensor_irq_init-->gpio_to_irq得出
	int irq_pin;		//中断管脚,这个由设备树得出.
	int power_pin;		//sensor电源管理gpio
	int reset_pin;		//sensor复位gpio
	int standby_pin;	//sensor休眠gpio
	int irq_enable;		//=1:使用中断方式读sensor检测到的数据,=0使用delay work轮询
	int poll_delay_ms;	//从设备树上获取,默认30.它受应用层控制.
	int x_min;			//最小有效值,目前陀螺仪使用
	int y_min;			//最小有效值,目前陀螺仪使用
	int z_min;			//最小有效值,目前陀螺仪使用
	int factory;		//从设备树获取,但没被使用.
	int layout;			//当前器件默认那种布局
	unsigned char address;//i2c设备地址
	unsigned long irq_flags;//从设备树获取,中断管脚.
	signed char orientation[9];//sensor方向矩阵
	short m_layout[4][3][3];//指南针方向矩阵.但没被使用.
	int *project_name;		//从设备树获取,但没被使用.
	int power_off_in_suspend;//从设备树获取,休眠唤起后是否从新初始化sensor标志位.
};

/*
这个结构体是单独给:lis3dh.c、mma8452.c这两个驱动用的.
*/
struct gsensor_platform_data {
	u16 model;
	u16 swap_xy;
	u16 swap_xyz;
	signed char orientation[9];
	int (*get_pendown_state)(void);
	int (*init_platform_hw)(void);
	int (*gsensor_platform_sleep)(void);
	int (*gsensor_platform_wakeup)(void);
	void (*exit_platform_hw)(void);
};

struct akm8975_platform_data {
	short m_layout[4][3][3];
	char project_name[64];
	int gpio_DRDY;
};

struct akm_platform_data {
	short m_layout[4][3][3];
	char project_name[64];
	char layout;
	char outbit;
	int gpio_DRDY;
	int gpio_RST;
};

extern int sensor_register_slave(int type, struct i2c_client *client,
			struct sensor_platform_data *slave_pdata,
			struct sensor_operate *(*get_sensor_ops)(void));


extern int sensor_unregister_slave(int type, struct i2c_client *client,
			struct sensor_platform_data *slave_pdata,
			struct sensor_operate *(*get_sensor_ops)(void));

#define DBG(x...)

#define GSENSOR_IOCTL_MAGIC			'a'
#define GBUFF_SIZE				12	/* Rx buffer size */

/* IOCTLs for MMA8452 library */
#define GSENSOR_IOCTL_INIT						_IO(GSENSOR_IOCTL_MAGIC, 0x01)
#define GSENSOR_IOCTL_RESET					_IO(GSENSOR_IOCTL_MAGIC, 0x04)
#define GSENSOR_IOCTL_CLOSE					_IO(GSENSOR_IOCTL_MAGIC, 0x02)
#define GSENSOR_IOCTL_START					_IO(GSENSOR_IOCTL_MAGIC, 0x03)
#define GSENSOR_IOCTL_GETDATA					_IOR(GSENSOR_IOCTL_MAGIC, 0x08, char[GBUFF_SIZE+1])
#define GSENSOR_IOCTL_APP_SET_RATE			_IOW(GSENSOR_IOCTL_MAGIC, 0x10, short)
#define GSENSOR_IOCTL_GET_CALIBRATION		_IOR(GSENSOR_IOCTL_MAGIC, 0x11, int[3])


#define COMPASS_IOCTL_MAGIC					'c'
/* IOCTLs for APPs */
#define ECS_IOCTL_APP_SET_MODE				_IOW(COMPASS_IOCTL_MAGIC, 0x10, short)
#define ECS_IOCTL_APP_SET_MFLAG				_IOW(COMPASS_IOCTL_MAGIC, 0x11, short)
#define ECS_IOCTL_APP_GET_MFLAG				_IOW(COMPASS_IOCTL_MAGIC, 0x12, short)
#define ECS_IOCTL_APP_SET_AFLAG				_IOW(COMPASS_IOCTL_MAGIC, 0x13, short)
#define ECS_IOCTL_APP_GET_AFLAG				_IOR(COMPASS_IOCTL_MAGIC, 0x14, short)
#define ECS_IOCTL_APP_SET_TFLAG				_IOR(COMPASS_IOCTL_MAGIC, 0x15, short)/* NOT use */
#define ECS_IOCTL_APP_GET_TFLAG				_IOR(COMPASS_IOCTL_MAGIC, 0x16, short)/* NOT use */
#define ECS_IOCTL_APP_RESET_PEDOMETER		_IOW(COMPASS_IOCTL_MAGIC, 0x17)	/* NOT use */
#define ECS_IOCTL_APP_SET_DELAY				_IOW(COMPASS_IOCTL_MAGIC, 0x18, short)
#define ECS_IOCTL_APP_SET_MVFLAG				_IOW(COMPASS_IOCTL_MAGIC, 0x19, short)
#define ECS_IOCTL_APP_GET_MVFLAG				_IOR(COMPASS_IOCTL_MAGIC, 0x1A, short)
#define ECS_IOCTL_APP_GET_DELAY				_IOR(COMPASS_IOCTL_MAGIC, 0x1B, short)

#ifdef CONFIG_COMPAT
#define COMPAT_ECS_IOCTL_APP_SET_MODE			_IOW(COMPASS_IOCTL_MAGIC, 0x10, compat_short_t)
#define COMPAT_ECS_IOCTL_APP_SET_MFLAG			_IOW(COMPASS_IOCTL_MAGIC, 0x11, compat_short_t)
#define COMPAT_ECS_IOCTL_APP_GET_MFLAG			_IOW(COMPASS_IOCTL_MAGIC, 0x12, compat_short_t)
#define COMPAT_ECS_IOCTL_APP_SET_AFLAG			_IOW(COMPASS_IOCTL_MAGIC, 0x13, compat_short_t)
#define COMPAT_ECS_IOCTL_APP_GET_AFLAG			_IOR(COMPASS_IOCTL_MAGIC, 0x14, compat_short_t)
#define COMPAT_ECS_IOCTL_APP_SET_TFLAG			_IOR(COMPASS_IOCTL_MAGIC, 0x15, compat_short_t)/* NOT use */
#define COMPAT_ECS_IOCTL_APP_GET_TFLAG			_IOR(COMPASS_IOCTL_MAGIC, 0x16, compat_short_t)/* NOT use */
#define COMPAT_ECS_IOCTL_APP_RESET_PEDOMETER	_IOW(COMPASS_IOCTL_MAGIC, 0x17) /* NOT use */
#define COMPAT_ECS_IOCTL_APP_SET_DELAY			_IOW(COMPASS_IOCTL_MAGIC, 0x18, compat_short_t)
#define COMPAT_ECS_IOCTL_APP_SET_MVFLAG			_IOW(COMPASS_IOCTL_MAGIC, 0x19, compat_short_t)
#define COMPAT_ECS_IOCTL_APP_GET_MVFLAG			_IOR(COMPASS_IOCTL_MAGIC, 0x1A, compat_short_t)
#define COMPAT_ECS_IOCTL_APP_GET_DELAY			_IOR(COMPASS_IOCTL_MAGIC, 0x1B, compat_short_t)
#endif

#define LIGHTSENSOR_IOCTL_MAGIC					'l'
#define LIGHTSENSOR_IOCTL_GET_ENABLED			_IOR(LIGHTSENSOR_IOCTL_MAGIC, 1, int *)
#define LIGHTSENSOR_IOCTL_ENABLE					_IOW(LIGHTSENSOR_IOCTL_MAGIC, 2, int *)
#define LIGHTSENSOR_IOCTL_SET_RATE				_IOW(LIGHTSENSOR_IOCTL_MAGIC, 3, short)

#ifdef CONFIG_COMPAT
#define COMPAT_LIGHTSENSOR_IOCTL_GET_ENABLED	_IOR(LIGHTSENSOR_IOCTL_MAGIC, 1, compat_uptr_t)
#define COMPAT_LIGHTSENSOR_IOCTL_ENABLE			_IOW(LIGHTSENSOR_IOCTL_MAGIC, 2, compat_uptr_t)
#define COMPAT_LIGHTSENSOR_IOCTL_SET_RATE		_IOW(LIGHTSENSOR_IOCTL_MAGIC, 3, compat_short_t)
#endif

#define PSENSOR_IOCTL_MAGIC				'p'
#define PSENSOR_IOCTL_GET_ENABLED		_IOR(PSENSOR_IOCTL_MAGIC, 1, int *)
#define PSENSOR_IOCTL_ENABLE				_IOW(PSENSOR_IOCTL_MAGIC, 2, int *)
#define PSENSOR_IOCTL_DISABLE				_IOW(PSENSOR_IOCTL_MAGIC, 3, int *)

#ifdef CONFIG_COMPAT
#define COMPAT_PSENSOR_IOCTL_GET_ENABLED	_IOR(PSENSOR_IOCTL_MAGIC, 1, compat_uptr_t)
#define COMPAT_PSENSOR_IOCTL_ENABLE			_IOW(PSENSOR_IOCTL_MAGIC, 2, compat_uptr_t)
#define COMPAT_PSENSOR_IOCTL_DISABLE			_IOW(PSENSOR_IOCTL_MAGIC, 3, compat_uptr_t)
#endif

#define PRESSURE_IOCTL_MAGIC 				'r'
#define PRESSURE_IOCTL_GET_ENABLED		_IOR(PRESSURE_IOCTL_MAGIC, 1, int *)
#define PRESSURE_IOCTL_ENABLE				_IOW(PRESSURE_IOCTL_MAGIC, 2, int *)
#define PRESSURE_IOCTL_DISABLE			_IOW(PRESSURE_IOCTL_MAGIC, 3, int *)
#define PRESSURE_IOCTL_SET_DELAY			_IOW(PRESSURE_IOCTL_MAGIC, 4, int *)


#define TEMPERATURE_IOCTL_MAGIC			't'
#define TEMPERATURE_IOCTL_GET_ENABLED	_IOR(TEMPERATURE_IOCTL_MAGIC, 1, int *)
#define TEMPERATURE_IOCTL_ENABLE			_IOW(TEMPERATURE_IOCTL_MAGIC, 2, int *)
#define TEMPERATURE_IOCTL_DISABLE		_IOW(TEMPERATURE_IOCTL_MAGIC, 3, int *)
#define TEMPERATURE_IOCTL_SET_DELAY		_IOW(TEMPERATURE_IOCTL_MAGIC, 4, int *)


extern int sensor_rx_data(struct i2c_client *client, char *rxData, int length);
extern int sensor_tx_data(struct i2c_client *client, char *txData, int length);
extern int sensor_write_reg(struct i2c_client *client, int addr, int value);
extern int sensor_read_reg(struct i2c_client *client, int addr);
extern int sensor_tx_data_normal(struct i2c_client *client, char *buf, int num);
extern int sensor_rx_data_normal(struct i2c_client *client, char *buf, int num);
extern int sensor_write_reg_normal(struct i2c_client *client, char value);
extern int sensor_read_reg_normal(struct i2c_client *client);


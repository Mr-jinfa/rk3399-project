#ifndef COMPASS_APP_H
#define COMPASS_APP_H



#define SENSOR_IOCTL_MAGIC			'a'

//get & probe sensors
#define SENSOR_IOCTL_GET_MAP			_IOR(SENSOR_IOCTL_MAGIC, 0xff, int)
#define SENSOR_IOCTL_PROBE				_IOR(SENSOR_IOCTL_MAGIC, 0xfe, char[32])

#define COMPASS_IOCTL_MAGIC					'c'
/* IOCTLs for APPs */
#define ECS_IOCTL_APP_SET_MODE				_IOW(COMPASS_IOCTL_MAGIC, 0x10, short)
#define ECS_IOCTL_APP_SET_MFLAG				_IOW(COMPASS_IOCTL_MAGIC, 0x11, short)	//设置采集电磁传感器数据的延时函数的延时时间.
#define ECS_IOCTL_APP_GET_MFLAG				_IOW(COMPASS_IOCTL_MAGIC, 0x12, short)
#define ECS_IOCTL_APP_SET_AFLAG				_IOW(COMPASS_IOCTL_MAGIC, 0x13, short)	//设置采集加速度传感器数据的延时函数的延时时间.
#define ECS_IOCTL_APP_GET_AFLAG				_IOR(COMPASS_IOCTL_MAGIC, 0x14, short)
#define ECS_IOCTL_APP_SET_TFLAG				_IOR(COMPASS_IOCTL_MAGIC, 0x15, short)/* NOT use */
#define ECS_IOCTL_APP_GET_TFLAG				_IOR(COMPASS_IOCTL_MAGIC, 0x16, short)/* NOT use */
#define ECS_IOCTL_APP_RESET_PEDOMETER		_IOW(COMPASS_IOCTL_MAGIC, 0x17)	/* NOT use */
#define ECS_IOCTL_APP_SET_DELAY				_IOW(COMPASS_IOCTL_MAGIC, 0x18, short)	//开启延时函数
#define ECS_IOCTL_APP_SET_MVFLAG				_IOW(COMPASS_IOCTL_MAGIC, 0x19, short)	//设置采集陀螺仪传感器数据的延时函数的延时时间.
#define ECS_IOCTL_APP_GET_MVFLAG				_IOR(COMPASS_IOCTL_MAGIC, 0x1A, short)
#define ECS_IOCTL_APP_GET_DELAY				_IOR(COMPASS_IOCTL_MAGIC, 0x1B, short)
#define ECS_IOCTL_APP_ENABLE				_IO(COMPASS_IOCTL_MAGIC, 0x1C)	//Replace open func
#define ECS_IOCTL_APP_DISABLE				_IO(COMPASS_IOCTL_MAGIC, 0x1D)	//Replace release func

struct sensor_axis {
	int x;
	int y;
	int z;
};







#endif

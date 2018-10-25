/*
*本例程是测试rockchip sensor core的compass设备的测试代码.
*给出了操作compass的基本方法.其它类型的sensor如:gsensor类似(具体操作请参照sensor core of gsensor部分代码和gsensor类的驱动)
*方法介绍:
*1.compass_operate:打开/关闭具体compass_sensor 第一个装入的是compass_sensor0
*2.sensor_probe:选择sensor core of compass 的ioctl层中具体的compass(为什么要选择?因为这个ioctl层需要同时支持多个compass并发访问)
*3.ioctl cmd [arg]:sensor core of compass 给出的相应命令(其实是HAL层提出的命令,sensor core负责接收并回调具体sensor drv提供的钩子,具体sensor drv负责处理)
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <linux/input.h>

#include "compass_app.h"

struct sensor_axis axis;

struct input_event *axis_event;

int compass_operate(char *commpass_name,int *fd_compass, int index, int enable)
{
	sprintf(commpass_name, "/dev/compass_sensor%d", index);
	if(enable)
	{
		*fd_compass = open(commpass_name, O_RDWR);
		if(*fd_compass < 0)
			return -1;
	}
	if(enable == 0)	
		close(*fd_compass);
	return 0;
}

int sensor_probe(int fd, char *name)
{
	/*为sensor core of compass层选择合适的compass*/
	if(ioctl(fd, SENSOR_IOCTL_PROBE, name) <0)
	{
		printf("ioctl probe %s fail", name);
		return -1;
	}
}

int main(void)
{
	int ret, index=0;
	int fd_compass, axis_fd;
	char dev_addr[32];
	short rate = 200;	//delay work 200ms 底层设置最大200ms
	short flag=1;	//1:底层通过input dev上报数据
	char commpass_name[32] = {0};
	
	printf("which compass you want to operate?\n \
	input 0 is operate compass_sensor0\n");
	scanf("%d", &index);
	if(index >= 32)
	{
		printf("error orrwrite\n");
		return -0;
	}
	if(compass_operate(commpass_name, &fd_compass, index, 1) !=0)
	{
		printf("compass open fail\n");
		return -1;
	}
	
	sensor_probe(fd_compass, commpass_name+5);
	if(ioctl(fd_compass, ECS_IOCTL_APP_SET_MVFLAG, &flag) <0)
	{
		printf("[app] compass set flag fail\n");
	}
	
	sensor_probe(fd_compass, commpass_name+5);
	if(ioctl(fd_compass, ECS_IOCTL_APP_SET_DELAY, &rate) <0)
	{
		printf("[app] compass set rate fail\n");
	}

	sensor_probe(fd_compass, commpass_name+5);
	if(ioctl(fd_compass, ECS_IOCTL_APP_ENABLE, NULL) <0)
	{
		printf("[app] compass set enable fail\n");
	}
	
	axis_event = (struct input_event *)calloc(sizeof(struct input_event), 1);
	axis_fd = open("/dev/input/event3", O_RDWR);
	if (axis_fd < 0)
	{
		perror("can't open compass event!");
		return -1;
	}
	axis_event = (struct input_event *)calloc(sizeof(struct input_event), 1);

	while(1)
	{
		ret = read(axis_fd, axis_event,sizeof(axis));
		switch(axis_event->type)
		{
			case EV_ABS:
				if(axis_event->code == ABS_X){
					printf("X:%d  ", axis_event->value);
				}
				else if(axis_event->code == ABS_Y){
					printf("Y:%d  ", axis_event->value);
				}
				else if(axis_event->code == ABS_Z){
					printf("Z:%d  ", axis_event->value);
				}
			break;
			case EV_SYN:
				break;
			default:
				printf("error event\n");
				break;
		}
		sleep(1);
	}
	sensor_probe(fd_compass, commpass_name+5);
	if(ioctl(fd_compass, ECS_IOCTL_APP_DISABLE, NULL) <0)
	{
		printf("[app] compass set enable fail\n");
	}
	if(compass_operate(commpass_name, &fd_compass, index, 0) !=0)
	{
		printf("compass release fail\n");
		return -1;
	}
	return 0;
}

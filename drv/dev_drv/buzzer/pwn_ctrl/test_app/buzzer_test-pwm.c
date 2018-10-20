#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define PWM1_BUZZER_ENABLE _IO('P',0)
#define PWM1_BUZZER_CONFIG _IO('P',1)
#define PWM1_BUZZER_DISABLE _IO('P',2)
struct pwm_cfg
{
	int duty_ns;
	int period_ns;
};

int main(void)
{
	int fd_buzzer;
	int ret;
	struct pwm_cfg pwm1;
	pwm1.duty_ns = 0;
	pwm1.period_ns = 10000;
	fd_buzzer = open("/dev/buzzer_drv", O_WRONLY);
	if(fd_buzzer < 0)
	{
		perror("open buzzer driver");
		return -1;
	}
	ret = ioctl(fd_buzzer,PWM1_BUZZER_CONFIG,&pwm1);
	if(ret < 0)
		perror("ioctl PWM1_BUZZER_CONFIG error\n");
		ret = ioctl(fd_buzzer,PWM1_BUZZER_ENABLE,&pwm1);
	if(ret < 0 )
		perror("ioctl PWM1_BUZZER_ENABLE error");
	while(1)
	{
		sleep(1);
		pwm1.duty_ns = 0;
		ioctl(fd_buzzer,PWM1_BUZZER_CONFIG,&pwm1);
		sleep(1);
		pwm1.duty_ns = 10000;
		ioctl(fd_buzzer,PWM1_BUZZER_CONFIG,&pwm1);
	}
	close(fd_buzzer);
	return 0;
}
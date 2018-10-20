#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#define BUZZER_START _IO('B',0)
#define BUZZER_STOP _IO('B',1)
int main(void)
{
	int fd_buzzer;
	int ret;
	fd_buzzer = open("/dev/buzzer_drv", O_WRONLY);
	if(fd_buzzer < 0)
	{
		perror("open buzzer driver");
		return -1;
	}
	while(1)
	{
		ret = ioctl(fd_buzzer,BUZZER_START);
		if(ret < 0)
			perror("write buzzer driver ");
		
		printf("BUZZER_START\n");
		sleep(1);
		ret = ioctl(fd_buzzer,BUZZER_STOP);
		if(ret < 0 )
			perror("write buzzer driver ");

		printf("BUZZER_STOP\n");
		sleep(1);
	}
		close(fd_buzzer);
		return 0;
}

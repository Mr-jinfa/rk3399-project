#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
int val;
int main(void)
{
	int fd_adc;
	int ret;
	fd_adc = open("/dev/adc_drv", O_RDWR);
	if(fd_adc < 0)
	{
		perror("open adc driver");
		return -1;
	}
	while(1)
	{
		sleep(1);
		ret = read(fd_adc,&val,4);
		if(ret < 0)
		{
			perror("read adc driver\n");
			return -1;
		}
		printf("val = %d\n",val);
	}
	close(fd_adc);
	return 0;
}

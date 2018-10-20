ifneq ($(KERNELRELEASE),)
obj-m:=adc_drv.o
else
#generate the path
CURRENT_PATH:=$(shell pwd)
#the absolute path
LINUX_KERNEL_PATH:=~/rk3399_work/kernel
#complie object
default:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules
clean:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) clean
endif
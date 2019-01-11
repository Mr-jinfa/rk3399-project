obj-m += ramblock_drv.o
LINUX_KERNEL_PATH:=~/rk3399_work/kernel
CURRENT_PATH:=$(shell pwd)

default:
	$(MAKE)  -C $(LINUX_KERNEL_PATH)  M=$(CURRENT_PATH) modules 

clean:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) clean

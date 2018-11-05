# rk3399-project
基于瑞芯微rk3399源码包适配到gec3399板卡中

#修改工程包
  1.添加最小内核(kernel)
修改~/rk3399_work/device/rockchip/rk3399BoardConfig.mk
1.1将RK_KERNEL_DEFCONFIG修改为rockchip_minlinux_defconfig
修改kernel/scripts/kconfig/Makefile加一个路径$(RK_TARGET_PRODUCT)
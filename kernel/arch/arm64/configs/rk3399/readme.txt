分多个不同板卡的内核配置的目的:
上层选择不同的内核配置就可以适配不同板卡。

rockchip_linux_defconfig：
是原厂提过来的rk3399默认内核配置

rockchip_linux_minconfig：
是经rockchip_linux_defconfig裁剪后的能保证gec3399平台运行Linux、ethernet、lcd(基本功能)的内核配置。

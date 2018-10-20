/*
 * drivers/char/dma_test/sunxi_dma_test.c
 *
 * Copyright(c) 2013-2015 Allwinnertech Co., Ltd.
 *      http://www.allwinnertech.com
 *
 * Author: liugang <liugang@allwinnertech.com>
 *
 * sunxi dma test driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 *log:
 *2018.10.11：本例程由sunxi平台移植过来
 */

#include "rk_dma_mem2mem.h"

/* wait queue for waiting dma done */
wait_queue_head_t g_dtc_queue[DTC_MAX];
atomic_t g_adma_done = ATOMIC_INIT(0); /* dma done flag */
//int size_available[] = {SZ_4K, SZ_64K, SZ_256K, SZ_512K, SZ_512K + SZ_64K, SZ_512K + SZ_256K, SZ_1M};
int size_available[] = {SZ_4K, SZ_16K, SZ_32K, SZ_64K, SZ_128K, SZ_256K, SZ_512K};


static void __dma_test_init_waitqueue(void)
{
	u32 i = 0;

	for(i = 0; i < DTC_MAX; i++)
		init_waitqueue_head(&g_dtc_queue[i]);
}

static void __dma_callback(void *dma_async_param)
{
	chan_info *pinfo = (chan_info *)dma_async_param;

	wake_up_interruptible(&pinfo->dma_wq);
	atomic_set(&pinfo->dma_done, 1);
}

buf_group *init_buf(void)
{
//建一个表项
	buf_group *pbuf = kmalloc(sizeof(buf_group), GFP_KERNEL);
	int i, buf_cnt = (get_random_int()%BUF_MAX_CNT) + 1;	//调用get_random_int目的是使buf_cnt是一个随机数(表明测试是随机的)
	int size, index;

	if (!pbuf)
		return NULL;

	for (i = 0; i < buf_cnt; i++) {
		index = get_random_int() % ARRAY_SIZE(size_available);
		size = size_available[index];
printk("%s(%d): buf %d, index %d, size 0x%x\n", __func__, __LINE__, i, index, size);
		pbuf->item[i].src_va = kmalloc(size, GFP_KERNEL);
		if (!pbuf->item[i].src_va){
			printk("dma_alloc src_va coherent error\n");
			break;
		}
		pbuf->item[i].src_pa = (unsigned int *)virt_to_phys(pbuf->item[i].src_va);
	
		pbuf->item[i].dst_va = kmalloc(size, GFP_KERNEL);
		if (!pbuf->item[i].dst_va) {
		printk("dma_alloc dst_va coherent error\n");
			kfree(pbuf->item[i].src_va);
			break;
		}
		pbuf->item[i].dst_pa = (unsigned int *)virt_to_phys(pbuf->item[i].dst_va);
		/* 初始化刚才分配出来的值 */
		memset(pbuf->item[i].src_va, 0x54, size);
		memset(pbuf->item[i].dst_va, 0xab, size);
		pbuf->item[i].size = size;
	}

	pbuf->cnt = i;
	if(0 == pbuf->cnt)
		return NULL;

printk("%s(%d): buf cnt %d, buffers:\n", __func__, __LINE__, pbuf->cnt);
for (i = 0; i < pbuf->cnt; i++)
printk("        src: va 0x%p, pa 0x%p; dst: va 0x%p, pa 0x%p\n",
	pbuf->item[i].src_va, pbuf->item[i].src_pa, pbuf->item[i].dst_va, pbuf->item[i].dst_pa);

	return pbuf;
}

void deinit_buf(buf_group *pbuf)
{
	int i;

	if (!pbuf)
		return;

	for (i = 0; i < pbuf->cnt; i++) {
		kfree(pbuf->item[i].src_va);
		kfree(pbuf->item[i].dst_va);
	}
	kfree(pbuf);
}

int check_result(buf_group *pbuf)
{
	int i, j;

	if (!pbuf)
		return -EINVAL;

	for (i = 0; i < pbuf->cnt; i++) {
		if(memcmp(pbuf->item[i].src_va, pbuf->item[i].dst_va, pbuf->item[i].size)) {
			printk("%s(%d) err: buffer %d memcmp failed!\n", __func__, __LINE__, i);

			printk("   src buffer: ");
			for (j = 0; j < 16; j++)
				printk("%d ", *(pbuf->item[i].src_va + j));
			printk("\n");
			printk("   dst buffer: ");
			for (j = 0; j < 16; j++)
				printk("%d ", *(pbuf->item[i].dst_va + j));
			printk("\n");
			return -EIO;
		}
	}
	return 0;
}

int case_memcpy_single_chan(void)
{
	struct sg_table src_sg_table, dst_sg_table;
	struct dma_async_tx_descriptor *tx = NULL;
	struct dma_slave_config config;
	struct dma_chan *chan;
	struct scatterlist *sg;
	buf_group *buffers = NULL;
	long timeout = 5 * HZ;
	chan_info dma_info;
	dma_cap_mask_t mask;
	dma_cookie_t cookie;
	int i, ret = -EINVAL;

	buffers = init_buf();
	if (!buffers) {
		pr_err("%s(%d) err: init_buf failed!\n", __func__, __LINE__);
		return -EBUSY;
	}

	/* 初始化mask-通道类型掩码 */
	dma_cap_zero(mask);
	dma_cap_set(DMA_SG, mask);
	dma_cap_set(DMA_MEMCPY, mask);
	/* 根据mask申请合适通道 
	第二个参数:dma_chan_filter这个函数主要是查找你的dma传输的设备的请求信号线，其具体是在注册时填写的。
	这里会根据这个函数返回的真假来判断已经注册在总线上的dma slave的。
	如果写dev2mem/ mem2dev/ dev2dev/ cleint驱动的话就需要dma_chan_filter钩子*/
	chan = dma_request_channel(mask , NULL , NULL);
	if (!chan) {
		pr_err("%s(%d) err: dma_request_channel failed!\n", __func__, __LINE__);
		goto out1;
	}
/* 申请sg(scatter gather ) 表*/
	if (sg_alloc_table(&src_sg_table, buffers->cnt, GFP_KERNEL)) {
		pr_err("%s(%d) err: alloc src sg_table failed!\n", __func__, __LINE__);
		goto out2;
	}
	if (sg_alloc_table(&dst_sg_table, buffers->cnt, GFP_KERNEL)) {
		pr_err("%s(%d) err: alloc dst sg_table failed!\n", __func__, __LINE__);
		goto out3;
	}

	/* assign sg buf 用刚才分配的表来指向(管理)刚分配出来的buffers*/
	sg = src_sg_table.sgl;
	for (i = 0; i < buffers->cnt; i++, sg = sg_next(sg)) {
		sg_set_buf(sg, phys_to_virt((unsigned long)buffers->item[i].src_pa), buffers->item[i].size);
		sg_dma_address(sg) = (unsigned long)buffers->item[i].src_pa;
	}
	sg = dst_sg_table.sgl;
	for (i = 0; i < buffers->cnt; i++, sg = sg_next(sg)) {
		sg_set_buf(sg, phys_to_virt((unsigned long)buffers->item[i].dst_pa), buffers->item[i].size);
		sg_dma_address(sg) = (unsigned long)buffers->item[i].dst_pa;
	}

/* 配置设备(内存也属于设备)的配置信息 */
	config.direction = DMA_MEM_TO_MEM;
	config.src_addr = 0; /* not used for memcpy */
	config.dst_addr = 0;
	config.src_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
	config.dst_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
	config.src_maxburst = 8;
	config.dst_maxburst = 8;
//	config.slave_id = rk_slave_id(DRQDST_SDRAM, DRQSRC_SDRAM);

	dmaengine_slave_config(chan , &config);

/* 传入通道的配置信息、sg表、dma控制位信息。返回传输描述符 */
	tx = chan->device->device_prep_dma_sg(chan, dst_sg_table.sgl, buffers->cnt,
		src_sg_table.sgl, buffers->cnt, DMA_PREP_INTERRUPT | DMA_CTRL_ACK);

	/* set callback */
	dma_info.chan = chan;
	init_waitqueue_head(&dma_info.dma_wq);
	atomic_set(&dma_info.dma_done, 0);
	tx->callback = __dma_callback;	//当传输完成时dma controller driver会调用
	tx->callback_param = &dma_info;

	/* enqueue */
	cookie = dmaengine_submit(tx);
	/* start dma */
	dma_async_issue_pending(chan);

	/* wait transfer over */
	ret = wait_event_interruptible_timeout(dma_info.dma_wq, atomic_read(&dma_info.dma_done)==1, timeout);
	if (unlikely(-ERESTARTSYS == ret || 0 == ret)) {
		pr_err("%s(%d) err: wait dma done failed!\n", __func__, __LINE__);
		goto out4;
	}

	ret = check_result(buffers);
out4:
	sg_free_table(&src_sg_table);
out3:
	sg_free_table(&dst_sg_table);
out2:
	dma_release_channel(chan);
out1:
	if (buffers)
		deinit_buf(buffers);
	if(ret)
		printk("%s(%d) err: test failed!\n", __func__, __LINE__);
	else
		printk("%s(%d): test success!\n", __func__, __LINE__);
	return ret;
}


static int dma_test_main(int id)
{
	u32 uret = 0;

	switch(id) {
	case DTC_MEMCPY_SINGLE_CHAN:
		uret = case_memcpy_single_chan();
		break;
	default:
		uret = __LINE__;
		break;
	}

	if(0 == uret)
		printk("%s: test success!\n", __func__);
	else
		printk("%s: test failed!\n", __func__);
	return uret;
}

const char *case_name[] = {
	"DTC_MEMCPY_SINGLE_CHAN",
};

ssize_t test_store(struct class *class, struct class_attribute *attr,
			const char *buf, size_t size)
{
	int id = 0;

	if(kstrtoint(buf, 10, (int *)&id)) {
		pr_err("%s: invalid string %s\n", __func__, buf);
		return -EINVAL;
	}
	pr_info("%s: string %s, test case %s\n", __func__, buf, case_name[id]);

	if(0 != dma_test_main(id))
		pr_err("%s: dma_test_main failed! id %d\n", __func__, id);
	else
		pr_info("%s: dma_test_main success! id %d\n", __func__, id);
	return size;
}

ssize_t help_show(struct class *class, struct class_attribute *attr, char *buf)
{
	ssize_t cnt = 0;

	cnt += sprintf(buf + cnt, "usage: echo id > test\n");
	cnt += sprintf(buf + cnt, "     id for case DTC_MEMCPY_SINGLE_CHAN        is %d\n", (int)DTC_MEMCPY_SINGLE_CHAN);
	cnt += sprintf(buf + cnt, "case description:\n");
	cnt += sprintf(buf + cnt, "     DTC_MEMCPY_SINGLE_CHAN:        case for single channel\n");
	return cnt;
}

static struct class_attribute dma_test_class_attrs[] = {
	__ATTR(test, 0220, NULL, test_store), /* not 222, for CTS, other group cannot have write permission */
	__ATTR(help, 0444, help_show, NULL),
	__ATTR_NULL,
};

static struct class dma_test_class = {
	.name		= "sunxi_dma_test",
	.owner		= THIS_MODULE,
	.class_attrs	= dma_test_class_attrs,
};

static int __init sw_dma_test_init(void)
{
	int status;

	pr_info("%s enter\n", __func__);

	/* init dma wait queue */
	__dma_test_init_waitqueue();

	status = class_register(&dma_test_class);
	if(status < 0)
		pr_info("%s err, status %d\n", __func__, status);
	else
		pr_info("%s success\n", __func__);
	return 0;
}

static void __exit sw_dma_test_exit(void)
{
	pr_info("sw_dma_test_exit: enter\n");
	class_unregister(&dma_test_class);
}

module_init(sw_dma_test_init);
module_exit(sw_dma_test_exit);
MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("JinFa");
MODULE_DESCRIPTION ("rockchip dma mem2mem driver");

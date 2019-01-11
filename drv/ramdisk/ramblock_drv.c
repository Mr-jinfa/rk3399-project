#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/timer.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/hdreg.h>
#include <linux/kdev_t.h>
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/buffer_head.h>
#include <linux/bio.h>


#define SBULL_MAJOR    0
#define SBULL_HARDSECT    512       //虚拟磁盘扇区大小

#define KERNEL_SECTOR_SIZE    512   //内核扇区大小


static int sbull_major = SBULL_MAJOR;   //保存虚拟磁盘主设备号
static int hardsect_size = SBULL_HARDSECT;  //虚拟磁盘每个扇区的大小
static int nsectors = 1024;                  //虚拟磁盘扇区个数
module_param(sbull_major, int, 0);
module_param(hardsect_size, int, 0);
module_param(nsectors, int, 0);


/*该结构体包含了我们这个虚拟磁盘的所有变量和信息*/
struct sbull_dev {
    int size;
    u8 *data;
    short users;
    spinlock_t lock;
    struct request_queue *queue;
    struct gendisk *gd;
};
static struct sbull_dev *device = NULL;

/*
*获得驱动器信息
*/
static int ramblock_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{
	geo->heads     = 2;
	geo->cylinders = nsectors/2;
	geo->sectors   = nsectors;
	return 0;
}



static struct block_device_operations ramblock_fops = {
	.owner	= THIS_MODULE,
	.getgeo	= ramblock_getgeo,
};

/*
*功能:实现对若干个扇区(扇区堆)操作
*参数:
*dev:虚拟磁盘
*sector:需要处理扇区首地址
*nsect:剩余需要处理的扇区个数
*buffer：一个扇区的缓存区
*write:1为写操作，0为读操作
*/
static void sbull_transfer(struct sbull_dev *dev, unsigned long sector, unsigned long nsect, char *buffer, int write)
{
	unsigned long offset = sector*KERNEL_SECTOR_SIZE;   // 从磁头开始到需要操作的扇区头的偏移
    unsigned long nbytes = nsect*KERNEL_SECTOR_SIZE;    //需要操作的字节数：扇区大小*扇区数目

	static int r_cnt = 0;
	static int w_cnt = 0;	
	
    if ((offset+nbytes) > dev->size)    //如果在当前磁头基础上操作的字节对于磁盘有溢出
    {
		printk("Beyond-end write (%ld %ld)\n", offset, nbytes);
        return;
    }

    if (write)
    {
		printk("do_ramblock_request write %d\n", ++w_cnt);
        memcpy(dev->data+offset, buffer, nbytes);
    }
    else
    {
		printk("do_ramblock_request read  %d\n", ++r_cnt);
        memcpy(buffer, dev->data+offset, nbytes);
    }

}

/*
*处理一个bio请求
*/
static int sbull_xfer_bio(struct sbull_dev *dev, struct bio *bio)
{
    struct bio_vec bvec;  //用来描述与这个bio请求对应的所有的内存
	struct bvec_iter iter;  //描述一个扇区（片段）
	
	
    sector_t sector = bio->bi_iter.bi_sector;  //获取第一个处理扇区（片段）的首地址 
	//通过bio内存的起始地址循环获取需要处理的bio中扇区（片段）信息
	bio_for_each_segment(bvec, bio, iter)
    {
		char *buffer = __bio_kmap_atomic(bio, iter); //获得一个扇区的缓存区
	
		/*（扇区）片段处理函数*：
		 * sector：这次扇区的起始地址
		   bio_cur_bytes(bio)>>9 ：剩余需要处理的扇区个数
		   buffer：缓存区域
		   bio_data_dir(bio) == WRITE ：该片段读或者写
		*/
		sbull_transfer(dev, sector, bio_cur_bytes(bio)>>9, buffer, bio_data_dir(bio) == WRITE);
		
		sector += bio_cur_bytes(bio)>>9;//更新需要处理扇区首地址
		__bio_kunmap_atomic(buffer);	//释放该缓存区
    }
    return 0;
}

/*
*处理一个请求
*/
static int sbull_xfer_request(struct sbull_dev *dev, struct request *req)
{
    struct bio *bio;  //每个请求中含有多个bio需要处理  
    int nsect = 0;    //记录处理bio的个数
	/*循环处理bio*/
    __rq_for_each_bio(bio, req) {
        sbull_xfer_bio(dev, bio);
        //更新bio下标
        nsect += bio->bi_iter.bi_size/KERNEL_SECTOR_SIZE;
    }
    return nsect;
}

/*请求处理函数*/
static void sbull_full_request(struct request_queue *q)
{
    struct request *req;
    int sectors_xferred;
    struct sbull_dev *dev = device;

	/*循环从请求队列中取出请求*/
    while((req = blk_fetch_request(q)) != NULL)
    {
		/*先判断这个请求是否为fs请求*/
        if (req->cmd_type != REQ_TYPE_FS)
        {//报告请求已出错完成
            __blk_end_request_all(req, -EIO);
            continue;
        }	
		/*对这个请求进行处理*/
        sectors_xferred = sbull_xfer_request(dev, req);
		/*提交这个处理已经正确完成*/
        __blk_end_request_all(req, 0);
    }
}

static void ramdisk_device_init(struct sbull_dev *dev, int major)
{
	/*该虚拟磁盘的大小（字节数）：扇区 * 每个扇区的字节数*/
	dev->size = nsectors*hardsect_size;
	/*分配我们虚拟磁盘的空间*/
	dev->data = vmalloc(dev->size);
	if (dev->data == NULL){
		printk("%s %d  fail\n",__func__,__LINE__);
        return;
    }

	spin_lock_init(&dev->lock);  
	dev->queue = blk_init_queue(sbull_full_request, &dev->lock);
    if (dev->queue == NULL){
		printk("%s %d  fail\n",__func__,__LINE__);
        goto out_vfree;
    }

	/*分配一个分区*/
	dev->gd = alloc_disk(1);
    if (!dev->gd) {
        goto out_vfree;
    }

	dev->gd->major = major;
	/*设置次设备号为从零开始*/
	dev->gd->first_minor = 0;
	/*把队列放入磁盘设备中*/
	dev->gd->queue = dev->queue;
	sprintf(dev->gd->disk_name, "ramblock");
	dev->gd->fops = &ramblock_fops;
	/*设置虚拟磁盘的扇区总数：即有多少个扇区，内核默认扇区大小为KERNEL_SECTOR_SIZE（默认）*/
	set_capacity(dev->gd, nsectors*(hardsect_size/KERNEL_SECTOR_SIZE));

	/*生成设备文件dev目录下*/
	add_disk(dev->gd);
	return;
	
out_vfree:
	if (dev->data)
		vfree(dev->data);
}



static int __init sbull_init(void)
{
	/*获得一个主设备号，能使用cat /proc/devices查看到设备属性*/
	sbull_major = register_blkdev(sbull_major, "sbull");  
    if (sbull_major <= 0) {
        return -EBUSY;
	}

	/*分配一个struct sbull_dev类型结构体*/
	device = kmalloc(sizeof(struct sbull_dev), GFP_KERNEL);
    if (device == NULL) {
        goto out_unregister;
	}

	memset(device, 0, sizeof(struct sbull_dev));

	/*对分配的device进行初始化*/
	ramdisk_device_init(device, sbull_major);

    return 0;

out_unregister:
	unregister_blkdev(sbull_major, "sbull");
	return -ENOMEM;
}

static void sbull_exit(void)
{
	struct sbull_dev *dev = device;

    if (dev->gd)
    {
        del_gendisk(dev->gd);
        put_disk(dev->gd);
    }

    if (dev->queue)
    {
        blk_cleanup_queue(dev->queue);
    }

    if (dev->data)
    {
        vfree(dev->data);
    }

    unregister_blkdev(sbull_major, "sbull");
    kfree(device);
}

module_init(sbull_init);
module_exit(sbull_exit);
MODULE_LICENSE("GPL");
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>

#define MEMORY_SIZE 1024
#define CHARDEV_MAJOR 101

static int chardev_major = CHARDEV_MAJOR;
module_param(chardev_major,int,S_IRUGO);

struct chardev_dev{
    struct cdev cdev;
    unsigned char mem[MEMORY_SIZE];
};

struct chardev_dev *devp;

static ssize_t chardev_read(struct file *fp,char __user *buf,size_t size, loff_t *ppos){
    unsigned long pos = *ppos;
    unsigned int readcount = size;
    int ret = 0;
    struct chardev_dev *dev = fp->private_data;

    if(pos >= MEMORY_SIZE) return 0;
    if(readcount > MEMORY_SIZE - pos) readcount = MEMORY_SIZE - pos;

    if(copy_to_user(buf,dev->mem+pos,readcount)){
        ret = -EFAULT;
    }else{
        *ppos += readcount;
        ret = readcount;
        printk(KERN_INFO "read %u byte(s) from %lu\n",readcount,pos);
    }
    return ret;
}

static ssize_t chardev_write(struct file *fp,const char __user *buf,size_t size,loff_t *ppos){
    unsigned long pos = *ppos;
    unsigned int writecount = size;
    int ret = 0;
    struct chardev_dev *dev = fp->private_data;

    if(pos >= MEMORY_SIZE) return 0;
    if(writecount > MEMORY_SIZE - pos) writecount = MEMORY_SIZE - pos;

    if(copy_from_user(dev->mem+pos,buf,writecount)){
        ret = -EFAULT;
    }else{
        *ppos += writecount;
        ret = writecount;
        printk(KERN_INFO "read %u byte(s) from %lu\n",writecount,pos);
    }
    return ret;
}

static loff_t chardev_llseek(struct file *fp,loff_t offset,int orig){
    loff_t ret = 0;
    switch(orig){
        case 0:
            if(offset < 0){
                ret = -EINVAL;
                break;
            }
            if((unsigned int)offset > MEMORY_SIZE){
                ret = -EINVAL;
                break;
            }
            fp->f_pos = (unsigned int)offset;
            ret = fp->f_pos;
            break;
        case 1:
            if((fp->f_pos + offset) > MEMORY_SIZE){
                ret = -EINVAL;
                break;
            }
            if((fp->f_pos + offset) < 0){
                ret = -EINVAL;
                break;
            }
            fp->f_pos = (unsigned int)(fp->f_pos + offset);
            ret = fp->f_pos;
        break;
        default:
            ret = -EINVAL;
            break;
    }
    return ret;
}

static long chardev_ioctl(struct file *fp,unsigned int cmd,unsigned long arg){
    struct chardev_dev *dev = fp->private_data;
    switch(cmd){
        case 1 /*MEM_CLEAR*/:
            memset(dev->mem,0,MEMORY_SIZE);
            printk(KERN_INFO "chardev is set to zero\n");
            break;
        default:
            return -EINVAL;
    }
    return 0;
}

static int chardev_open(struct inode *node,struct file *fp){
    fp->private_data = devp;
    return 0;
}

static int chardev_release(struct inode *inode,struct file *fp){
    return 0;
}

static const struct file_operations chardev_fops = {
    .owner = THIS_MODULE,
    .llseek = chardev_llseek,
    .read = chardev_read,
    .write = chardev_write,
    .unlocked_ioctl = chardev_ioctl,
    .open = chardev_open,
    .release = chardev_release
};

static void chardev_setup_cdev(struct chardev_dev *dev, int index){
    int err,devno = MKDEV(chardev_major,index);

    cdev_init(&dev->cdev,&chardev_fops);
    dev->cdev.owner = THIS_MODULE;
    err = cdev_add(&dev->cdev,devno,1);
    if(err){
        printk(KERN_NOTICE "Error %d adding chardev %d\n",err,index);
    }
}

static int __init chardev_init(void){
    int ret;
    dev_t devno = MKDEV(chardev_major,0);


    if(chardev_major){
        ret = register_chrdev_region(devno,1,"chardev");
    }else{
        ret = alloc_chrdev_region(&devno,0,1,"chardev");
        chardev_major = MAJOR(devno);
    }

    if(ret < 0){
        return ret;
    }
    devp = kzalloc(sizeof(struct chardev_dev),GFP_KERNEL);
    if(!devp){
        ret = -ENOMEM;
        unregister_chrdev_region(devno,1);
        return ret;
    }

    chardev_setup_cdev(devp,0);
    return 0;
}

static void __exit chardev_exit(void){
    cdev_del(&devp->cdev);
    kfree(devp);
    unregister_chrdev_region(MKDEV(chardev_major,0),1);
}
module_init(chardev_init);
module_exit(chardev_exit);

MODULE_AUTHOR("Jiamu Sun<x739566858@outlook.com>");
MODULE_LICENSE("GPL v2");
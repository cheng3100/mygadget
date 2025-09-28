# 字符设备驱动全流程——打造用户空间可控的驱动接口

**Author:** dump linux

**Date:** 2025-07-03

**Link:** https://zhuanlan.zhihu.com/p/1922940958858778466

在 Linux 驱动世界中，有一种最常见的“用户可交互”接口，那就是 **[字符设备](https://zhida.zhihu.com/search?content_id=259710552&content_type=Article&match_order=1&q=%E5%AD%97%E7%AC%A6%E8%AE%BE%E5%A4%87&zhida_source=entity)**。

你可能熟悉这些：

```bash
/dev/ttyS0
/dev/i2c-0
/dev/input/event1
/dev/gpiochip0
```

它们的共同点是：

✅ 都是以 `/dev/xxx` 形式存在 ✅ 支持 `open()`、`read()`、`write()`、`ioctl()` 等标准调用 ✅ 属于 **[字符设备驱动](https://zhida.zhihu.com/search?content_id=259710552&content_type=Article&match_order=1&q=%E5%AD%97%E7%AC%A6%E8%AE%BE%E5%A4%87%E9%A9%B1%E5%8A%A8&zhida_source=entity)（char device driver）**

今天我们就来手把手写一个**完整的字符设备驱动**，实现从内核态 → 用户态的交互通路。

## **一、什么是字符设备？**

字符设备是一种**以字节流方式读写的设备**，与块设备（如硬盘）相比，其数据没有固定结构。

### **驱动中需要完成的事情**

| 步骤 | 说明 |
| --- | --- |
| 分配设备号 | 让设备有合法的主/次编号（major/minor） |
| 初始化 cdev | 实际驱动数据结构，与操作函数绑定 |
| 实现 file_operations 接口 | 定义 open/read/write 等函数 |
| 注册到 /dev | 自动创建设备节点，用户态可访问 |

## **二、编写字符设备驱动的核心代码结构**

我们来写一个叫做 `/dev/hello_chrdev` 的驱动，写入什么，就回显什么（echo 驱动）。

### **1\. 驱动头文件和结构**

```c
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
​
#define DEVICE_NAME "hello_chrdev"
static dev_t dev_num;
static struct cdev hello_cdev;
static struct class *hello_class;
static char kernel_buffer[128];
```

### **2\. 实现 `file_operations` 接口**

```c
static int hello_open(struct inode *inode, struct file *file)
{
    pr_info("hello_chrdev: device opened\n");
    return 0;
}
​
static ssize_t hello_read(struct file *file, char __user *buf, size_t len, loff_t *offset)
{
    return simple_read_from_buffer(buf, len, offset, kernel_buffer, strlen(kernel_buffer));
}
​
static ssize_t hello_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
    size_t to_copy = min(len, sizeof(kernel_buffer) - 1);
    if (copy_from_user(kernel_buffer, buf, to_copy))
        return -EFAULT;
    kernel_buffer[to_copy] = '\0';
    pr_info("hello_chrdev: received \"%s\"\n", kernel_buffer);
    return to_copy;
}
```

### **3\. 定义操作结构体并注册字符设备**

```c
static const struct file_operations hello_fops = {
    .owner = THIS_MODULE,
    .open = hello_open,
    .read = hello_read,
    .write = hello_write,
};
​
static int __init hello_init(void)
{
    int ret;
​
    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) return ret;
​
    cdev_init(&hello_cdev, &hello_fops);
    ret = cdev_add(&hello_cdev, dev_num, 1);
    if (ret < 0) return ret;
​
    hello_class = class_create(THIS_MODULE, "hello_class");
    device_create(hello_class, NULL, dev_num, NULL, DEVICE_NAME);
​
    pr_info("hello_chrdev: initialized successfully\n");
    return 0;
}
​
static void __exit hello_exit(void)
{
    device_destroy(hello_class, dev_num);
    class_destroy(hello_class);
    cdev_del(&hello_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("hello_chrdev: unloaded\n");
}
​
module_init(hello_init);
module_exit(hello_exit);
MODULE_LICENSE("GPL");
```

## **三、测试驱动是否生效**

### **1\. 编译并加载驱动**

```bash
make
insmod hello_chrdev.ko
```

内核日志应出现：

```text
hello_chrdev: initialized successfully
```

### **2\. 检查设备节点**

```bash
ls -l /dev/hello_chrdev
```

### **3\. 读写测试**

```bash
echo "hello from user" > /dev/hello_chrdev
cat /dev/hello_chrdev
```

控制台输出：

```text
hello from user
```

## **四、常见问题与排查建议**

| 问题 | 排查建议 |
| --- | --- |
| 没有生成 /dev/hello_chrdev | 检查 class_create() 和 device_create() 是否成功 |
| 写入无效或乱码 | 检查内核 buffer 是否被正确处理/初始化 |
| copy_from_user() 失败 | 注意用户态传入的指针是否合法 |
| read() 总返回 0 | 注意 loff_t *offset，是否未更新 |

## **五、与前几讲驱动有何不同？**

| 对象类型 | 前几讲（platform/I2C/SPI） | 本讲（char device） |
| --- | --- | --- |
| 驱动入口 | probe() 来自 platform/i2c/spi 系统 | init() 自注册，无需绑定外设 |
| 通信方式 | 与外设通信（GPIO/I2C/SPI） | 与用户空间通信（文件接口） |
| 用户态访问 | 通常通过 sysfs/debugfs | /dev/xxx 文件，标准IO |
| 特征 | 多为外设驱动 | 通用接口，常用于中间层/调试接口 |

## **✅ 总结一句话**

> **字符设备驱动是用户空间与内核空间之间的“通信桥梁”，掌握它意味着你能设计自己的 `/dev` 设备。**

几乎所有可交互外设驱动，最终都离不开字符设备形式的封装与发布。
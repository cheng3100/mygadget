# 驱动调试技巧合集——printk、dmesg、debugfs 与 trace 的实战用法

**Author:** dump linux

**Date:** 2025-07-03

**Link:** https://zhuanlan.zhihu.com/p/1922942186057635216

驱动开发的痛点，不在写，而在**调试**。

-   内核模块加载失败，原因不明？
-   probe() 不触发，设备是否匹配？
-   IRQ 进来了，但执行结果怪异？
-   用户态调用 `read()` 没数据，在哪里断了？

在用户态你可以 `printf`，但在内核态，**你需要一整套武器库**。

本讲我们就来系统梳理：Linux 驱动开发中常用的调试手段及**实战用法**。

## **一、基础输出：printk + dmesg**

内核打印的第一选择就是：

```c
printk(KERN_INFO "xxx\n");
```

或其简化宏：

```c
pr_info("模块加载成功\n");
pr_err("I2C 读取失败\n");
pr_debug("中断次数=%d\n", count);
```

这些日志会进入内核日志缓冲区：

```bash
dmesg | grep my_driver
```

> **用法建议：建议用 \`pr\_xxx()\` 宏，风格统一，可手动加上模块名。**

## **二、动态调试：开启/关闭调试语句**

有时候你写了 `pr_debug()`，但什么也不输出？那是因为默认关闭了调试输出。

启用方法：

```bash
echo 'file my_driver.c +p' > /sys/kernel/debug/dynamic_debug/control
```

关闭方法：

```bash
echo 'file my_driver.c -p' > /sys/kernel/debug/dynamic_debug/control
```

或使用 `modprobe` 开启：

```bash
modprobe my_driver dyndbg=+p
```

> 要启用该机制，需内核编译时打开 `CONFIG_DYNAMIC_DEBUG=y`

## **三、使用 debugfs 暴露内部状态**

如果你想在用户态动态查看驱动状态、统计信息、内部变量 —— 不要用 [sysfs](https://zhida.zhihu.com/search?content_id=259710753&content_type=Article&match_order=1&q=sysfs&zhida_source=entity)，**请用 debugfs**。

注册接口：

```c
#include <linux/debugfs.h>
​
static struct dentry *dir, *file;
static u32 irq_count;
​
static int __init my_debug_init(void)
{
    dir = debugfs_create_dir("my_driver", NULL);
    debugfs_create_u32("irq_count", 0444, dir, &irq_count);
    return 0;
}
```

用户态查看：

```bash
cat /sys/kernel/debug/my_driver/irq_count
```

或实时监测：

```bash
watch cat /sys/kernel/debug/my_driver/irq_count
```

## **四、trace\_printk：开发期临时追踪利器**

`trace_printk()` 是一个强大的开发期工具，用于快速插桩，但不会污染 dmesg。

```c
#include <linux/tracepoint.h>
trace_printk("key_irq triggered, value=%d\n", val);
```

查看：

```bash
cat /sys/kernel/debug/tracing/trace
```

开启：

```bash
echo 1 > /sys/kernel/debug/tracing/tracing_on
```

关闭：

```bash
echo 0 > /sys/kernel/debug/tracing/tracing_on
```

⚠️ **trace\_printk主要开销来自缓冲区写入，关闭后几乎没有开销。建议仅在调试时启用，避免在生产环境影响性能。**

## **五、查看 probe 匹配过程**

怀疑驱动 probe 没调用？

```bash
dmesg | grep -i probe
```

确认 DTS 匹配是否成功：

```bash
cat /proc/device-tree/*/compatible
```

或者添加 `printk("probe called\n");` 强制打印。

## **六、调试设备节点创建**

怀疑 `/dev/my_chrdev` 没有生成？

```bash
lsmod | grep my_chrdev
ls -l /dev | grep my_chrdev
```

确认 class 与 device 创建是否成功：

```c
class_create(...)
device_create(...)
```

检查是否有返回 NULL 或 IS\_ERR。

## **七、驱动崩溃或内核 oops 怎么办？**

使用 `addr2line` 追踪调用栈地址：

```bash
addr2line -e my_driver.ko 0x1234
```

更好的是，使用带调试信息的内核构建 (`CONFIG_DEBUG_INFO=y`) 并使用：

```bash
gdb vmlinux
(gdb) l *0xffffffff81234567
```

或者使用 `[crash](https://zhida.zhihu.com/search?content_id=259710753&content_type=Article&match_order=1&q=crash&zhida_source=entity)` 工具配合 `vmcore` 分析系统崩溃。

## **✨ 八、其他建议技巧合集**

| 场景 | 调试方式 |
| --- | --- |
| I2C/SPI 读写失败 | 使用 i2cdump、逻辑分析仪抓取波形 |
| probe 不调用 | 检查 DTS、compatible、驱动 of_match_table |
| 按键不响应中断 | cat /proc/interrupts 看中断计数是否变化 |
| poll/select 无响应 | 检查是否正确 wake_up()，且注册 poll() |
| GPIO 驱动异常 | 检查 gpiod_get() 与 gpiod_direction_* |

## **✅ 总结一句话**

> **能写驱动是本事，能调试驱动才是真功夫。**

调试手段是开发者的眼睛，不熟练这些工具，你写的代码就像“蒙着眼”在跑。
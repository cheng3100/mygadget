# GPIO子系统原理与驱动开发

**Author:** dump linux

**Date:** 2025-07-03

**Link:** https://zhuanlan.zhihu.com/p/1921865953706242957

在嵌入式系统中，有两个东西是“最先接触”的：

-   一个是 **LED**（点灯）
-   一个是 **GPIO**（通用输入输出口）

本讲，我们将从“点亮一盏LED灯”开始，**实战写一个完整的GPIO驱动**，并理解 Linux 内核下 GPIO 子系统的机制。

## **一、什么是 GPIO？**

GPIO（General Purpose Input Output）是 [SoC](https://zhida.zhihu.com/search?content_id=259612561&content_type=Article&match_order=1&q=SoC&zhida_source=entity) 提供的最基本硬件资源之一，可以被配置为：

-   输入：读取电平状态（如按键、传感器）
-   输出：控制电平（如点灯、控制开关）

虽然听起来简单，但在 Linux 驱动层面，它其实走的是一整套“GPIO 子系统”架构。

## **二、GPIO 子系统在 Linux 中的角色**

Linux 内核通过 **抽象接口** 管理 GPIO 引脚，主要目标：

| 目的 | 原因 |
| --- | --- |
| 屏蔽硬件差异 | 每个 SoC GPIO 编号/寄存器不同，抽象统一接口 |
| 提供统一 API | 驱动通过标准 API 控制引脚 |
| 支持设备树管理 | DTS 直接定义 GPIO 属性，内核动态解析 |
| 管理权限/中断/状态 | 避免重复申请、冲突访问 |

关键接口来自：

```c
#include <linux/gpio/consumer.h>
```

## **三、平台DTS中定义GPIO**

我们先从设备树中为驱动指定一个GPIO：

```text
led_test {
    compatible = "abc,led-test";
    gpios = <&gpio1 3 GPIO_ACTIVE_HIGH>;
};
```

这里的 `<&gpio1 3 GPIO_ACTIVE_HIGH>` 表示：

-   控制器是 `gpio1`
-   第3号引脚
-   有效电平为高（输出高点亮）

## **四、驱动中如何获取并使用GPIO？**

Linux 推荐使用 GPIO 描述符接口（gpiod），更现代、抽象好。

```c
#include <linux/gpio/consumer.h>
​
struct gpio_desc *led;
​
led = devm_gpiod_get(&pdev->dev, NULL, GPIOD_OUT_HIGH);
if (IS_ERR(led))
    return PTR_ERR(led);
```

点亮 / 熄灭 LED：

```c
gpiod_set_value(led, 1);  // 输出高
gpiod_set_value(led, 0);  // 输出低
```

在 `probe()` 中完整写法如下：

```c
static int led_test_probe(struct platform_device *pdev)
{
    struct gpio_desc *led;
​
    led = devm_gpiod_get(&pdev->dev, NULL, GPIOD_OUT_LOW);
    if (IS_ERR(led))
        return PTR_ERR(led);
​
    gpiod_set_value(led, 1);  // 默认点亮
    dev_info(&pdev->dev, "LED is on.\n");
    return 0;
}
```

## **五、注册驱动完整示例**

```c
static const struct of_device_id led_test_dt_ids[] = {
    { .compatible = "abc,led-test" },
    { }
};
​
static struct platform_driver led_test_driver = {
    .probe = led_test_probe,
    .driver = {
        .name = "led_test",
        .of_match_table = led_test_dt_ids,
    },
};
​
module_platform_driver(led_test_driver);
MODULE_LICENSE("GPL");
```

编译后生成 `led_test.ko`，使用：

```bash
insmod led_test.ko
dmesg | grep LED
```

## **六、字符设备控制方式（进阶可选）**

你可以将 GPIO 通过字符设备暴露到用户空间：

```c
static ssize_t led_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos)
{
    if (buf[0] == '1')
        gpiod_set_value(led, 1);
    else
        gpiod_set_value(led, 0);
    return len;
}
```

这样用户可以直接：

```bash
echo 1 > /dev/led_test
```

实现从应用层控制 GPIO。

## **七、常见问题与排查**

| 问题 | 排查建议 |
| --- | --- |
| GPIO 不起作用 | DTS 写错控制器、引脚编号、有效电平 |
| gpiod_get() 报错 -ENOENT/-EINVAL | 检查设备树中是否定义 gpios = ... |
| LED 无响应 | GPIO 方向未设为输出、或者电路为反相配置 |
| DTS 改了驱动没生效 | 未重新编译 .dtb 或烧写镜像 |

## **✅ 总结一句话**

> **点亮一盏LED，是驱动开发的“Hello World”。但要写好它，你需要理解设备树、GPIO子系统、驱动绑定和调试流程的全链条。**

掌握 GPIO 子系统，是你打开所有外设驱动开发的第一道门。
# 驱动入口点——platform_device解析

**Author:** dump linux

**Date:** 2025-07-03

**Link:** https://zhuanlan.zhihu.com/p/1921519370875695788

当你写完[设备树](https://zhida.zhihu.com/search?content_id=259571384&content_type=Article&match_order=1&q=%E8%AE%BE%E5%A4%87%E6%A0%91&zhida_source=entity)、编好内核，接下来一定会问：

> 驱动是怎么“被系统找到”并“启动起来”的？

这一讲我们揭开驱动加载的第一层面纱：**platform\_device 与 platform\_driver 的匹配与调用机制**。这部分几乎是**所有 RK 平台驱动开发的核心基础**。

## **一、什么是 platform\_device？**

在内核中，**platform\_device 表示一个“平台设备”**——即在 [SoC](https://zhida.zhihu.com/search?content_id=259571384&content_type=Article&match_order=1&q=SoC&zhida_source=entity) 内部通过 bus（如 [APB](https://zhida.zhihu.com/search?content_id=259571384&content_type=Article&match_order=1&q=APB&zhida_source=entity)、[AHB](https://zhida.zhihu.com/search?content_id=259571384&content_type=Article&match_order=1&q=AHB&zhida_source=entity)）连接的外设，例如 GPIO、[I2C 控制器](https://zhida.zhihu.com/search?content_id=259571384&content_type=Article&match_order=1&q=I2C+%E6%8E%A7%E5%88%B6%E5%99%A8&zhida_source=entity)、[定时器](https://zhida.zhihu.com/search?content_id=259571384&content_type=Article&match_order=1&q=%E5%AE%9A%E6%97%B6%E5%99%A8&zhida_source=entity)、[RTC](https://zhida.zhihu.com/search?content_id=259571384&content_type=Article&match_order=1&q=RTC&zhida_source=entity)、[PWM](https://zhida.zhihu.com/search?content_id=259571384&content_type=Article&match_order=1&q=PWM&zhida_source=entity) 等。

它和 PCI、USB 那些“热插拔设备”不同：

| 类型 | 特点 | 示例设备 |
| --- | --- | --- |
| Platform | 固定地址，编译时已知 | UART、GPIO、PWM |
| USB/PCI | 动态发现，支持热插拔 | U盘、网卡、声卡 |

所以我们写自定义驱动时，**大多数是 platform 驱动**。

## **二、platform\_driver 与 device 的匹配机制**

驱动加载的大致流程：

1.  系统通过设备树，注册一个 platform\_device；
2.  驱动模块注册一个 platform\_driver；
3.  内核自动匹配 `compatible` 或 `name`；
4.  匹配成功后，调用 driver 的 `[probe()](https://zhida.zhihu.com/search?content_id=259571384&content_type=Article&match_order=1&q=probe%28%29&zhida_source=entity)` 函数；
5.  你在 `probe()` 中初始化设备、申请资源等。

## **三、示例解析：驱动与设备如何对接？**

### **① 设备树定义（[DTS](https://zhida.zhihu.com/search?content_id=259571384&content_type=Article&match_order=1&q=DTS&zhida_source=entity)）**

```text
my_sensor@40 {
    compatible = "abc,my-sensor";
    reg = <0x40>;
    status = "okay";
};
```

### **② 驱动注册代码**

```c
static const struct of_device_id my_sensor_of_match[] = {
    { .compatible = "abc,my-sensor" },
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, my_sensor_of_match);
​
static struct platform_driver my_sensor_driver = {
    .driver = {
        .name = "my_sensor",
        .of_match_table = my_sensor_of_match,
    },
    .probe = my_sensor_probe,
    .remove = my_sensor_remove,
};
​
module_platform_driver(my_sensor_driver);
```

关键点解读：

| 关键字段 | 含义 |
| --- | --- |
| of_match_table | 用于和 DTS 中的 compatible 进行匹配 |
| .name | 可用于传统的非 DT 匹配，但 RK 平台主要使用 DT 方式 |
| probe() | 设备匹配成功时调用，你的驱动代码就是从这里开始运行的 |
| module_platform_driver() | 宏封装了 init/exit 的注册与注销逻辑 |

## **四、`probe()`函数中你该做什么？**

这是驱动开发最关键的回调函数，一般执行：

1.  解析设备树参数（如中断号、GPIO等）
2.  映射寄存器资源（使用 `of_iomap()` / `devm_*()`）
3.  注册字符设备、input、misc 等子系统接口
4.  设置中断处理函数
5.  初始化定时器、tasklet、工作队列等结构

> 举个例子：我们来看看一个典型的 `probe()` 模板

```c
static int my_sensor_probe(struct platform_device *pdev)
{
    struct resource *res;
    void __iomem *regs;
​
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    regs = devm_ioremap_resource(&pdev->dev, res);
    if (IS_ERR(regs))
        return PTR_ERR(regs);
​
    dev_info(&pdev->dev, "my sensor probed at 0x%x\n", res->start);
​
    // 设备初始化、注册接口等...
    return 0;
}
```

## **⚙️ 五、platform\_device 是从哪来的？**

在 RK 平台中，一般不需要手动写 `platform_device` 注册代码，而是：

-   内核启动时解析设备树（`.dtb`）
-   根据 `compatible` 字段自动构建 `platform_device` 并加入总线
-   内核遍历所有 `platform_driver` 并试图匹配 `of_match_table`

所以，**写对设备树 + 驱动匹配表，系统就会自动帮你完成 device-driver 绑定。**

## **六、内核调试建议与命令**

### **查看设备是否注册成功**

```bash
dmesg | grep my-sensor
```

### **查看驱动绑定信息**

```bash
cat /sys/bus/platform/devices/
cat /sys/bus/platform/drivers/my_sensor/
```

### **动态加载驱动**

```bash
insmod my_sensor.ko
```

## **❌ 常见问题与排查建议**

| 现象 | 原因与解决办法 |
| --- | --- |
| probe() 没有调用 | 检查 DTS 中 compatible 是否写错 |
| 编译时报错找不到 platform_get_resource | 确保头文件和 kernel 版本匹配 |
| insmod 成功但没有输出日志 | 检查是否开启了 dev_info() 输出，串口连接是否正常 |
| 多驱动竞争一个设备 | 检查 of_match_table 是否有歧义 |

## **✅ 总结一句话**

> **platform\_driver 是 Linux 中连接设备树与驱动逻辑的桥梁，理解这个机制，你就可以“驱动一切”。**

RK 平台几乎所有外设（SPI、I2C、GPIO、摄像头、屏幕）都走这个框架，所以这部分理解透了，后面就水到渠成。
# 中断驱动机制详解——以GPIO按键为例

**Author:** dump linux

**Date:** 2025-07-03

**Link:** https://zhuanlan.zhihu.com/p/1922940154672321096

在之前的几讲中，我们已经实现了GPIO、[I2C](https://zhida.zhihu.com/search?content_id=259710433&content_type=Article&match_order=1&q=I2C&zhida_source=entity)、[SPI](https://zhida.zhihu.com/search?content_id=259710433&content_type=Article&match_order=1&q=SPI&zhida_source=entity)等外设驱动，但它们大多是“主动轮询”的。

那如果你想让系统**自动响应某个事件**，比如：

-   用户按下一个物理按键
-   外部模块发来中断信号
-   某个状态发生变化…

这就离不开中断机制了。本讲我们将带你写一个**完整的中断驱动**，通过 **GPIO + IRQ** 实现“按键触发”的事件响应。

## **一、中断（IRQ）基础知识**

中断是一种 **异步触发机制**，可以让 CPU 停止当前执行任务，优先处理一个高优先级事件。

在 Linux 内核中，常用术语包括：

| 名称 | 含义 |
| --- | --- |
| IRQ number | 中断编号，由硬件或 DTS 定义 |
| ISR | 中断服务函数（Interrupt Service Routine） |
| Edge/Level | 中断触发方式：上升沿、下降沿、低电平等 |
| Debounce | 消抖机制，用于抑制按钮弹跳带来的虚假触发 |

## **二、DTS 中定义中断属性（以 GPIO 按键为例）**

```text
key_input {
    compatible = "abc,key-input";
    gpios = <&gpio2 4 GPIO_ACTIVE_LOW>;
    interrupt-parent = <&gpio2>;
    interrupts = <4 IRQ_TYPE_EDGE_FALLING>;
};
```

说明：

-   `gpios`：指定物理引脚
-   `interrupts`：声明中断号 + 触发方式
-   `IRQ_TYPE_EDGE_FALLING`：下降沿触发（按下时为低）

> 注意：不同平台中 GPIO 控制器是否支持中断、支持哪些触发方式，请查阅芯片手册或 device tree bindings 文档。

## **三、驱动代码编写：注册并处理中断**

### **1\. probe 函数中获取 IRQ 并注册 handler**

```c
static irqreturn_t key_irq_handler(int irq, void *dev_id)
{
    pr_info("Key Pressed! IRQ: %d\n", irq);
    return IRQ_HANDLED;
}
​
static int key_input_probe(struct platform_device *pdev)
{
    int irq;
    struct gpio_desc *key_gpio;
​
    key_gpio = devm_gpiod_get(&pdev->dev, NULL, GPIOD_IN);
    if (IS_ERR(key_gpio))
        return PTR_ERR(key_gpio);
​
    irq = gpiod_to_irq(key_gpio);
    if (irq < 0)
        return irq;
​
    return devm_request_irq(&pdev->dev, irq, key_irq_handler,
                            IRQF_TRIGGER_FALLING, "key_irq", NULL);
}
```

### **2\. 注册平台驱动**

```c
static const struct of_device_id key_input_of_match[] = {
    { .compatible = "abc,key-input" },
    { }
};
​
static struct platform_driver key_input_driver = {
    .probe = key_input_probe,
    .driver = {
        .name = "key_input",
        .of_match_table = key_input_of_match,
    },
};
​
module_platform_driver(key_input_driver);
MODULE_LICENSE("GPL");
```

## **四、测试中断是否生效**

-   编译驱动并加载：

```bash
insmod key_input.ko
```

-   使用串口/ADB 观察内核日志：

```bash
dmesg -w
```

-   按下连接的按键（GND → GPIO2\_4）：

```text
[ 123.456 ] Key Pressed! IRQ: 149
```

恭喜你！第一个中断驱动生效了！

## **五、常见问题与调试建议**

| 问题 | 原因与排查方法 |
| --- | --- |
| gpiod_to_irq() 返回负值 | DTS 中缺少 interrupts 或 GPIO 不支持中断 |
| 按下按键没有响应 | 检查触发方式、是否消抖失败、是否短接 GND |
| IRQ 报错“busy or invalid” | 同一 GPIO 被多次注册、未正确 free |
| 中断抖动频繁触发 | 硬件未加 RC 消抖，或软件端缺乏延迟机制处理 |

## **六、进阶：中断 + 用户空间交互**

我们可以配合 **`wake_up_interruptible()`** + **`poll()`/`select()`** 实现用户态等待中断：

```c
DECLARE_WAIT_QUEUE_HEAD(key_wq);
static int key_event_flag = 0;
​
static irqreturn_t key_irq_handler(int irq, void *dev_id)
{
    key_event_flag = 1;
    wake_up_interruptible(&key_wq);
    return IRQ_HANDLED;
}
```

在字符设备驱动中实现 `poll()`：

```c
static unsigned int key_poll(struct file *file, poll_table *wait)
{
    poll_wait(file, &key_wq, wait);
    if (key_event_flag) {
        key_event_flag = 0;
        return POLLIN;
    }
    return 0;
}
```

## **✅ 总结一句话**

> **中断是驱动开发中的灵魂机制，学会它你就能让设备主动“开口说话”。**

从GPIO按键，到网络芯片、音频输入、硬件事件，几乎所有的外设交互都离不开中断。
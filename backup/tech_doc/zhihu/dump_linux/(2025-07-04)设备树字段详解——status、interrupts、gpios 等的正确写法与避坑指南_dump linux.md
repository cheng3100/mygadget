# 设备树字段详解——status、interrupts、gpios 等的正确写法与避坑指南

**Author:** dump linux

**Date:** 2025-07-04

**Link:** https://zhuanlan.zhihu.com/p/1922944093719364605

驱动不生效的 90% 原因，不在代码，而在 **[设备树](https://zhida.zhihu.com/search?content_id=259711100&content_type=Article&match_order=1&q=%E8%AE%BE%E5%A4%87%E6%A0%91&zhida_source=entity)（[DTS](https://zhida.zhihu.com/search?content_id=259711100&content_type=Article&match_order=1&q=DTS&zhida_source=entity)）**。

-   `[probe()](https://zhida.zhihu.com/search?content_id=259711100&content_type=Article&match_order=1&q=probe%28%29&zhida_source=entity)` 不调用？
-   GPIO 控不起来？
-   中断没反应？
-   明明 [compatible](https://zhida.zhihu.com/search?content_id=259711100&content_type=Article&match_order=1&q=compatible&zhida_source=entity) 写对了，驱动就是不起作用？

**设备树字段没写对**，是最常见也最容易忽视的坑。

本讲我们集中梳理 Linux 设备树中最常见的几个关键字段：`status`、`compatible`、`[reg](https://zhida.zhihu.com/search?content_id=259711100&content_type=Article&match_order=1&q=reg&zhida_source=entity)`、`[interrupts](https://zhida.zhihu.com/search?content_id=259711100&content_type=Article&match_order=1&q=interrupts&zhida_source=entity)`、`gpios` —— 以及它们背后的“坑”。

## **一、status：设备启用的“总开关”**

```text
&spi1 {
    status = "okay";   // 开启设备，默认可能是 "disabled"
};
```

### **常见值**

| 值 | 含义 |
| --- | --- |
| "okay" | 启用 |
| "disabled" | 禁用，不会绑定驱动 |
| "fail" | 初始化失败（不常用） |

### **⚠️ 常见坑**

-   没写 `status`，默认可能是 `"disabled"`
-   父节点被禁用，子节点也会被递归忽略
-   字段拼写错误如 `statuss` → 驱动永远不调用！

**建议：** 对所有新增设备节点都显式写上 `status = "okay";`。

## **二、compatible：驱动匹配的核心关键字**

```text
my_sensor@40 {
    compatible = "abc,my-sensor";
};
```

-   内核通过 `[of_match_table](https://zhida.zhihu.com/search?content_id=259711100&content_type=Article&match_order=1&q=of_match_table&zhida_source=entity)` 与 `compatible` 字符串匹配来调用 `probe()`

### **⚠️ 常见坑**

-   多个 compatible 值时，要与驱动匹配表一致（可写成数组）
-   匹配失败不会报错，只是悄悄忽略，需用 `[dmesg](https://zhida.zhihu.com/search?content_id=259711100&content_type=Article&match_order=1&q=dmesg&zhida_source=entity)` 排查

```text
compatible = "abc,my-sensor", "generic,i2c-device";
static const struct of_device_id my_match[] = {
    { .compatible = "abc,my-sensor" },
    {}
};
```

## **三、reg：地址 or 片选号**

该字段用于定义设备地址（I2C）或片选编号（SPI）或资源索引（MMIO）。

### **I2C 示例**

```text
my_sensor@40 {
    reg = <0x40>;  // I2C 地址
};
```

### **SPI 示例**

```text
oled@0 {
    reg = <0>;  // SPI 片选号 CS0
};
```

### **⚠️ 常见坑**

-   I2C 地址必须为 7 位有效地址
-   reg 写错，驱动可能 `probe()` 失败或总线冲突
-   `@xx` 也要匹配 reg 值，否则设备树编译会出现警告

## **⏲ 四、interrupts / interrupt-parent：中断定义**

```text
button@0 {
    gpios = <&gpio0 4 GPIO_ACTIVE_LOW>;
    interrupt-parent = <&gpio0>;
    interrupts = <4 IRQ_TYPE_EDGE_FALLING>;
};
```

| 字段名 | 含义 |
| --- | --- |
| interrupt-parent | 哪个控制器生成该中断（GPIO 控制器） |
| interrupts | 中断号（通常是 GPIO 号） + 类型 |

### **中断触发类型**

| 宏名 | 说明 |
| --- | --- |
| IRQ_TYPE_EDGE_RISING | 上升沿触发 |
| IRQ_TYPE_EDGE_FALLING | 下降沿触发 |
| IRQ_TYPE_LEVEL_HIGH | 高电平触发 |
| IRQ_TYPE_LEVEL_LOW | 低电平触发 |

### **⚠️ 常见坑**

-   `interrupt-parent` 必须指定，否则中断号无效
-   `interrupts = <4 1>` 中的 `1` 是触发类型，不可省略（中断控制器节点中\`#interrupt-cells = <2>\`时需要设置触发类型）
-   GPIO 不一定支持中断（某些引脚无IRQ能力）

## **五、gpios：引脚控制接口（GPIO descriptor）**

```text
reset-gpios = <&gpio1 3 GPIO_ACTIVE_LOW>;
```

该语法中：

-   `&gpio1` 表示 GPIO 控制器（node）
-   `3` 表示 GPIO 编号
-   `GPIO_ACTIVE_LOW` 表示低电平有效（影响 `gpiod_set_value()` 的行为）

### **在驱动中使用**

```c
struct gpio_desc *rst;
rst = devm_gpiod_get(&pdev->dev, "reset", GPIOD_OUT_LOW);
gpiod_set_value(rst, 1);
```

### **⚠️ 常见坑**

-   名称不匹配，如 `reset-gpios`写成 `reset-gpio`
-   电平极性反了（高低逻辑颠倒）导致设备无法工作
-   GPIO 控制器未开启（父节点 status="disabled"）

## **六、例子：一个完整的设备节点**

```text
&i2c1 {
    status = "okay";
    my_sensor@40 {
        compatible = "abc,my-sensor";
        reg = <0x40>;

        interrupt-parent = <&gpio2>;
        interrupts = <10 IRQ_TYPE_EDGE_FALLING>;

        reset-gpios = <&gpio1 5 GPIO_ACTIVE_LOW>;
        status = "okay";
    };
};
```

## **七、调试设备树的小技巧**

### **1\. 查看设备树运行时结构**

```bash
cat /proc/device-tree/i2c1/my_sensor@40/compatible
hexdump -C /proc/device-tree/...
```

### **2\. 检查设备树是否加载成功**

-   使用 `fdtgrep` 工具查字段（需安装）
-   使用 `dtc` 反编译 `/boot/dtb` 看实际加载结构

### **3\. 添加 log 验证是否匹配驱动**

在驱动的 `probe()` 中添加：

```c
dev_info(&pdev->dev, "probe called for %s\n", dev_name(&pdev->dev));
```

## **✅ 总结一句话**

> **设备树字段写得对，你的驱动才有资格被“召唤”；写错了，内核就当它不存在。**

设备树 = 硬件信息描述语言，懂它才能调得好驱动。
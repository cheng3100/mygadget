# 设备树（Device Tree）到底怎么读？

**Author:** dump linux

**Date:** 2025-07-03

**Link:** https://zhuanlan.zhihu.com/p/1920946673393333974

你或许见过这样的代码片段：

```c
&i2c1 {
    status = "okay";
    my_sensor@40 {
        compatible = "myvendor,my-sensor";
        reg = <0x40>;
        ...
    };
};
```

初看像 JSON，结果不能加注释；  
像 XML，又没有闭合标签；  
能看懂一半，却不知道这东西到底“怎么用”“干嘛的”。

别急，这一讲我们**彻底讲清楚[设备树](https://zhida.zhihu.com/search?content_id=259508430&content_type=Article&match_order=1&q=%E8%AE%BE%E5%A4%87%E6%A0%91&zhida_source=entity)（[DTS](https://zhida.zhihu.com/search?content_id=259508430&content_type=Article&match_order=1&q=DTS&zhida_source=entity)）的来龙去脉与阅读技巧**。

## 一、设备树是什么？

设备树（Device Tree）是**用来描述硬件资源**的数据结构。它告诉操作系统：“我有哪些设备，地址是多少，驱动怎么找我。”

在 Linux 内核启动早期，内核**读取一个 `.dtb` 文件（[DTS 编译](https://zhida.zhihu.com/search?content_id=259508430&content_type=Article&match_order=1&q=DTS+%E7%BC%96%E8%AF%91&zhida_source=entity)而来）**，由此建立系统的“硬件抽象视图”。

通俗一点：

-   **设备树 = 硬件的说明书**  
    
-   用“树状结构”写明设备挂载在哪个总线上、用哪个地址、需要什么驱动

## 二、设备树的组成结构

以一个 [I2C 传感器](https://zhida.zhihu.com/search?content_id=259508430&content_type=Article&match_order=1&q=I2C+%E4%BC%A0%E6%84%9F%E5%99%A8&zhida_source=entity)为例，我们来拆解典型的设备树结构：

```c
&i2c1 {
    status = "okay";               // 启用i2c1控制器
    my_sensor@40 {
        compatible = "abc,my-sensor";  // 匹配的驱动名
        reg = <0x40>;                  // I2C 地址
        interrupt-parent = <&gpio1>;
        interrupts = <23 IRQ_TYPE_EDGE_RISING>;
        ...
    };
};
```

### 关键字段解读：

| 字段 | 作用 |
| --- | --- |
| &i2c1 | 引用已定义的 i2c 控制器节点（SoC 手册中有定义） |
| status | 是否启用设备，通常为 "okay" 或 "disabled" |
| my_sensor@40 | 设备节点名称 + I2C 地址（@40） |
| compatible | 关键字段，用于驱动和设备匹配 |
| reg | 寄存器/地址信息，表示地址为 0x40 |
| interrupts | 中断号及触发方式 |

## ⚙️ 三、设备树是如何与驱动绑定的？

这是很多初学者感到“魔法”的一部分，其实是明规则 + 内核机制：

-   驱动代码中，定义支持的 `compatible` 字符串：

```c
static const struct of_device_id my_sensor_ids[] = {
    { .compatible = "abc,my-sensor" },
    {}
};
MODULE_DEVICE_TABLE(of, my_sensor_ids);
```

-   内核加载设备树时会逐个节点解析，匹配驱动中注册的 `compatible`；  
    
-   匹配成功后，调用该驱动的 `probe()` 函数，并将设备信息传入。

✅ 所以，只要你的 DTS 写了 `"compatible = x"`，而驱动中注册了这个 x，两者就能“握手成功”。

## 四、[RK平台](https://zhida.zhihu.com/search?content_id=259508430&content_type=Article&match_order=1&q=RK%E5%B9%B3%E5%8F%B0&zhida_source=entity)下的DTS实战路径

在`Linux`内核源码中，设备树文件路径大致如下：

```c
kernel/arch/arm64/boot/dts/rockchip/
```

-   主板设备树：`rk3588s-orangepi-5.dts`  
    
-   SoC通用配置：`rk3588s.dtsi`

常见写法：

```c
&i2c0 {
    status = "okay";

    sensor@1a {
        compatible = "ovti,ov5647";
        reg = <0x1a>;
        clocks = <&cru SCLK_CIF_OUT>;
        ...
    };
};
```

> 提示：设备树文件中 `#include`、`/include/` 语法也要注意，不同板型间有继承关系。

## 五、DTS阅读技巧 & 常见误区

### ✅ 阅读技巧

-   **从外设节点入手**（如 i2c、spi），查找其挂载的子节点  
    
-   使用 SDK 提供的 `scripts/dtc` 工具编译 `.dts` → `.dtb` 进行验证  
    
-   使用 `fdtdump` 或 `hexdump -C` 查看 dtb 内容，辅助调试

### ❌ 常见误区

| 误区 | 正解 |
| --- | --- |
| DTS 写了 compatible 就一定能生效 | 驱动必须注册了对应 compatible，并成功编进内核 |
| DTS 改了就生效 | 改完必须重新编译 .dtb 并重新烧录/替换 |
| DTS 是 C 语言 | DTS 是 FDT（Flattened Device Tree）格式，不是 C 语法 |

## 总结一句话

> **设备树就像是硬件的说明书+配置文件，驱动程序根据它来“认识硬件”并“绑定硬件”。**

会读、会写、会改 DTS，是嵌入式驱动工程师的**入门必修课**。

**扩展阅读 & 工具推荐**：

-   DTS 语法手册：[https://elinux.org/Device\_Tree\_Usage](https://link.zhihu.com/?target=https%3A//elinux.org/Device_Tree_Usage)  
    
-   fdtput/fdtget 工具使用  
    
-   推荐阅读：内核文档 `Documentation/devicetree/` 下的各类子系统说明
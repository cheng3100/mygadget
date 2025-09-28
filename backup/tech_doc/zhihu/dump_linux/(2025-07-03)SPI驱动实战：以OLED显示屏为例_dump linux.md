# SPI驱动实战：以OLED显示屏为例

**Author:** dump linux

**Date:** 2025-07-03

**Link:** https://zhuanlan.zhihu.com/p/1922239933357220840

相比 [I2C](https://zhida.zhihu.com/search?content_id=259656947&content_type=Article&match_order=1&q=I2C&zhida_source=entity)，[SPI 协议](https://zhida.zhihu.com/search?content_id=259656947&content_type=Article&match_order=1&q=SPI+%E5%8D%8F%E8%AE%AE&zhida_source=entity)传输速度更快、协议更简单，是嵌入式平台中非常常见的通信方式。

你会在很多场景中遇到 SPI 设备：

-   Flash 存储（如 W25Qxx 系列）
-   OLED/LCD 显示屏（如 SSD1306/ILI9341）
-   多通道ADC/DAC芯片
-   SD卡接口（早期模式）
-   各类自定义专用芯片…

今天我们就以一个典型 SPI OLED 模块（SSD1306 控制器）为例，实战编写一个 SPI 驱动，并在 [RK 平台](https://zhida.zhihu.com/search?content_id=259656947&content_type=Article&match_order=1&q=RK+%E5%B9%B3%E5%8F%B0&zhida_source=entity)上成功显示图像。

## **一、SPI 设备通信原理简述**

| 信号线 | 说明 |
| --- | --- |
| SCLK | 时钟线 |
| MOSI | 主设备输出，从设备输入 |
| MISO | 主设备输入，从设备输出 |
| CS/SS | 片选信号 |

SPI 是全双工通信协议，**通信更快，但需要更多线**（4根）。不像 I2C，SPI 没有地址机制，而是通过片选线（CS）区分设备。

## **二、[DTS](https://zhida.zhihu.com/search?content_id=259656947&content_type=Article&match_order=1&q=DTS&zhida_source=entity) 中定义 SPI 外设节点**

```text
&spi1 {
    status = "okay";
    oled@0 {
        compatible = "abc,oled-spi";
        reg = <0>;                    // 表示片选号（CS0）
        spi-max-frequency = <10000000>;
​
        dc-gpios = <&gpio3 2 GPIO_ACTIVE_HIGH>; // Data/Command 引脚
        reset-gpios = <&gpio2 5 GPIO_ACTIVE_LOW>;
    };
};
```

说明：

-   `reg = <0>`：表示使用 SPI 控制器的第0号片选（CS0）
-   `spi-max-frequency`：设置传输速率上限
-   `dc-gpios`、`reset-gpios`：常用于OLED控制的引脚，需单独控制

## **三、驱动核心结构（spi\_driver）**

### **1\. 匹配表**

```c
static const struct of_device_id oled_dt_ids[] = {
    { .compatible = "abc,oled-spi" },
    {}
};
MODULE_DEVICE_TABLE(of, oled_dt_ids);
```

### **2\. probe 函数**

```c
static int oled_probe(struct spi_device *spi)
{
    struct gpio_desc *dc_gpio, *reset_gpio;
    int ret;
​
    dev_info(&spi->dev, "OLED SPI device probed\n");
​
    // 获取 GPIO
    dc_gpio = devm_gpiod_get(&spi->dev, "dc", GPIOD_OUT_LOW);
    reset_gpio = devm_gpiod_get(&spi->dev, "reset", GPIOD_OUT_LOW);
​
    // OLED 硬复位
    gpiod_set_value(reset_gpio, 0);
    msleep(50);
    gpiod_set_value(reset_gpio, 1);
​
    // 初始化 OLED
    oled_init_sequence(spi, dc_gpio);
​
    return 0;
}
```

## **四、SPI 数据发送方式**

发送一段命令（如 OLED 初始化指令）：

```c
int oled_send_cmd(struct spi_device *spi, u8 cmd, struct gpio_desc *dc_gpio)
{
    gpiod_set_value(dc_gpio, 0);  // 指令模式
    return spi_write(spi, &cmd, 1);
}
```

发送数据（如图像）：

```c
int oled_send_data(struct spi_device *spi, const u8 *data, int len, struct gpio_desc *dc_gpio)
{
    gpiod_set_value(dc_gpio, 1);  // 数据模式
    return spi_write(spi, data, len);
}
```

## **五、注册 SPI 驱动模块**

```c
static struct spi_driver oled_driver = {
    .driver = {
        .name = "oled_spi",
        .of_match_table = oled_dt_ids,
    },
    .probe = oled_probe,
    .remove = NULL,
};
​
module_spi_driver(oled_driver);
MODULE_LICENSE("GPL");
```

编译后插入模块：

```bash
insmod oled_spi.ko
```

> 确认设备节点：`ls /sys/bus/spi/devices/`

## **六、与 I2C 驱动有何区别？**

| 项目 | I2C | SPI |
| --- | --- | --- |
| 驱动类型 | i2c_driver | spi_driver |
| 芯片地址 | DTS 中 reg = <0x3C> | DTS 中 reg = <0> 表示 CS0 |
| 总线机制 | 地址仲裁 + ACK | 片选CS，单设备控制 |
| 通信函数 | i2c_smbus_xxx / regmap | spi_write/read, spi_sync |
| 常用场景 | PMIC、传感器、EEPROM | 显示屏、Flash、通信模块 |

## **七、调试技巧**

| 调试项 | 命令/方法 |
| --- | --- |
| 驱动是否匹配成功 | dmesg \| grep oled |
| SPI 总线中设备是否注册 | ls /sys/bus/spi/devices/ |
| 检查SPI配置 | cat /proc/device-tree/spi1/oled@0/... |
| 抓取 SPI 信号 | 逻辑分析仪 / 示波器 / RK平台 SPI loopback |
| 显示图像 | 写简单测试程序，向 OLED 写入数据画面 |

## **✅ 总结一句话**

> **SPI 驱动开发是进阶嵌入式开发的必经之路，掌握它意味着你能驾驭更高速、复杂的外围设备通信。**

OLED 显示、Flash 存储、触控芯片，SPI 都是“通吃型”协议。
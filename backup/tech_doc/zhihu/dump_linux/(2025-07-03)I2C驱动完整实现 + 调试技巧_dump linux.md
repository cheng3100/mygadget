# I2C驱动完整实现 + 调试技巧

**Author:** dump linux

**Date:** 2025-07-03

**Link:** https://zhuanlan.zhihu.com/p/1922236553167639198

在[RK平台](https://zhida.zhihu.com/search?content_id=259656396&content_type=Article&match_order=1&q=RK%E5%B9%B3%E5%8F%B0&zhida_source=entity)或任何嵌入式平台上，[I2C](https://zhida.zhihu.com/search?content_id=259656396&content_type=Article&match_order=1&q=I2C&zhida_source=entity)设备几乎无处不在：

-   传感器（温湿度、光线、陀螺仪）
-   电源管理芯片（[PMIC](https://zhida.zhihu.com/search?content_id=259656396&content_type=Article&match_order=1&q=PMIC&zhida_source=entity)）
-   [EEPROM](https://zhida.zhihu.com/search?content_id=259656396&content_type=Article&match_order=1&q=EEPROM&zhida_source=entity)、触控芯片、显示接口芯片……

今天我们从0开始，一步步**写出一个完整的 I2C 设备驱动**，掌握读写流程、调试技巧以及 regmap 框架。

## **一、I2C在Linux中的结构**

I2C 驱动一般分为三部分：

| 层级 | 角色 | 示例 |
| --- | --- | --- |
| 控制器驱动 | i2c 控制器本身，如 rk3x-i2c | RK 平台已有，不需要你写 |
| 设备树描述 | 指定从设备、地址等 | DTS 中的 @xx 节点 |
| 客户端驱动 | 你要写的设备驱动 | 温度传感器/EEPROM等 |

你写的主要是客户端驱动：通过 **I2C API 访问寄存器**，与硬件通信。

## **二、DTS中定义I2C设备**

```text
&i2c1 {
    status = "okay";
    my_sensor@40 {
        compatible = "abc,my-sensor";
        reg = <0x40>;
    };
};
```

说明：

-   `i2c1` 是控制器；
-   `my_sensor@40` 是挂在该总线上的设备，I2C 地址是 `0x40`；
-   `compatible` 用于匹配驱动；

## **三、驱动代码完整流程（platform+i2c\_client）**

### **1\. 匹配表**

```c
static const struct of_device_id my_sensor_dt_ids[] = {
    { .compatible = "abc,my-sensor" },
    {}
};
MODULE_DEVICE_TABLE(of, my_sensor_dt_ids);
```

### **2\. probe函数**

```c
static int my_sensor_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int ret;
    u8 val;
​
    dev_info(&client->dev, "my_sensor probed at addr=0x%02x\n", client->addr);
​
    // 读取寄存器0x00
    ret = i2c_smbus_read_byte_data(client, 0x00);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to read reg 0x00\n");
        return ret;
    }
​
    val = ret;
    dev_info(&client->dev, "Register 0x00 = 0x%02x\n", val);
​
    return 0;
}
```

### **3\. 驱动注册**

```c
static struct i2c_driver my_sensor_driver = {
    .driver = {
        .name = "my_sensor",
        .of_match_table = my_sensor_dt_ids,
    },
    .probe = my_sensor_probe,
    .remove = NULL,
};
module_i2c_driver(my_sensor_driver);
MODULE_LICENSE("GPL");
```

编译出 `my_sensor.ko`，`insmod` 后自动 probe。

## **四、推荐使用 regmap 框架（可选优化）**

`regmap` 是 Linux 为简化寄存器访问而设计的通用API框架，适用于寄存器型设备（如I2C/SPI设备）。

### **初始化**

```c
struct regmap *regmap;
​
static const struct regmap_config my_sensor_regmap_config = {
    .reg_bits = 8,
    .val_bits = 8,
};
​
regmap = devm_regmap_init_i2c(client, &my_sensor_regmap_config);
if (IS_ERR(regmap))
    return PTR_ERR(regmap);
```

### **读写**

```c
regmap_write(regmap, 0x01, 0xFF);
regmap_read(regmap, 0x02, &val);
```

优势：

-   支持缓存/cache
-   易于扩展调试
-   支持 `debugfs` 自动挂载寄存器

## **五、调试技巧总结**

### **① 查看设备是否匹配成功**

```bash
dmesg | grep my_sensor
```

### **② 查看总线上设备**

```bash
i2cdetect -y 1   # 查看 i2c1 总线
```

输出如下表示地址为0x40的设备被探测到：

```text
     0 1 2 3 4 5 6 7 8 9 a b c d e f
00:          -- -- -- -- -- -- -- --
40: 40 -- -- -- -- -- -- -- -- -- --
```

### **③ 调试失败场景**

| 症状 | 原因与解决建议 |
| --- | --- |
| probe() 未触发 | 检查 DTS 是否启用，compatible 是否匹配 |
| 读写寄存器失败（-EREMOTEIO） | I2C 设备未连接、地址错误、供电异常 |
| i2cdetect 无法探测设备 | 部分设备不响应 probe，可无视，只看 probe() 调用 |
| 总线挂死，系统卡住 | 时序问题，可尝试 reset GPIO + bus recover |

## **六、与平台 driver 不同之处？**

| 项目 | platform_driver | i2c_driver |
| --- | --- | --- |
| 绑定机制 | device tree + match | device tree + i2c_client |
| 获取资源 | platform_get_resource() | i2c_client->addr |
| 核心结构 | platform_device | i2c_client |
| 通信方式 | 内存映射寄存器 | i2c_smbus / regmap API |

## **✅ 总结一句话**

> **I2C 驱动是嵌入式驱动开发中的重要基石，掌握它意味着你可以“和设备说话”了。**

学会它，你可以驱动各种传感器、IO扩展器、PMIC、电机控制器……打通万物互联。
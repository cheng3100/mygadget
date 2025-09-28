# RK平台开发环境准备指南（含源码/编译器）

**Author:** dump linux

**Date:** 2025-07-03

**Link:** https://zhuanlan.zhihu.com/p/1921302069513556288

想在[瑞芯微](https://zhida.zhihu.com/search?content_id=259554125&content_type=Article&match_order=1&q=%E7%91%9E%E8%8A%AF%E5%BE%AE&zhida_source=entity)（Rockchip）平台上开发驱动，第一步永远不是写代码，而是把开发环境搭好。

如果你开发环境都还没跑起来，一定不要跳过这一讲。**环境不顺，调试一生。**

本讲我们将手把手搭好一套完整的 [RK Linux](https://zhida.zhihu.com/search?content_id=259554125&content_type=Article&match_order=1&q=RK+Linux&zhida_source=entity) 驱动开发环境，从源码、工具链、配置、编译到镜像验证，适配主流开发板如 [RK3588](https://zhida.zhihu.com/search?content_id=259554125&content_type=Article&match_order=1&q=RK3588&zhida_source=entity)、[RK3566](https://zhida.zhihu.com/search?content_id=259554125&content_type=Article&match_order=1&q=RK3566&zhida_source=entity)、[RK3399](https://zhida.zhihu.com/search?content_id=259554125&content_type=Article&match_order=1&q=RK3399&zhida_source=entity) 等。

## **一、开发环境总览**

我们要准备以下几个部分：

| 项目 | 说明 |
| --- | --- |
| Linux源码 | 包含内核、设备树、驱动 |
| 编译工具链 | aarch64-linux-gnu-gcc |
| U-Boot源码 | 引导程序，可选编译 |
| 调试接口 | UART串口、网口、SD卡等 |

## **二、获取 Linux BSP**

通常通过以下几种方式获取：

### **✅ 官方渠道**

-   开发板厂家(以[香橙派](https://zhida.zhihu.com/search?content_id=259554125&content_type=Article&match_order=1&q=%E9%A6%99%E6%A9%99%E6%B4%BE&zhida_source=entity)为例)：[https://github.com/orangepi-xunlong/linux-orangepi](https://link.zhihu.com/?target=https%3A//github.com/orangepi-xunlong/linux-orangepi)
-   `Rockchip` 官方：[https://github.com/rockchip-linux/kernel](https://link.zhihu.com/?target=https%3A//github.com/rockchip-linux/kernel)

`Rockchip` 官方对具体的开发板支持比较欠缺，请获取开发板厂家提供的 `Linux` 源码。

> 建议选择和开发板上预装系统版本一致的 `Linux Kernel` 分支。

## **三、准备交叉编译工具链**

不同的 RK SoC 用的是不同架构，这里只列出最常用的 `AArch64` 架构：

| 平台 | 架构 | 推荐交叉编译器 |
| --- | --- | --- |
| RK3588 | AArch64 | aarch64-linux-gnu-gcc |
| RK3566 | AArch64 | aarch64-linux-gnu-gcc |
| RK3399 | AArch64 | aarch64-linux-gnu-gcc |

### **✅ 安装方式（Ubuntu/Debian）**

```bash
sudo apt install gcc-aarch64-linux-gnu
```

## **四、构建内核并生成镜像**

### **1\. 配置内核**

```bash
cd <kernel_source_dir>
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- rockchip_linux_defconfig
```

### **2\. 编译内核**

```bash
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- rockchip_linux_defconfig -j$(nproc)
```

输出物：

-   `Image`：内核镜像
-   `rk3588s-orangepi-5-pro.dtb`：设备树文件
-   `modules/`：驱动模块

### **3\. 构建 rootfs（可选）**

使用开发板厂家提供的构建工具：[https://github.com/orangepi-xunlong/orangepi-build](https://link.zhihu.com/?target=https%3A//github.com/orangepi-xunlong/orangepi-build)

## **五、更新系统镜像**

### **✅ 导出内核模块**

```text
mkdir build-modules
make modules_install INSTALL_MOD_PATH=./build-modules/
```

### **✅ 导出内核镜像和initramfs**

```text
mkdir build-boot
make install INSTALL_PATH=./build-boot/
```

### **✅ 导出设备树**

```text
mkdir build-dtbs
make dtbs_install INSTALL_DTBS_PATH=./build-dtbs/
```

### **✅ 安装**

通过 `ssh` 远程拷贝到开发板，或者拷贝到开发板的 `SD` 卡后按需更新相关系统镜像文件。

```text
sudo cp -r build-modules/lib/modules/x.x.x-xxx/ /lib/modules/
sudo cp build-boot/* /boot/
sudo cp -r build-dtbs/rockchip /boot/dtbs/
```

## **六、调试口连接（串口 + ssh）**

### **串口连接**

-   板子一般提供 3.3V TTL UART
-   推荐使用 **USB转串口模块（CP2102/CH340）**
-   推荐使用 `MobaXterm` 自带的串口工具，波特率设置为 **1500000**（RK 常用波特率）

### **ssh远程连接**

-   若板子已运行 `Linux` + `OpenSSH server` 服务

```bash
ssh xxx@192.168.1.x
```

## **七、开发板目录结构速览**

```text
├── /boot/           # 内核镜像、initramfs
├── /boot/dtbs/      # 设备树
├── /lib/modules/    # 内核模块
```

## **八、常见问题排查**

| 问题 | 原因与解决办法 |
| --- | --- |
| 编译时报错找不到交叉编译器 | CROSS_COMPILE 路径不对或未安装工具链 |
| DTS修改后无效 | 未重新编译 dtb，或者镜像未更新 |
| 串口乱码 | 波特率错误，确认是否 1500000（RK 常用） |
| 编译慢 | 使用 -j$(nproc) 充分利用多核 |

## **✅ 总结一句话**

> **搭建好开发环境，是驱动开发最重要的第一步，RK 平台的逻辑是清晰的，只要按流程搭建就能顺利上手。**

如果你跟着操作到这里，恭喜你，已经可以开始编写自己的第一个驱动模块了！
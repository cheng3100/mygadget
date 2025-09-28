# DMA驱动机制详解——高效数据搬运与 zero-copy 的实现路径

**Author:** dump linux

**Date:** 2025-07-04

**Link:** https://zhuanlan.zhihu.com/p/1922944642296550813

在嵌入式驱动开发中，我们经常需要将大量数据从内存传输到设备，或者反过来。

如果你还在用 `[memcpy](https://zhida.zhihu.com/search?content_id=259711233&content_type=Article&match_order=1&q=memcpy&zhida_source=entity)()` + `[write](https://zhida.zhihu.com/search?content_id=259711233&content_type=Article&match_order=1&q=write&zhida_source=entity)()` 来搬数据，那你的驱动性能可能只发挥了 30%。

要想高效，就得引入**DMA（[Direct Memory Access](https://zhida.zhihu.com/search?content_id=259711233&content_type=Article&match_order=1&q=Direct+Memory+Access&zhida_source=entity)）机制** —— 让设备绕过CPU、直连内存，高速搬运。

本讲我们将从驱动视角出发，手把手讲清：

-   DMA 的底层机制
-   Linux 中的常用接口
-   如何映射用户空间内存供 DMA 使用
-   [RK 平台](https://zhida.zhihu.com/search?content_id=259711233&content_type=Article&match_order=1&q=RK+%E5%B9%B3%E5%8F%B0&zhida_source=entity)的 DMA 注意事项

## **一、什么是 DMA？为什么需要它？**

DMA（Direct Memory Access）是一种**硬件协助搬数据的机制**，跳过 CPU，直接在内存 ↔ 设备之间传输数据。

| 对比项 | 使用 memcpy + write | 使用 DMA |
| --- | --- | --- |
| 是否占用 CPU | 是 | 否（异步） |
| 是否拷贝两次数据 | 是（用户态→内核→设备） | 否（可 zero-copy） |
| 数据量大时性能 | 低 | 高 |
| 驱动复杂度 | 简单 | 略高，需要 DMA API 管理内存和同步 |

## **二、Linux 驱动中使用 DMA 的常见场景**

-   摄像头采集图像
-   音频录制 / 播放
-   SPI / I2C 大块数据收发（如 flash 烧录）
-   显示驱动将 framebuffer 推送给显示控制器
-   NPU、VPU 等 AI 处理器与内存交互

## **三、DMA 在 Linux 内核中的三种内存分配方式**

### **1\. `dma_alloc_coherent()`：分配连续物理页，设备与CPU可同时访问**

```c
dma_addr_t dma_handle;
void *cpu_addr = dma_alloc_coherent(dev, size, &dma_handle, GFP_KERNEL);
```

-   **cpu\_addr**：CPU 虚拟地址（可访问）
-   **dma\_handle**：设备访问的总线地址（通常是物理地址或[IOMMU](https://zhida.zhihu.com/search?content_id=259711233&content_type=Article&match_order=1&q=IOMMU&zhida_source=entity)地址）

### **2\. `dma_map_single()`：将现有 buffer 转换为设备可访问地址**

适合你自己已经有 `kmalloc()` 出来的 buffer。

```c
dma_addr_t dma_handle = dma_map_single(dev, buf, size, DMA_TO_DEVICE);
```

使用完后要释放：

```c
dma_unmap_single(dev, dma_handle, size, DMA_TO_DEVICE);
```

### **3\. `dma_alloc_attrs()`：底层通用接口，支持更多属性控制**

用于有特殊需求的驱动（如显存、异步写缓冲）。

## **四、zero-copy：让用户态 buffer 被设备直接访问**

普通做法：

```text
用户态 malloc
↓ write() 系统调用
内核态 copy_from_user
↓ DMA copy
设备访问
```

zero-copy：

```text
内核态 dma_alloc_coherent()
↓ 映射 buffer 到用户空间
用户态 mmap buffer
↓ DMA copy
设备访问
```

### **核心步骤**

1.  用户态通过 `mmap()` 映射驱动提供的 DMA buffer
2.  驱动中实现 `.mmap()` 接口，将 `dma_alloc_coherent()` 地址暴露给用户空间

```c
static int mydrv_mmap(struct file *filp, struct vm_area_struct *vma)
{
    return dma_mmap_coherent(dev, vma, cpu_addr, dma_handle, size);
}
```

> ⚠️ 注意：必须使用 `dma_alloc_coherent()` 分配的内存才能这样映射

## **五、完整驱动示例片段**

### **分配 DMA buffer**

```c
#define DMA_SIZE 4096
​
static void *dma_buf;
static dma_addr_t dma_phy;
​
dma_buf = dma_alloc_coherent(&pdev->dev, DMA_SIZE, &dma_phy, GFP_KERNEL);
```

### **提供 mmap 映射给用户态**

```c
static int demo_mmap(struct file *filp, struct vm_area_struct *vma)
{
    return dma_mmap_coherent(&pdev->dev, vma, dma_buf, dma_phy, DMA_SIZE);
}
```

用户空间使用：

```c
fd = open("/dev/mydma", O_RDWR);
// buf 就是设备可直接访问的内存
buf = mmap(NULL, DMA_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
```

## **六、RK 平台使用 DMA 的注意事项**

| 项目 | 注意点 |
| --- | --- |
| IOMMU 是否启用 | 若开启，dma_handle 不是物理地址，而是 IOVA |
| DMA buffer cache一致性 | 使用 dma_alloc_coherent() 可保证一致性，避免 cache 不一致问题 |
| DMA 控制器 | RK平台常用 DMAC，某些外设有专用 DMA 通道 |
| 设备树中声明 | 某些设备需显式声明 dma-coherent; 否则分配失败 |

## **七、调试 DMA 的技巧与工具**

-   使用 `dma-debug` 选项编译内核，可以打印所有映射情况
-   使用 `ioremap()` / `memcpy()` 替代 DMA 做对比，观察性能差异
-   使用 `ftrace` 或 `perf` 统计 DMA 操作时间
-   查看 `/proc/iomem` 或 `/sys/kernel/debug/dma_buf` 状态信息

## **✅ 总结一句话**

> **DMA 是驱动高效搬数据的“快车道”，学会它，你的驱动就拥有了向性能要空间的能力。**

摄像头、音频、AI 模块，哪怕是 SPI flash 烧录，想高性能都绕不开 DMA。
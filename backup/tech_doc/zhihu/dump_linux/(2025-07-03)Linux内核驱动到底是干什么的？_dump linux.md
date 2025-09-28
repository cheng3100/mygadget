# Linux内核驱动到底是干什么的？

**Author:** dump linux

**Date:** 2025-07-03

**Link:** https://zhuanlan.zhihu.com/p/1920895238974738902

在成为一名驱动工程师之前，我也一度以为“驱动”就是把[寄存器](https://zhida.zhihu.com/search?content_id=259501188&content_type=Article&match_order=1&q=%E5%AF%84%E5%AD%98%E5%99%A8&zhida_source=entity)地址敲进去，点灯、读数、搞定。直到真正入行，才发现这只是冰山一角。

今天，我们从三个问题聊起：

-   驱动开发的本质是什么？  
    
-   它和“应用层开发”有什么区别？  
    
-   为什么嵌入式系统里，驱动开发如此重要？  
    

## 一、驱动到底是什么？

通俗讲：  
**[驱动程序](https://zhida.zhihu.com/search?content_id=259501188&content_type=Article&match_order=1&q=%E9%A9%B1%E5%8A%A8%E7%A8%8B%E5%BA%8F&zhida_source=entity)是“操作系统”和“硬件”之间的翻译官。**

-   硬件只认寄存器、电压电流；  
    
-   应用层代码（比如用Python或C写的程序）根本不知道下面有多复杂；  
    
-   中间的“翻译工作”，就是驱动在干的事。  
    

在 Linux 内核中，每一类设备（比如 SPI、I2C、USB、GPU、摄像头、屏幕）都有一套完整的驱动框架——这既是一种标准，也是一种约束。

## 二、驱动和应用的区别是什么？

我们用一个**点亮LED灯**的例子对比一下：

| 项目 | 驱动工程师视角 | 应用层程序员视角 |
| --- | --- | --- |
| 关注点 | 硬件寄存器、内核API、设备树匹配 | open/close/ioctl/read/write等调用 |
| 工作层次 | 内核态 | 用户态 |
| 成果 | 编写内核模块，注册设备驱动 | 写个小程序控制GPIO设备 |
| 难度 | 高（需要掌握硬件+内核机制） | 低（依赖已有驱动） |

简言之：

-   应用是“用户在使用设备”；  
    
-   驱动是“内核在控制设备”。  
    

## 三、驱动在嵌入式系统中有多重要？

对于使用[瑞芯微](https://zhida.zhihu.com/search?content_id=259501188&content_type=Article&match_order=1&q=%E7%91%9E%E8%8A%AF%E5%BE%AE&zhida_source=entity)（RK）、[全志](https://zhida.zhihu.com/search?content_id=259501188&content_type=Article&match_order=1&q=%E5%85%A8%E5%BF%97&zhida_source=entity)、[NXP](https://zhida.zhihu.com/search?content_id=259501188&content_type=Article&match_order=1&q=NXP&zhida_source=entity)等 SoC 的嵌入式开发者来说，驱动就是通往硬件世界的钥匙。

尤其是国产平台，大量外设（摄像头、MIPI屏、传感器）都需要自己适配。没有现成驱动，你的应用就是“盲人”。

**而驱动工程师的价值，就体现在这三点：**

1.  **你能让“不会说话”的硬件，听懂Linux。**  
    
2.  **你能通过设备树、内核框架，接入系统，统一管理。**  
    
3.  **你能优化性能、排查故障，撑起整个系统的稳定性。**  
    

## 写在最后

你可能会问：

> “我非得学驱动吗？Linux不是已经很成熟了吗？”  

没错，Linux已经很强大。但当你接触到定制板子、国产芯片、新外设时，**你会发现——驱动，是最关键的一环**。

在接下来的专栏中，我会结合 RK 平台（比如 [RK3588s](https://zhida.zhihu.com/search?content_id=259501188&content_type=Article&match_order=1&q=RK3588s&zhida_source=entity)、[RK3399](https://zhida.zhihu.com/search?content_id=259501188&content_type=Article&match_order=1&q=RK3399&zhida_source=entity) 等），带你**从零写出自己的驱动代码**，不仅能用、还能调、还能改。
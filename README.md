<div align=center><img src="https://raw.githubusercontent.com/loongwrt/loongwrt/master/pic/loongwrt.png" alt="loongwrt"/></div>

# 项目介绍

这是一个致力于国产芯在OpenWrt上移植和研发的开源项目, 经过测试且稳定运行的LS1C300B系该项目的开山之作. 我们决定成立loongwrt项目并将源码公开, 邀请感兴趣的您一起来参与.愿更多的开发者参与进来, 一起完善!

<div align=center><img src="https://raw.githubusercontent.com/loongwrt/loongwrt/master/pic/ls1c300b-on-rosy.png" alt="ls1c300b-on-rosy"/></div>

# 心路历程

1. 2018年12月, LS1C300B智龙V3开发板在OpenWrt上移植成功.

* bootloader : 采用PMON
* openwrt : master分支
* loongwrt : loongson32-3.18 可以理解该分支为稳定版
* 存在的问题: 内核版本过旧, 没有u-boot支持

2. 2019年1月, 在上一版本上基于Linux-4.19内核移植, 但不稳定.

* bootloader : 采用PMON
* openwrt : master分支
* loongwrt : master 可以理解该分支为开发版本/体验版
* Flash :  **W25Q128FV 16MBytes**
* 存在的问题: 每次运行到文件系统初始化(mount_root阶段)就随机出现各种错误, 例如, 无故重启, 卡死(较少), 内核panic, 待其反复重启数次后方才稳定下来.

3. 2019年6月1日，龙芯派2代在一众好友的帮助下移植成功.

* bootloader : 采用[PMON](https://github.com/FlyGoat/pmon-ls2k-pi2/releases/download/v2.3/flash.bin)
* openwrt : master分支
* loongwrt : 分支名为loong64-3.10
* linux : linux源码在[这里](http://cgit.loongnix.org/cgit/linux-3.10)可以下载
* 存在的问题 : 该版本仅为一个初稿，基础应用和外设的支持和稳定性有待更新和修复，欢迎大家踊跃参与贡献.

# 新版期待

<div align=center><img src="https://raw.githubusercontent.com/loongwrt/loongwrt/master/pic/ls1c300b-n19-board.jpg" alt="ls1c300b-n19-board"/></div>

# 众人拾柴火焰高

Alipay : rosysong@rosinson.com

Wechat :

<div align=center><img src="https://raw.githubusercontent.com/loongwrt/loongwrt/master/pic/wechat-pay.jpg" alt="wechat-pay"/></div>


# 项目介绍

这是一个致力于国产芯在OpenWrt上移植和研发的开源项目, 经过测试且稳定运行的LS1C300B系该项目的开山之作. 愿更多的开发者参与进来, 共同完善!

<div align=center><img src="https://raw.githubusercontent.com/loongwrt/loongwrt/master/pic/ls1c300b-on-rosy.png" alt="ls1c300b-on-rosy"/></div>

# 项目历程

1. 2018年12月, LS1C300B智龙V3开发板在OpenWrt上移植成功.

    bootloader : 采用PMON
    openwrt : master分支

    存在的问题: 内核版本过旧, 没有u-boot支持

2. 2019年12月, 在上一版本上基于Linux-4.19内核移植, 但不稳定.

    bootloader : 采用PMON
    openwrt : master分支

    存在的问题: 每次运行到文件系统初始化(mount_root阶段)就随机出现各种错误,
    例如, 无故重启, 卡死(较少), 内核panic, 待其反复重启数次后方才稳定下来.

3. 我们决定成立loongwrt项目组织, 将源码公开, 邀请感兴趣的您一起来参与.

# 新版期待

<div align=center><img src="https://raw.githubusercontent.com/loongwrt/loongwrt/master/pic/ls1c300b-n19-board.jpg" alt="ls1c300b-n19-board"/></div>

# 众人拾柴火焰高

Alipay : rosysong@rosinson.com

Wechat :

<div align=center><img src="https://raw.githubusercontent.com/loongwrt/loongwrt/master/pic/wechat-pay.jpg" alt="wechat-pay"/></div>


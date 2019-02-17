#
# Copyright (C) 2018 OpenWrt.org
# Copyright (C) 2018 RosyWrt Team
# Copyright (C) 2018 Rosy Song <rosysong@rosinson.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

ARCH:=mipsel
BOARD:=loongson32
BOARDNAME:=Loongson32 Boards
FEATURES:=nand usb squashfs
SUBTARGETS:=ls1c300b

KERNEL_PATCHVER:=3.18

DEVICE_TYPE:=other

DEFAULT_PACKAGES += \
	kmod-gpio-button-hotplug kmod-leds-gpio

define Target/Description
	Build firmware images for Loongson1x based boards.
endef

include $(INCLUDE_DIR)/target.mk

$(eval $(call BuildTarget))

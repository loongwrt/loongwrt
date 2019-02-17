#
# Copyright (C) 2009-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

UIMAGE:=$(BIN_DIR)/$(IMG_PREFIX)-uImage.bin

define MkImageGzip
	mkimage -A mips -O linux -T kernel -a 0x80200000 -C gzip \
		-e 0x80200000 -n 'MIPS OpenWrt Linux-$(LINUX_VERSION)' \
		-d $(1) $(2)
endef

define Image/Prepare
	$(call MkImageGzip,$(LINUX_DIR)/arch/$(LINUX_KARCH)/boot/compressed/vmlinux.bin.z,$(KDIR)/uimage)
endef

define Image/BuildKernel
	$(CP) $(LINUX_DIR)/vmlinuz $(UIMAGE)
endef

define Image/Build/squashfs
	$(call prepare_generic_squashfs,$(KDIR)/root.squashfs)
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/$(IMG_PREFIX)-root.$(1) bs=128k conv=sync
	( \
		dd if=$(UIMAGE) bs=7680k conv=sync; \
		dd if=$(KDIR)/root.$(1) bs=128k conv=sync; \
	) > $(BIN_DIR)/$(IMG_PREFIX)-$(1)-sysupgrade.img
endef

define Image/Build
	$(call Image/Build/squashfs,squashfs)
endef

$(eval $(call BuildImage))

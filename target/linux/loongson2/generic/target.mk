BOARDNAME:=Generic
CPU_TYPE :=mips64
FEATURES += usb
DEFAULT_PACKAGES += 

define Target/Description
	Build firmware images for modern loongson2 based boards with CPUs
	supporting at least the mips64(little endian) instruction set with
	LOONGSON2K Pi2.
endef


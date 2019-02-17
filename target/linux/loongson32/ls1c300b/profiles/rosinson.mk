
define Profile/N19
	NAME:=ROSINSON N19 Nas board
	PACKAGES:=kmod-usb2 kmod-usb-storage \
		kmod-usb-uhci kmod-usb-ohci
endef

define Profile/N19/Description
	Package set compatible with the ROSINSON N19 board.
endef

$(eval $(call Profile,N19))

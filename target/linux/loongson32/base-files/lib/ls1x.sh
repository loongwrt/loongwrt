#!/bin/sh
#
# Copyright (C) 2018 RosyWrt Team
# Copyright (C) 2018 Rosy Song <rosysong@rosinson.com>
#

LS1X_BOARD_NAME=
LS1X_MODEL=

ls1x_board_detect() {
	local machine
	local name

	machine=$(awk 'BEGIN{FS="[ \t]+:[ \t]"} /machine/ {print $2}' /proc/cpuinfo)

	case "$machine" in
	*"N19")
		name="n19"
		;;
	esac

	[ -z "$name" ] && name="unknown"

	[ -z "$LS1X_BOARD_NAME" ] && LS1X_BOARD_NAME="$name"
	[ -z "$LS1X_MODEL" ] && LS1X_MODEL="$machine"

	[ -e "/tmp/sysinfo/" ] || mkdir -p "/tmp/sysinfo/"

	echo "$LS1X_BOARD_NAME" > /tmp/sysinfo/board_name
	echo "$LS1X_MODEL" > /tmp/sysinfo/model
}

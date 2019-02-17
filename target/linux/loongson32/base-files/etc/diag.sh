#!/bin/sh
# Copyright (C) 2018 RosyWrt Team
# Copyright (C) 2018 Rosy Song <rosysong@rosinson.com>

. /lib/functions.sh
. /lib/functions/leds.sh

get_status_led() {
	local board=$(board_name)

	case $board in
	n19)
		status_led="n19:red:status"
		;;
	esac
}

set_state() {
	get_status_led

	case "$1" in
	preinit)
		status_led_blink_preinit
		;;
	failsafe)
		status_led_blink_failsafe
		;;
	preinit_regular)
		status_led_blink_preinit_regular
		;;
	done)
		status_led_on
		;;
	esac
}

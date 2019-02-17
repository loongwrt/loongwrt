#!/bin/sh

do_ls1x() {
	. /lib/ls1x.sh

	ls1x_board_detect
}

boot_hook_add preinit_main do_ls1x

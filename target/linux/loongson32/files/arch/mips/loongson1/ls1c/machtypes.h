/*
 *  Loongson32 machine type definitions
 *
 *  Copyright (C) 20018 RosyWrt Team
 *  Copyright (C) 20018 Rosy Song <rosysong@rosinson.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#ifndef _LS1X_MACHTYPE_H
#define _LS1X_MACHTYPE_H

#include <asm/mips_machine.h>

enum ls1x_mach_type {
	LS1X_MACH_GENERIC_OF = -1,	/* Device tree board */
	LS1X_MACH_GENERIC = 0,
	LS1X_MACH_N19,
};

#endif /* _LS1X_MACHTYPE_H */

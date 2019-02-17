/* 
 * LS1C - HWMon interface for ADC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __ASM_ARCH_ADC_HWMON_H
#define __ASM_ARCH_ADC_HWMON_H __FILE__

/* LS1C ADC register */
#define ADC_CNT			0x00
#define ADC_S_CTRL		0x04
#define ADC_C_CTRL		0x08
#define X_RANGE			0x10
#define Y_RANGE			0x14
#define AWATCHDOG_RANGE	0x18
#define AXIS			0x1c
#define ADC_S_DOUT0		0x20
#define ADC_S_DOUT1		0x24
#define ADC_C_DOUT		0x28
#define ADC_DEBOUNCE_CNT	0x2c
#define ADC_INT			0x30

#define LS1X_ADC_PRE(x)	(((x)&0xFFF)<<20)

/**
 * ls1x_hwmon_chcfg - channel configuration
 * @name: The name to give this channel.
 * @mult: Multiply the ADC value read by this.
 * @div: Divide the value from the ADC by this.
 *
 * The value read from the ADC is converted to a value that
 * hwmon expects (mV) by result = (value_read * @mult) / @div.
 */
struct ls1x_hwmon_chcfg {
	const char	*name;
	unsigned int	mult;
	unsigned int	div;
	int single;
};

/**
 * ls1x_hwmon_pdata - HWMON platform data
 * @in: One configuration for each possible channel used.
 */
struct ls1x_hwmon_pdata {
	struct ls1x_hwmon_chcfg	*in[4];
};

/**
 * ls1x_hwmon_set_platdata - Set platform data for S3C HWMON device
 * @pd: Platform data to register to device.
 *
 * Register the given platform data for use with the S3C HWMON device.
 * The call will copy the platform data, so the board definitions can
 * make the structure itself __initdata.
 */
extern void __init ls1x_hwmon_set_platdata(struct ls1x_hwmon_pdata *pd);

#endif /* __ASM_ARCH_ADC_HWMON_H */


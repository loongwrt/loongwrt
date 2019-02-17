/*
 * Copyright (c) 2011 Zhang, Keguang <keguang.zhang@gmail.com>
 * Copyright (c) 2015 Tang Haifeng <tanghaifeng-gz@loongson.cn> or <pengren.mcu@qq.com>
 *
 * This program is free software; you can redistribute	it and/or modify it
 * under  the terms of	the GNU General	 Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/clk.h>
#include <linux/gpio.h>

#include <platform.h>
#include <loongson1.h>
#include <irq.h>

#include "machtypes.h"

#ifdef CONFIG_MTD_NAND_LS1X
#include <ls1x_nand.h>
static struct mtd_partition ls1x_nand_partitions[] = {
	{
		.name	= "bootloader",
		.offset	= MTDPART_OFS_APPEND,
		.size	= 1024*1024,
	},  {
		.name	= "kernel",
		.offset	= MTDPART_OFS_APPEND,
		.size	= 13*1024*1024,
	},  {
		.name	= "rootfs",
		.offset	= MTDPART_OFS_APPEND,
		.size	= 50*1024*1024,
	},  {
		.name	= "data",
		.offset	= MTDPART_OFS_APPEND,
		.size	= MTDPART_SIZ_FULL,
	},
};

struct ls1x_nand_platform_data ls1x_nand_parts = {
	.parts		= ls1x_nand_partitions,
	.nr_parts	= ARRAY_SIZE(ls1x_nand_partitions),
};
#endif

#if defined(CONFIG_MTD_M25P80) || defined(CONFIG_MTD_M25P80_MODULE)
#include <linux/spi/flash.h>
static struct mtd_partition ls1x_spi_flash_partitions[] = {
	{
		.name = "pmon(spi)",
		.size = 0x00080000,
		.offset = 0,
	}
};

static struct flash_platform_data ls1x_spi_flash_data = {
	.name = "spi-flash",
	.parts = ls1x_spi_flash_partitions,
	.nr_parts = ARRAY_SIZE(ls1x_spi_flash_partitions),
	.type = "w25x40",
};
#endif

#ifdef CONFIG_SPI_LS1X_SPI0
#include <linux/spi/spi.h>
#include <linux/spi/spi_ls1x.h>
#if defined(CONFIG_SPI_CS_USED_GPIO)
static int spi0_gpios_cs[] = { 81, 82, 83, 84 };
#endif

struct ls1x_spi_platform_data ls1x_spi0_platdata = {
#if defined(CONFIG_SPI_CS_USED_GPIO)
	.gpio_cs_count = ARRAY_SIZE(spi0_gpios_cs),
	.gpio_cs = spi0_gpios_cs,
#elif defined(CONFIG_SPI_CS)
	.cs_count = SPI0_CS3 + 1,
#endif
};

static struct spi_board_info ls1x_spi0_devices[] = {
#if defined(CONFIG_MTD_M25P80) || defined(CONFIG_MTD_M25P80_MODULE)
	{
		.modalias	= "m25p80",
		.bus_num 		= 0,
		.chip_select	= SPI0_CS0,
		.max_speed_hz	= 60000000,
		.platform_data	= &ls1x_spi_flash_data,
		.mode = SPI_MODE_3,
	},
#endif
};
#endif

#if defined(CONFIG_KEYBOARD_GPIO) || defined(CONFIG_KEYBOARD_GPIO_MODULE)
#include <linux/input.h>
#include <linux/gpio_keys.h>
static struct gpio_keys_button ls1x_gpio_keys_buttons[] = {
	 {
		.code		= KEY_0,
		.gpio		= 85,
		.active_low	= 1,
		.desc		= "0",
		.wakeup		= 1,
		.debounce_interval	= 10, /* debounce ticks interval in msecs */
	},
	{
		.code		= KEY_1,
		.gpio		= 86,
		.active_low	= 1,
		.desc		= "1",
		.wakeup		= 1,
		.debounce_interval	= 10, /* debounce ticks interval in msecs */
	},
	{
		.code		= KEY_2,
		.gpio		= 92,
		.active_low	= 1,
		.desc		= "2",
		.wakeup		= 1,
		.debounce_interval	= 10, /* debounce ticks interval in msecs */
	},
};

static struct gpio_keys_platform_data ls1x_gpio_keys_data = {
	.nbuttons = ARRAY_SIZE(ls1x_gpio_keys_buttons),
	.buttons = ls1x_gpio_keys_buttons,
	.rep	= 1,	/* enable input subsystem auto repeat */
};

static struct platform_device ls1x_gpio_keys = {
	.name =	"gpio-keys",
	.id =	-1,
	.dev = {
		.platform_data = &ls1x_gpio_keys_data,
	}
};
#endif

#if defined(CONFIG_LEDS_GPIO) || defined(CONFIG_LEDS_GPIO_MODULE)
#include <linux/leds.h>
struct gpio_led ls1x_gpio_leds[] = {
	{
		.name			= "ok",
		.gpio			= 6,
		.active_low		= 0,
		.default_trigger	= NULL,
		.default_state	= LEDS_GPIO_DEFSTATE_ON,
	}, {
		.name			= "led1",
		.gpio			= 52,
		.active_low		= 1,
		.default_trigger	= NULL,
		.default_state	= LEDS_GPIO_DEFSTATE_ON,
	}, {
		.name			= "led2",
		.gpio			= 53,
		.active_low		= 1,
		.default_trigger	= NULL,
		.default_state	= LEDS_GPIO_DEFSTATE_ON,
	},
};

static struct gpio_led_platform_data ls1x_gpio_leds_pdata = {
	.leds		= ls1x_gpio_leds,
	.num_leds	= ARRAY_SIZE(ls1x_gpio_leds),
};

static struct platform_device ls1x_gpio_leds_info = {
	.name	= "leds-gpio",
	.id	= -1,
	.dev	= {
		.platform_data	= &ls1x_gpio_leds_pdata,
	}
};
#endif //#if defined(CONFIG_LEDS_GPIO) || defined(CONFIG_LEDS_GPIO_MODULE)

static struct platform_device *ls1c_platform_devices[] __initdata = {
	&ls1x_uart_pdev,
#ifdef CONFIG_MTD_NAND_LS1X
	&ls1x_nand_pdev,
#endif
#if defined(CONFIG_LS1X_GMAC0)
	&ls1x_eth0_pdev,
#endif
#if defined(CONFIG_LS1X_GMAC1)
	&ls1x_eth1_pdev,
#endif
#ifdef CONFIG_USB_OHCI_HCD_PLATFORM
	&ls1x_ohci_pdev,
#endif
#ifdef CONFIG_USB_EHCI_HCD_PLATFORM
	&ls1x_ehci_pdev,
#endif
#ifdef CONFIG_LS1X_WDT
	&ls1x_wdt_pdev,
#endif
#ifdef CONFIG_USB_DWC2
	&ls1x_otg_pdev,
#endif
#ifdef CONFIG_SPI_LS1X_SPI0
	&ls1x_spi0_pdev,
#endif
#if defined(CONFIG_LEDS_GPIO) || defined(CONFIG_LEDS_GPIO_MODULE)
	&ls1x_gpio_leds_info,
#endif
#if defined(CONFIG_KEYBOARD_GPIO) || defined(CONFIG_KEYBOARD_GPIO_MODULE)
	&ls1x_gpio_keys,
#endif
};

static void __init n19_setup(void)
{
	int err;
	ls1x_serial_setup(&ls1x_uart_pdev);

#if defined(CONFIG_SPI_LS1X_SPI0)
	spi_register_board_info(ls1x_spi0_devices, ARRAY_SIZE(ls1x_spi0_devices));
#endif

	platform_add_devices(ls1c_platform_devices,
				   ARRAY_SIZE(ls1c_platform_devices));
	return err;
}

//arch_initcall(ls1c_platform_init);
MIPS_MACHINE(LS1X_MACH_N19, "N19", "ROSINSON N19 Board",
	n19_setup);

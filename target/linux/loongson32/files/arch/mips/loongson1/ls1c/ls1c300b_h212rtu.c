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
#include <linux/err.h>
#include <linux/pwm.h>

#include <platform.h>
#include <loongson1.h>
#include <irq.h>
#include <linux/phy.h>
#include <linux/stmmac.h>

#include <asm-generic/sizes.h>
#include "cbus.h"
#include <ls1x_nand.h>
static struct mtd_partition ls1x_nand_partitions[] = {
	{
		.name	= "kernel",
		.offset	= 2*1024*1024,
		.size	= 20*1024*1024,
	},  {
		.name	= "rootfs",
		.offset	= 22*1024*1024,
		.size	= 106*1024*1024,
	},  {
		.name	= "pmon(nand)",
		.offset	= 0,
		.size	= 2*1024*1024,
	},

};

struct ls1x_nand_platform_data ls1x_nand_parts = {
	.parts		= ls1x_nand_partitions,
	.nr_parts	= ARRAY_SIZE(ls1x_nand_partitions),
};

#include <linux/spi/flash.h>
static struct mtd_partition ls1x_spi_flash_partitions[] = {
	{
		.name = "pmon(spi)",
		.size = 0x00080000,
		.offset = 0,
//		.mask_flags = MTD_CAP_ROM
	}
};

static struct flash_platform_data ls1x_spi_flash_data = {
	.name = "spi-flash",
	.parts = ls1x_spi_flash_partitions,
	.nr_parts = ARRAY_SIZE(ls1x_spi_flash_partitions),
	.type = "w25x40",
};

#include <linux/spi/mmc_spi.h>
#include <linux/mmc/host.h>
/* 开发板使用GPIO6引脚作为MMC/SD卡的插拔探测引脚 */
#define DETECT_GPIO  56
//static int mmc_spi_get_cd(struct device *dev)
//{
//	return !gpio_get_value(DETECT_GPIO);
//}

static struct mmc_spi_platform_data mmc_spi __maybe_unused = {
//	.get_cd = mmc_spi_get_cd,
	.caps = MMC_CAP_NEEDS_POLL,
	.ocr_mask = MMC_VDD_32_33 | MMC_VDD_33_34, /* 3.3V only */
};

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
	{
		.modalias	= "m25p80",
		.bus_num 		= 0,
		.chip_select	= SPI0_CS0,
		.max_speed_hz	= 60000000,
		.platform_data	= &ls1x_spi_flash_data,
		.mode = SPI_MODE_3,
	},
	{
		.modalias		= "mmc_spi",
		.bus_num 		= 0,
		.chip_select	= SPI0_CS2,
		.max_speed_hz	= 25000000,
		.platform_data	= &mmc_spi,
		.mode = SPI_MODE_3,
	},
};

#include <linux/i2c.h>
static struct i2c_board_info ls1x_i2c0_board_info[] = {
};

static struct i2c_board_info ls1x_i2c1_board_info[] = {
};

static struct i2c_board_info ls1x_i2c2_board_info[] = {
};

#include <linux/i2c-ls1x.h>
struct ls1x_i2c_platform_data ls1x_i2c0_data = {
	.bus_clock_hz = 100000, /* i2c bus clock in Hz */
	.devices	= ls1x_i2c0_board_info, /* optional table of devices */
	.num_devices	= ARRAY_SIZE(ls1x_i2c0_board_info), /* table size */
};

struct ls1x_i2c_platform_data ls1x_i2c1_data = {
	.bus_clock_hz = 100000, /* i2c bus clock in Hz */
	.devices	= ls1x_i2c1_board_info, /* optional table of devices */
	.num_devices	= ARRAY_SIZE(ls1x_i2c1_board_info), /* table size */
};

struct ls1x_i2c_platform_data ls1x_i2c2_data = {
	.bus_clock_hz = 100000, /* i2c bus clock in Hz */
	.devices	= ls1x_i2c2_board_info, /* optional table of devices */
	.num_devices	= ARRAY_SIZE(ls1x_i2c2_board_info), /* table size */
};

static void ls1x_i2c_setup(void)
{
	/* 使能I2C控制器 */
	__raw_writel(__raw_readl(LS1X_MUX_CTRL0) & (~I2C0_SHUT), LS1X_MUX_CTRL0);
	__raw_writel(__raw_readl(LS1X_MUX_CTRL0) & (~I2C1_SHUT), LS1X_MUX_CTRL0);
	__raw_writel(__raw_readl(LS1X_MUX_CTRL0) & (~I2C2_SHUT), LS1X_MUX_CTRL0);
	__raw_writel(__raw_readl(LS1X_GPIO_CFG2) & ~(3<<(85-64)), LS1X_GPIO_CFG2); //关闭GPIO85/86

	/*
	 * PIN74    GPIO85    I2C_SDA0
	 * PIN75    GPIO86    I2C_SCL0
	 *
	 * PIN3    GPIO76    I2C_SDA1
	 * PIN6    GPIO77    I2C_SCL1
	 */
/*
	gpio_func_dis(1,85);//sda0
	gpio_func_dis(2,85);//sda0
	gpio_func_dis(3,85);//sda0
	gpio_func_dis(4,85);//sda0
	gpio_func_dis(5,85);//sda0
	gpio_func_dis(1,86);//scl0
	gpio_func_dis(2,86);//scl0
	gpio_func_dis(3,86);//scl0
	gpio_func_dis(4,86);//scl0
	gpio_func_dis(5,86);//scl0
*/
	gpio_func(4,50);//sda2
	gpio_func(4,51);//scl2
}

#ifdef CONFIG_SENSORS_LS1X
#include <hwmon.h>
static struct ls1x_hwmon_pdata bast_hwmon_info = {
	/* battery voltage (0-3.3V) */
	.in[0] = &(struct ls1x_hwmon_chcfg) {
		.name		= "battery-voltage",
		.mult		= 3300,	/* 3.3V参考电压 */
		.div		= 1024,
		.single		= 1,
	},
	.in[1] = &(struct ls1x_hwmon_chcfg) {
		.name		= "adc-d1",
		.mult		= 3300,	/* 3.3V参考电压 */
		.div		= 1024,
		.single		= 1,
	},
};

void ls1x_hwmon_set_platdata(struct ls1x_hwmon_pdata *pd)
{
	struct ls1x_hwmon_pdata *npd;

	if (!pd) {
		printk(KERN_ERR "%s: no platform data\n", __func__);
		return;
	}

	npd = kmemdup(pd, sizeof(struct ls1x_hwmon_pdata), GFP_KERNEL);
	if (!npd)
		printk(KERN_ERR "%s: no memory for platform data\n", __func__);

	ls1x_hwmon_pdev.dev.platform_data = npd;
}
#endif

#if defined(CONFIG_KEYBOARD_GPIO) || defined(CONFIG_KEYBOARD_GPIO_MODULE)
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
struct gpio_led gpio_leds[] = {
	{
		.name			= "ok",
		.gpio			= 6,
		.active_low		= 1,
//		.default_trigger	= "timer", /* 触发方式 */
		.default_trigger	= NULL,
		.default_state	= LEDS_GPIO_DEFSTATE_ON,
	}, {
		.name			= "led1",
		.gpio			= 52,
		.active_low		= 1,
//		.default_trigger	= "heartbeat", /* 触发方式 */
		.default_trigger	= NULL,
		.default_state	= LEDS_GPIO_DEFSTATE_ON,
	}, {
		.name			= "led2",
		.gpio			= 53,
		.active_low		= 1,
//		.default_trigger	= "timer",	/* 触发方式 */
		.default_trigger	= NULL,
		.default_state	= LEDS_GPIO_DEFSTATE_ON,
	},
};

static struct gpio_led_platform_data gpio_led_info = {
	.leds		= gpio_leds,
	.num_leds	= ARRAY_SIZE(gpio_leds),
};

static struct platform_device leds = {
	.name	= "leds-gpio",
	.id	= -1,
	.dev	= {
		.platform_data	= &gpio_led_info,
	}
};
#endif //#if defined(CONFIG_LEDS_GPIO) || defined(CONFIG_LEDS_GPIO_MODULE)


static struct platform_device *ls1c_platform_devices[] __initdata = {
	&ls1x_uart_pdev,
	&ls1x_nand_pdev,
	&ls1x_eth0_pdev,
	&ls1x_ohci_pdev,
	//&ls1x_ehci_pdev,
	&ls1x_toy_pdev,
	&ls1x_wdt_pdev,
#ifdef CONFIG_USB_GADGET_SNPS_DWC_OTG
	&ls1x_otg_pdev,
#endif
	&ls1x_spi0_pdev,
#ifdef CONFIG_LS1X_FB0
	&ls1x_fb0_pdev,
#endif
	&ls1x_i2c0_pdev,
	&ls1x_i2c1_pdev,
	&ls1x_i2c2_pdev,

#ifdef CONFIG_PWM_LS1X_PWM0
	&ls1x_pwm0_pdev,
#endif
#ifdef CONFIG_PWM_LS1X_PWM1
	&ls1x_pwm1_pdev,
#endif
#ifdef CONFIG_PWM_LS1X_PWM2
	&ls1x_pwm2_pdev,
#endif
#ifdef CONFIG_PWM_LS1X_PWM3
	&ls1x_pwm3_pdev,
#endif

#ifdef CONFIG_SENSORS_LS1X
	&ls1x_hwmon_pdev,
#endif
#if defined(CONFIG_KEYBOARD_GPIO) || defined(CONFIG_KEYBOARD_GPIO_MODULE)
	&ls1x_gpio_keys,
#endif
#if defined(CONFIG_LEDS_GPIO) || defined(CONFIG_LEDS_GPIO_MODULE)
	&leds,
#endif
};

static int __init ls1c_platform_init(void)
{
	int err;
	__raw_writel(0,LS1X_CBUS_FIFTHT0);//disable p0-p31 Function5 disable
	__raw_writel(0,LS1X_CBUS_FIFTHT1);//disable p31-p63 Function5 disable
	__raw_writel(0,LS1X_CBUS_FIFTHT2);//disable p63-p95 Function5 disable
	__raw_writel(0,LS1X_CBUS_FIFTHT3);//disable p96-p127 Function5 disable

	/* P0-P5 Function1-3 disable */
	__raw_writel(__raw_readl(LS1X_CBUS_FIRST0) & (~0x0000003f), LS1X_CBUS_FIRST0);//P0,P1,P2,P3,P4,P5 Function1 disable
	__raw_writel(__raw_readl(LS1X_CBUS_SECOND0) & (~0x0000003f), LS1X_CBUS_SECOND0);//P0,P1,P2,P3,P4,P5 Function2 disable
	__raw_writel(__raw_readl(LS1X_CBUS_THIRD0) & (~0x0000003f), LS1X_CBUS_THIRD0); //P0,P1,P2,P3,P4,P5 Function3 disable

	gpio_func(2,74);//rx0
	gpio_func(2,75);//tx0
	gpio_func(4,2);//rx1
	gpio_func(4,3);//tx1
//	gpio_func(4,4);//rx2
//	gpio_func(4,5);//tx2
	gpio_func(2,36);//rx2 console
	gpio_func(2,37);//tx2 console
	gpio_func(4,0);//rx3
	gpio_func(4,1);//tx3
	gpio_func(5,58);//rx4
	gpio_func(5,59);//tx4
	gpio_func(5,60);//rx5
	gpio_func(5,61);//tx5
	gpio_func(5,62);//rx6
	gpio_func(5,63);//tx6
	gpio_func(5,64);//rx7
	gpio_func(5,65);//tx7
	gpio_func(5,66);//rx8
	gpio_func(5,67);//tx8
	gpio_func(5,68);//rx9
	gpio_func(5,69);//tx9
	gpio_func(5,70);//rx10
	gpio_func(5,71);//tx10
	gpio_func(5,72);//rx11
	gpio_func(5,73);//tx11

	ls1x_serial_setup(&ls1x_uart_pdev);
#ifdef CONFIG_LS1X_FB0
	/* 使能LCD控制器 */
	__raw_writel(__raw_readl(LS1X_MUX_CTRL0) & ~LCD_SHUT, LS1X_MUX_CTRL0);
#endif
	/* 使能NAND控制器 */
	__raw_writel(__raw_readl(LS1X_MUX_CTRL0) & ~DMA0_SHUT, LS1X_MUX_CTRL0);
//	__raw_writel(__raw_readl(LS1X_MUX_CTRL0) & ~DMA1_SHUT, LS1X_MUX_CTRL0);
//	__raw_writel(__raw_readl(LS1X_MUX_CTRL0) & ~DMA2_SHUT, LS1X_MUX_CTRL0);
//	__raw_writel(__raw_readl(LS1X_MUX_CTRL0) & ~ECC_SHUT, LS1X_MUX_CTRL0);
	__raw_writel(__raw_readl(LS1X_MUX_CTRL0) & ~AC97_SHUT, LS1X_MUX_CTRL0);
	/* 使能SPI0控制器 */
	__raw_writel(__raw_readl(LS1X_MUX_CTRL0) & ~SPI0_SHUT, LS1X_MUX_CTRL0);
	spi_register_board_info(ls1x_spi0_devices, ARRAY_SIZE(ls1x_spi0_devices));
	/* 使能GMAC0控制器 */
	__raw_writel(__raw_readl(LS1X_MUX_CTRL0) & ~GMAC_SHUT, LS1X_MUX_CTRL0);
#ifdef CONFIG_USB_GADGET_SNPS_DWC_OTG
	/* 使能OTG控制器 */
	__raw_writel(__raw_readl(LS1X_MUX_CTRL0) & ~USBOTG_SHUT, LS1X_MUX_CTRL0);
#endif
	ls1x_i2c_setup();

//关闭iis
	__raw_writel(__raw_readl(LS1X_MUX_CTRL0) | I2S_SHUT, LS1X_MUX_CTRL0);
	/* 轮询方式或中断方式探测card的插拔 */
	gpio_request(DETECT_GPIO, "MMC_SPI GPIO detect");
	gpio_direction_input(DETECT_GPIO);		/* 输入使能 */

#if defined(CONFIG_PWM_LS1X_PWM2) || defined(CONFIG_PWM_LS1X_PWM3)
	__raw_writel(__raw_readl(LS1X_CBUS_FIRST1) & (~0x00300000), LS1X_CBUS_FIRST1); //52,53 fun4 pwm
	__raw_writel(__raw_readl(LS1X_CBUS_SECOND1) & (~0x00300000), LS1X_CBUS_SECOND1); 
	__raw_writel(__raw_readl(LS1X_CBUS_THIRD1) & (~0x00300000), LS1X_CBUS_THIRD1);
	__raw_writel(__raw_readl(LS1X_CBUS_FOURTHT1) | 0x00300000, LS1X_CBUS_FOURTHT1);
	__raw_writel(__raw_readl(LS1X_CBUS_FIFTHT1) & (~0x00300000), LS1X_CBUS_THIRD1);
#endif

#ifdef CONFIG_SENSORS_LS1X
	/* 使能ADC控制器 */
//	__raw_writel(__raw_readl(LS1X_MUX_CTRL0) & ~ADC_SHUT, LS1X_MUX_CTRL0);
	ls1x_hwmon_set_platdata(&bast_hwmon_info);
#endif

	err = platform_add_devices(ls1c_platform_devices,
				   ARRAY_SIZE(ls1c_platform_devices));
	return err;
}

arch_initcall(ls1c_platform_init);

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
//		.mask_flags = MTD_CAP_ROM
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

#ifdef CONFIG_GPIO_PCA953X
#include <linux/platform_data/pca953x.h>
#define PCA9555_GPIO_BASE 170
#define PCA9555_IRQ_BASE 170
#define PCA9555_GPIO_IRQ 31

#define LOCKER_TS	(PCA9555_GPIO_BASE+12)
#define LOCKER_BL	(PCA9555_GPIO_BASE+13)

static int pca9555_setup(struct i2c_client *client,
			       unsigned gpio_base, unsigned ngpio,
			       void *context)
{
	gpio_request(PCA9555_GPIO_IRQ, "pca9555 gpio irq");
	gpio_direction_input(PCA9555_GPIO_IRQ);
	return 0;
}

static struct pca953x_platform_data i2c_pca9555_platdata = {
	.gpio_base	= PCA9555_GPIO_BASE, /* Start directly after the CPU's GPIO */
	.irq_base = PCA9555_IRQ_BASE,
//	.invert		= 0, /* Do not invert */
	.setup		= pca9555_setup,
};

#if defined(CONFIG_LEDS_GPIO) || defined(CONFIG_LEDS_GPIO_MODULE)
#include <linux/leds.h>
struct gpio_led pca9555_gpio_leds[] = {
	{
		.name			= "locker_ts",
		.gpio			= LOCKER_TS,
		.active_low		= 1,
		.default_trigger	= NULL,
		.default_state	= LEDS_GPIO_DEFSTATE_ON,
	},
	{
		.name			= "locker_bl",
		.gpio			= LOCKER_BL,
		.active_low		= 0,
		.default_trigger	= NULL,
		.default_state	= LEDS_GPIO_DEFSTATE_ON,
	},
};

static struct gpio_led_platform_data pca9555_gpio_led_info = {
	.leds		= pca9555_gpio_leds,
	.num_leds	= ARRAY_SIZE(pca9555_gpio_leds),
};

static struct platform_device pca9555_leds = {
	.name	= "leds-gpio",
	.id	= 0,
	.dev	= {
		.platform_data	= &pca9555_gpio_led_info,
	}
};
#endif //#if defined(CONFIG_LEDS_GPIO) || defined(CONFIG_LEDS_GPIO_MODULE)
#endif //#ifdef CONFIG_GPIO_PCA953X

#if defined(CONFIG_I2C_OCORES) || defined(CONFIG_I2C_LS1X)
#include <linux/i2c.h>
static struct i2c_board_info ls1x_i2c0_board_info[] = {
#ifdef CONFIG_GPIO_PCA953X
	{
		I2C_BOARD_INFO("pca9555", 0x20),
		.irq = LS1X_GPIO_FIRST_IRQ + PCA9555_GPIO_IRQ,
		.platform_data = &i2c_pca9555_platdata,
	},
#endif
#ifdef CONFIG_SND_SOC_ES8328
	{
		I2C_BOARD_INFO("es8328", 0x10),
	},
#endif
#ifdef CONFIG_RTC_DRV_SD2068
	{
		I2C_BOARD_INFO("sd2068", 0x32),
	},
#endif
};
#endif

#ifdef CONFIG_I2C_OCORES
#include <linux/i2c-ocores.h>
struct ocores_i2c_platform_data ocores_i2c0_data = {
	.reg_shift = 0,
	.devices	= ls1x_i2c0_board_info, /* optional table of devices */
	.num_devices	= ARRAY_SIZE(ls1x_i2c0_board_info), /* table size */
};

static void ls1x_i2c_setup(void)
{
	struct ocores_i2c_platform_data *ocores_i2c_data;
	struct clk *clk;

	clk = clk_get(NULL, "apb_clk");
	if (IS_ERR(clk))
		panic("unable to get apb clock, err=%ld", PTR_ERR(clk));

	ocores_i2c_data = &ocores_i2c0_data;
	ocores_i2c_data->clock_khz = clk_get_rate(clk) / 1000;
//	ocores_i2c_data = &ocores_i2c1_data;
//	ocores_i2c_data->clock_khz = clk_get_rate(clk) / 1000;
}
#endif

#ifdef CONFIG_I2C_LS1X
#include <linux/i2c-ls1x.h>
struct ls1x_i2c_platform_data ls1x_i2c0_data = {
	.bus_clock_hz = 100000, /* i2c bus clock in Hz */
	.devices	= ls1x_i2c0_board_info, /* optional table of devices */
	.num_devices	= ARRAY_SIZE(ls1x_i2c0_board_info), /* table size */
};

struct ls1x_i2c_platform_data ls1x_i2c1_data = {
	.bus_clock_hz = 100000, /* i2c bus clock in Hz */
//	.devices	= ls1x_i2c1_board_info, /* optional table of devices */
//	.num_devices	= ARRAY_SIZE(ls1x_i2c1_board_info), /* table size */
};

struct ls1x_i2c_platform_data ls1x_i2c2_data = {
	.bus_clock_hz = 100000, /* i2c bus clock in Hz */
//	.devices	= ls1x_i2c2_board_info, /* optional table of devices */
//	.num_devices	= ARRAY_SIZE(ls1x_i2c2_board_info), /* table size */
};
#endif

#ifdef CONFIG_BACKLIGHT_GPIO
#define GPIO_BACKLIGHT_CTRL	180 //PCA9555_GPIO_BASE+10
#include <linux/platform_data/gpio_backlight.h>
static struct gpio_backlight_platform_data gpio_backlight_data = {
	.fbdev = &ls1x_fb0_pdev.dev,
	.gpio = GPIO_BACKLIGHT_CTRL,
	.def_value = 1,
	.name = "backlight",
};

static struct platform_device ls1x_bl_pdev = {
	.name			= "gpio-backlight",
	.dev = {
		.platform_data	= &gpio_backlight_data,
	},
};
#endif //#ifdef CONFIG_BACKLIGHT_GPIO

#if defined(CONFIG_KEYBOARD_GPIO) || defined(CONFIG_KEYBOARD_GPIO_MODULE)
#include <linux/input.h>
#include <linux/gpio_keys.h>
static struct gpio_keys_button ls1x_gpio_keys_buttons[] = {
	 {
		.code		= KEY_0,
		.gpio		= 170,	/* PCA9555_GPIO_BASE+0 */
		.active_low	= 1,
		.desc		= "0",
		.wakeup		= 1,
		.debounce_interval	= 10, /* debounce ticks interval in msecs */
	},
	{
		.code		= KEY_1,
		.gpio		= 171,	/* PCA9555_GPIO_BASE+1 */
		.active_low	= 1,
		.desc		= "1",
		.wakeup		= 1,
		.debounce_interval	= 10, /* debounce ticks interval in msecs */
	},
	{
		.code		= KEY_2,
		.gpio		= 172,	/* PCA9555_GPIO_BASE+2 */
		.active_low	= 1,
		.desc		= "2",
		.wakeup		= 1,
		.debounce_interval	= 10, /* debounce ticks interval in msecs */
	},
	{
		.code		= KEY_3,
		.gpio		= 173,	/* PCA9555_GPIO_BASE+3 */
		.active_low	= 1,
		.desc		= "3",
		.wakeup		= 1,
		.debounce_interval	= 10, /* debounce ticks interval in msecs */
	},
	{
		.code		= KEY_4,
		.gpio		= 174,	/* PCA9555_GPIO_BASE+4 */
		.active_low	= 1,
		.desc		= "4",
		.wakeup		= 1,
		.debounce_interval	= 10, /* debounce ticks interval in msecs */
	},
	{
		.code		= KEY_5,
		.gpio		= 175,	/* PCA9555_GPIO_BASE+5 */
		.active_low	= 1,
		.desc		= "5",
		.wakeup		= 1,
		.debounce_interval	= 10, /* debounce ticks interval in msecs */
	},
	{
		.code		= KEY_6,
		.gpio		= 176,	/* PCA9555_GPIO_BASE+6 */
		.active_low	= 1,
		.desc		= "6",
		.wakeup		= 1,
		.debounce_interval	= 10, /* debounce ticks interval in msecs */
	},
	{
		.code		= KEY_ENTER,
		.gpio		= 177,	/* PCA9555_GPIO_BASE+7 */
		.active_low	= 1,
		.desc		= "ENTER",
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

#if defined(CONFIG_MMC_LS1X)
#include <linux/mmc/host.h>
#include <mci.h>
#define SDIO_WP_GPIO  32
#define SDIO_DETECT_GPIO  84
/* 轮询方式探测card的插拔 */
static int ls1x_sdio_get_ro(struct device *dev)
{
	return gpio_get_value(SDIO_WP_GPIO);
}

static int ls1x_sdio_get_cd(struct device *dev)
{
	return !gpio_get_value(SDIO_DETECT_GPIO);
}

struct ls1x_mci_pdata ls1x_sdio_parts = {
	/* 中断方式方式探测card的插拔 */
//	.init = ls1x_mmc_init,
//	.exit = ls1x_mmc_exit,
//	.detect_delay = 1200,	/* msecs */

	/* 轮询方式方式探测card的插拔 */
	.get_ro = ls1x_sdio_get_ro,
	.get_cd = ls1x_sdio_get_cd,
	.caps = MMC_CAP_NEEDS_POLL,

//	.max_clk = 17000000, /* 部分sd卡可能会出现读写错误，可以尝试降低提供给sd卡的频率 */
};
#endif //CONFIG_MMC_LS1X

#ifdef CONFIG_CAN_SJA1000_PLATFORM
#include <linux/can/platform/sja1000.h>
static void ls1x_can_setup(void)
{
	struct sja1000_platform_data *sja1000_pdata;
	struct clk *clk;

	clk = clk_get(NULL, "apb_clk");
	if (IS_ERR(clk))
		panic("unable to get apb clock, err=%ld", PTR_ERR(clk));

	#ifdef CONFIG_LS1X_CAN0
	sja1000_pdata = &ls1x_sja1000_platform_data_0;
	sja1000_pdata->osc_freq = clk_get_rate(clk);
	#endif
	#ifdef CONFIG_LS1X_CAN1
	sja1000_pdata = &ls1x_sja1000_platform_data_1;
	sja1000_pdata->osc_freq = clk_get_rate(clk);
	#endif

	/* 设置复用关系 can0 gpio54/55 */
	__raw_writel(__raw_readl(LS1X_CBUS_FIRST1) & (~0x00c00000), LS1X_CBUS_FIRST1);
	__raw_writel(__raw_readl(LS1X_CBUS_SECOND1) & (~0x00c00000), LS1X_CBUS_SECOND1);
	__raw_writel(__raw_readl(LS1X_CBUS_THIRD1) | 0x00c00000, LS1X_CBUS_THIRD1);
	__raw_writel(__raw_readl(LS1X_CBUS_FOURTHT1) & (~0x00c00000), LS1X_CBUS_FOURTHT1);

	/* 使能can0控制器 */
	__raw_writel(__raw_readl(LS1X_MUX_CTRL0) & (~CAN0_SHUT), LS1X_MUX_CTRL0);
}
#endif //#ifdef CONFIG_CAN_SJA1000_PLATFORM

#ifdef CONFIG_LEDS_PWM
#include <linux/pwm.h>
#include <linux/leds_pwm.h>
static struct pwm_lookup pwm_lookup[] = {
	/* LEDB -> PMU_STAT */
	PWM_LOOKUP("ls1x-pwm.0", 0, "leds_pwm", "ls1x_pwm_led0",
			7812500, PWM_POLARITY_NORMAL),
	PWM_LOOKUP("ls1x-pwm.1", 0, "leds_pwm", "ls1x_pwm_led1",
			7812500, PWM_POLARITY_NORMAL),
/*	PWM_LOOKUP("ls1x-pwm.2", 0, "leds_pwm", "ls1x_pwm_led2",
			7812500, PWM_POLARITY_NORMAL),
	PWM_LOOKUP("ls1x-pwm.3", 0, "leds_pwm", "ls1x_pwm_led3",
			7812500, PWM_POLARITY_NORMAL),*/
};

static struct led_pwm ls1x_pwm_leds[] = {
	{
		.name		= "ls1x_pwm_led0",
		.max_brightness	= 255,
		.pwm_period_ns	= 7812500,
	},
	{
		.name		= "ls1x_pwm_led1",
		.max_brightness	= 255,
		.pwm_period_ns	= 7812500,
	},
/*	{
		.name		= "ls1x_pwm_led2",
		.max_brightness	= 255,
		.pwm_period_ns	= 7812500,
	},
	{
		.name		= "ls1x_pwm_led3",
		.max_brightness	= 255,
		.pwm_period_ns	= 7812500,
	},*/
};

static struct led_pwm_platform_data ls1x_pwm_data = {
	.num_leds	= ARRAY_SIZE(ls1x_pwm_leds),
	.leds		= ls1x_pwm_leds,
};

static struct platform_device ls1x_leds_pwm = {
	.name	= "leds_pwm",
	.id		= -1,
	.dev	= {
		.platform_data = &ls1x_pwm_data,
	},
};
#endif //#ifdef CONFIG_LEDS_PWM

#ifdef CONFIG_INPUT_GPIO_BEEPER
#include <linux/gpio.h>
#include <linux/gpio/machine.h>
static struct gpiod_lookup_table buzzer_gpio_table = {
	.dev_id = "gpio-beeper",	/* 注意 该值与ls1x_gpio_beeper.name相同 */
	.table = {
		GPIO_LOOKUP("ls1x-gpio1", 5, NULL, 0),	/* 使用第1组gpio的第5个引脚 注意"ls1x-gpio1"与gpio的label值相同 */
		{ },
	},
};

struct platform_device ls1x_gpio_beeper = {
	.name = "gpio-beeper",
	.id = -1,
};
#endif

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
#ifdef CONFIG_RTC_DRV_TOY_LOONGSON1CV2
	&ls1x_toy_pdev,
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
#ifdef CONFIG_LS1X_FB0
	&ls1x_fb0_pdev,
#endif
#ifdef CONFIG_I2C_OCORES
	&ls1x_i2c0_pdev,
#endif
#ifdef CONFIG_I2C_LS1X
	&ls1x_i2c0_pdev,
#endif
#ifdef CONFIG_SND_LS1X_SOC_I2S
	&ls1x_i2s_pdev,
#endif
#ifdef CONFIG_SND_LS1X_SOC
	&ls1x_pcm_pdev,
#endif
#ifdef CONFIG_GPIO_PCA953X
#if defined(CONFIG_LEDS_GPIO) || defined(CONFIG_LEDS_GPIO_MODULE)
	&pca9555_leds,
#endif
#endif
#if defined(CONFIG_KEYBOARD_GPIO) || defined(CONFIG_KEYBOARD_GPIO_MODULE)
	&ls1x_gpio_keys,
#endif
#ifdef CONFIG_BACKLIGHT_GPIO
	&ls1x_bl_pdev,
#endif
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
#ifdef CONFIG_LEDS_PWM
	&ls1x_leds_pwm,
#endif
#ifdef CONFIG_INPUT_GPIO_BEEPER
	&ls1x_gpio_beeper,
#endif
#if defined(CONFIG_MMC_LS1X)
	&ls1x_sdio_pdev,
#endif
#ifdef CONFIG_CAN_SJA1000_PLATFORM
#ifdef CONFIG_LS1X_CAN0
	&ls1x_sja1000_0,
#endif
#ifdef CONFIG_LS1X_CAN1
	&ls1x_sja1000_1,
#endif
#endif
#ifdef CONFIG_SENSORS_LS1X
	&ls1x_hwmon_pdev,
#endif
};

static int __init ls1c_platform_init(void)
{
	int err;

	ls1x_serial_setup(&ls1x_uart_pdev);
#if defined(CONFIG_SPI_LS1X_SPI0)
	spi_register_board_info(ls1x_spi0_devices, ARRAY_SIZE(ls1x_spi0_devices));
#endif
#ifdef CONFIG_CAN_SJA1000_PLATFORM
	ls1x_can_setup();
#endif
#ifdef CONFIG_I2C_OCORES
	ls1x_i2c_setup();
#endif

#if defined(CONFIG_MMC_LS1X)
	/* 轮询方式或中断方式探测card的插拔 */
	gpio_request(SDIO_WP_GPIO, "ls1x sdio wp");
	gpio_direction_input(SDIO_WP_GPIO);
	gpio_request(SDIO_DETECT_GPIO, "ls1x sdio detect");
	gpio_direction_input(SDIO_DETECT_GPIO);
#endif

	/* 根据需要修改复用引脚关系 */
#if defined(CONFIG_PWM_LS1X_PWM0) || defined(CONFIG_PWM_LS1X_PWM1)

#endif
#if defined(CONFIG_PWM_LS1X_PWM2) || defined(CONFIG_PWM_LS1X_PWM3)
	__raw_writel(__raw_readl(LS1X_CBUS_FIRST1) & (~0x00300000), LS1X_CBUS_FIRST1);
	__raw_writel(__raw_readl(LS1X_CBUS_SECOND1) & (~0x00300000), LS1X_CBUS_SECOND1);
	__raw_writel(__raw_readl(LS1X_CBUS_THIRD1) & (~0x00300000), LS1X_CBUS_THIRD1);
	__raw_writel(__raw_readl(LS1X_CBUS_FOURTHT1) | 0x00300000, LS1X_CBUS_FOURTHT1);
	__raw_writel(__raw_readl(LS1X_CBUS_FIFTHT1) & (~0x00300000), LS1X_CBUS_THIRD1);
#endif
#ifdef CONFIG_LEDS_PWM
	pwm_add_table(pwm_lookup, ARRAY_SIZE(pwm_lookup));
#endif
#ifdef CONFIG_INPUT_GPIO_BEEPER
	gpiod_add_lookup_table(&buzzer_gpio_table);
#endif

#ifdef CONFIG_SENSORS_LS1X
	ls1x_hwmon_set_platdata(&bast_hwmon_info);
#endif

	err = platform_add_devices(ls1c_platform_devices,
				   ARRAY_SIZE(ls1c_platform_devices));
	return err;
}

arch_initcall(ls1c_platform_init);

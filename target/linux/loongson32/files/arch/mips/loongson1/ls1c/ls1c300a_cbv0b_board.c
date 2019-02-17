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

#if defined(CONFIG_MMC_SPI) || defined(CONFIG_MMC_SPI_MODULE)
#include <linux/spi/mmc_spi.h>
#include <linux/mmc/host.h>
/* 开发板使用GPIO184引脚作为MMC/SD卡的插拔探测引脚 */
#define DETECT_GPIO  184	/* PCA9555_GPIO_BASE_0 + 14 */
#define WRITE_PROTECT_GPIO  185	/* 写保护探测 */ /* PCA9555_GPIO_BASE_0 + 15 */
static struct mmc_spi_platform_data mmc_spi __maybe_unused = {
	.flags = MMC_SPI_USE_CD_GPIO | MMC_SPI_USE_RO_GPIO,
	.cd_gpio = DETECT_GPIO,
	.ro_gpio = WRITE_PROTECT_GPIO,
//	.caps = MMC_CAP_NEEDS_POLL,
	.caps2 = MMC_CAP2_RO_ACTIVE_HIGH,
	.ocr_mask = MMC_VDD_32_33 | MMC_VDD_33_34, /* 3.3V only */
};
#endif  /* defined(CONFIG_MMC_SPI) || defined(CONFIG_MMC_SPI_MODULE) */

#ifdef CONFIG_TOUCHSCREEN_ADS7846
#include <linux/spi/ads7846.h>
#define ADS7846_GPIO_IRQ 180 /* 开发板触摸屏使用的外部中断 */ /* PCA9555_GPIO_BASE_0 + 10 */
static struct ads7846_platform_data ads_info __maybe_unused = {
	.model				= 7846,
	.vref_delay_usecs	= 1,
	.keep_vref_on		= 0,
	.settle_delay_usecs	= 20,
//	.x_plate_ohms		= 800,
	.pressure_min		= 0,
	.pressure_max		= 2048,
	.debounce_rep		= 3,
	.debounce_max		= 10,
	.debounce_tol		= 50,
//	.get_pendown_state	= ads7846_pendown_state,
	.get_pendown_state	= NULL,
	.gpio_pendown		= ADS7846_GPIO_IRQ,
	.filter_init		= NULL,
	.filter 			= NULL,
	.filter_cleanup 	= NULL,
};
#endif /* TOUCHSCREEN_ADS7846 */

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
#if defined(CONFIG_MMC_SPI) || defined(CONFIG_MMC_SPI_MODULE)
	{
		.modalias		= "mmc_spi",
		.bus_num 		= 0,
		.chip_select	= SPI0_CS2,
		.max_speed_hz	= 25000000,
		.platform_data	= &mmc_spi,
		.mode = SPI_MODE_3,
	},
#endif
#ifdef CONFIG_TOUCHSCREEN_ADS7846
	{
		.modalias = "ads7846",
		.platform_data = &ads_info,
		.bus_num 		= 0,
		.chip_select 	= SPI0_CS1,
		.max_speed_hz 	= 2500000,
		.mode 			= SPI_MODE_1,
		.irq			= ADS7846_GPIO_IRQ,
	},
#endif
};
#endif

#ifdef CONFIG_GPIO_PCA953X
#include <linux/platform_data/pca953x.h>
#define PCA9555_GPIO_BASE_0 170
#define PCA9555_IRQ_BASE_0 170
#define PCA9555_GPIO_IRQ_0 36

static int pca9555_setup_0(struct i2c_client *client,
			       unsigned gpio_base, unsigned ngpio,
			       void *context)
{
	gpio_request(PCA9555_GPIO_IRQ_0, "pca9555 gpio irq0");
	gpio_direction_input(PCA9555_GPIO_IRQ_0);

	gpio_request(gpio_base + 0, "mfrc531 irq");
	gpio_direction_input(gpio_base + 0);
	gpio_request(gpio_base + 1, "mfrc531 ncs");
	gpio_direction_output(gpio_base + 1, 1);
	gpio_request(gpio_base + 2, "mfrc531 rstpd");
	gpio_direction_output(gpio_base + 2, 0);

	return 0;
}

static struct pca953x_platform_data i2c_pca9555_platdata_0 = {
	.gpio_base	= PCA9555_GPIO_BASE_0, /* Start directly after the CPU's GPIO */
	.irq_base = PCA9555_IRQ_BASE_0,
//	.invert		= 0, /* Do not invert */
	.setup		= pca9555_setup_0,
};

#define PCA9555_GPIO_BASE_1 186
#define PCA9555_IRQ_BASE_1 186
#define PCA9555_GPIO_IRQ_1 31

static int pca9555_setup_1(struct i2c_client *client,
			       unsigned gpio_base, unsigned ngpio,
			       void *context)
{
	gpio_request(PCA9555_GPIO_IRQ_1, "pca9555 gpio irq1");
	gpio_direction_input(PCA9555_GPIO_IRQ_1);
	return 0;
}

static struct pca953x_platform_data i2c_pca9555_platdata_1 = {
	.gpio_base	= PCA9555_GPIO_BASE_1, /* Start directly after the CPU's GPIO */
	.irq_base = PCA9555_IRQ_BASE_1,
//	.invert		= 0, /* Do not invert */
	.setup		= pca9555_setup_1,
};

#define PCA9555_GPIO_BASE_2 202
#define PCA9555_IRQ_BASE_2 202
#define PCA9555_GPIO_IRQ_2 32

static int pca9555_setup_2(struct i2c_client *client,
			       unsigned gpio_base, unsigned ngpio,
			       void *context)
{
	gpio_request(PCA9555_GPIO_IRQ_2, "pca9555 gpio irq2");
	gpio_direction_input(PCA9555_GPIO_IRQ_2);
	return 0;
}

static struct pca953x_platform_data i2c_pca9555_platdata_2 = {
	.gpio_base	= PCA9555_GPIO_BASE_2, /* Start directly after the CPU's GPIO */
	.irq_base = PCA9555_IRQ_BASE_2,
//	.invert		= 0, /* Do not invert */
	.setup		= pca9555_setup_2,
};

#if defined(CONFIG_LEDS_GPIO) || defined(CONFIG_LEDS_GPIO_MODULE)
#include <linux/leds.h>
struct gpio_led pca9555_gpio_leds[] = {
	/* PCA9555 0 */
	{
		.name			= "soft_start_0",
		.gpio			= PCA9555_GPIO_BASE_0 + 3,
		.active_low		= 1,
		.default_trigger	= NULL,
		.default_state	= LEDS_GPIO_DEFSTATE_ON,
	},
	{
		.name			= "rs485",
		.gpio			= PCA9555_GPIO_BASE_0 + 4,
		.active_low		= 1,
		.default_trigger	= NULL,
		.default_state	= LEDS_GPIO_DEFSTATE_OFF,
	},
	{
		.name			= "lock_key",
		.gpio			= PCA9555_GPIO_BASE_0 + 7,
		.active_low		= 1,
		.default_trigger	= NULL,
		.default_state	= LEDS_GPIO_DEFSTATE_OFF,
	},
	{
		.name			= "usb_reset",
		.gpio			= PCA9555_GPIO_BASE_0 + 8,
		.active_low		= 0,
		.default_trigger	= NULL,
		.default_state	= LEDS_GPIO_DEFSTATE_ON,
	},
	{
		.name			= "wifi_rfen",
		.gpio			= PCA9555_GPIO_BASE_0 + 9,
		.active_low		= 1,
		.default_trigger	= NULL,
		.default_state	= LEDS_GPIO_DEFSTATE_OFF,
	},
	{
		.name			= "gsm_emerg_off",
		.gpio			= PCA9555_GPIO_BASE_0 + 12,
		.active_low		= 1,
		.default_trigger	= NULL,
		.default_state	= LEDS_GPIO_DEFSTATE_ON,
	},
	{
		.name			= "gsm_pwrkey",
		.gpio			= PCA9555_GPIO_BASE_0 + 13,
		.active_low		= 1,
		.default_trigger	= NULL,
		.default_state	= LEDS_GPIO_DEFSTATE_ON,
	},
	/* PCA9555 1 */
	{
		.name			= "led_red",
		.gpio			= PCA9555_GPIO_BASE_1 + 8,
		.active_low		= 0,
		.default_trigger	= "none",
		.default_state	= LEDS_GPIO_DEFSTATE_ON,
	},
	{
		.name			= "led_green",
		.gpio			= PCA9555_GPIO_BASE_1 + 9,
		.active_low		= 0,
		.default_trigger	= "heartbeat",
		.default_state	= LEDS_GPIO_DEFSTATE_ON,
	},
	{
		.name			= "led_blue",
		.gpio			= PCA9555_GPIO_BASE_1 + 10,
		.active_low		= 0,
		.default_trigger	= "none",
		.default_state	= LEDS_GPIO_DEFSTATE_ON,
	},
	{
		.name			= "soft_start_1",
		.gpio			= PCA9555_GPIO_BASE_1 + 13,
		.active_low		= 1,
		.default_trigger	= NULL,
		.default_state	= LEDS_GPIO_DEFSTATE_OFF,
	},
	/* PCA9555 2 */
	{
		.name			= "locker_bl",
		.gpio			= PCA9555_GPIO_BASE_2 + 1,
		.active_low		= 0,
		.default_trigger	= "none",
		.default_state	= LEDS_GPIO_DEFSTATE_ON,
	},
	{
		.name			= "audio_en",
		.gpio			= PCA9555_GPIO_BASE_2 + 11,
		.active_low		= 0,
		.default_trigger	= "none",
		.default_state	= LEDS_GPIO_DEFSTATE_OFF,
	},
	{
		.name			= "otg_vbus",
		.gpio			= PCA9555_GPIO_BASE_2 + 13,
		.active_low		= 0,
		.default_trigger	= "none",
		.default_state	= LEDS_GPIO_DEFSTATE_ON,
	},
	{
		.name			= "otg_id",
		.gpio			= PCA9555_GPIO_BASE_2 + 14,
		.active_low		= 0,
		.default_trigger	= "none",
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
		I2C_BOARD_INFO("pca9555", 0x26),
		.irq = LS1X_GPIO_FIRST_IRQ + PCA9555_GPIO_IRQ_0,
		.platform_data = &i2c_pca9555_platdata_0,
	},
	{
		I2C_BOARD_INFO("pca9555", 0x22),
		.irq = LS1X_GPIO_FIRST_IRQ + PCA9555_GPIO_IRQ_1,
		.platform_data = &i2c_pca9555_platdata_1,
	},
	{
		I2C_BOARD_INFO("pca9555", 0x23),
		.irq = LS1X_GPIO_FIRST_IRQ + PCA9555_GPIO_IRQ_2,
		.platform_data = &i2c_pca9555_platdata_2,
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
#define GPIO_BACKLIGHT_CTRL	197 //PCA9555_GPIO_BASE_1+11
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
#ifdef CONFIG_RTC_DRV_TOY_LOONGSON1
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

#ifdef CONFIG_SENSORS_LS1X
	ls1x_hwmon_set_platdata(&bast_hwmon_info);
#endif

	err = platform_add_devices(ls1c_platform_devices,
				   ARRAY_SIZE(ls1c_platform_devices));
	return err;
}

arch_initcall(ls1c_platform_init);

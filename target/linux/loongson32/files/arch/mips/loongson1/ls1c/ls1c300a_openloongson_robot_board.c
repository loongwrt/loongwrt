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
#if 0
//#if defined(CONFIG_PWM_LS1X)
struct pwm_device ls1x_pwm_list[] = {
	{ 0, 06, false },
	{ 1, 92, false },
	{ 2, 93, false },
	{ 3, 37, false },
};
#endif

#define CBUS_MUX(func, r) \
                ((void __iomem *)0xbfd011c0 + ((func -1) * 0x10) + (4*r))
#define CBUSS(func, r) \
                0x11c0 + ((func -1) * 0x10) + (4*r)
void cbus_dump(void) {
#ifdef CONFIG_LS1X_CBUS_DUMP
	int16_t i;
	char buff[500];
	printk(KERN_ERR "gpio  ==SHUT_CTRL=[%08x]==uart0fu=[%08x]==uart8fu=[%08x]=========\n",__raw_readl(LS1X_MUX_CTRL0),__raw_readl((void __iomem *)0xbfd00420),__raw_readl((void __iomem *)0xbfe4c904));
	printk(KERN_ERR "gpio  31-----0 63----32 95----64 127---96\n");

	for(i=0;i<5;i++) {
		printk(KERN_ERR "fun%d: %08x %08x %08x %08x\n",i+1,
__raw_readl((void __iomem *)0xbfd011c0+i*0x10),
__raw_readl((void __iomem *)0xbfd011c0+i*0x10+4),
__raw_readl((void __iomem *)0xbfd011c0+i*0x10+8),
__raw_readl((void __iomem *)0xbfd011c0+i*0x10+0xc));
		//printk(KERN_ERR buff);
	}
for(i=0;i<128;i++) {
sprintf(buff,"gpio%d:fun",i);
if(__raw_readl((void __iomem *)0xbfd011c0+(i/32)*4) & (1<<(i%32))) {
printk(KERN_ERR "%s1\n",buff);
continue;
}
if(__raw_readl((void __iomem *)0xbfd011d0+(i/32)*4) & (1<<(i%32))) {
printk(KERN_ERR "%s2\n",buff);
continue;
}
if(__raw_readl((void __iomem *)0xbfd011e0+(i/32)*4) & (1<<(i%32))) {
printk(KERN_ERR "%s3\n",buff);
continue;
}
if(__raw_readl((void __iomem *)0xbfd011f0+(i/32)*4) & (1<<(i%32))) {
printk(KERN_ERR "%s4\n",buff);
continue;
}
if(__raw_readl((void __iomem *)0xbfd01200+(i/32)*4) & (1<<(i%32))) {
printk(KERN_ERR "%s5\n",buff);
continue;
}
}

#endif// CONFIG_LS1X_CBUS_DUMP
}
void gpio_func(uint8_t func,uint8_t gpio_no) {
	uint8_t i,regno,regbit;
	uint32_t data=0;
	regno=(uint8_t) gpio_no/32;
	regbit=gpio_no % 32;
	//cbus_dump();
	data=__raw_readl(CBUS_MUX(func,regno));
	for(i=1;i<6;i++){
		if(i<func)
			__raw_writel(__raw_readl(CBUS_MUX(i,regno)) & ~(1<<regbit),CBUS_MUX(i,regno));
		else {
			__raw_writel(__raw_readl(CBUS_MUX(i,regno)) | (1<<regbit),CBUS_MUX(i,regno));
			break;
		}
	}
	printk(KERN_ERR "set gpio%d,fun%d,addr=[0xbfd0%x] %08x | %08x = %08x\n", gpio_no,func,CBUSS(func,regno),data,1<<regbit,__raw_readl(CBUS_MUX(func,regno)));
	//cbus_dump();
}

void gpio_func_dis(uint8_t func,uint8_t gpio_no) {
	uint8_t regno,regbit;
	regno=(uint8_t) gpio_no/32;
	regbit=gpio_no % 32;
	__raw_writel(__raw_readl(CBUS_MUX(func,regno)) & ~(1<<regbit),CBUS_MUX(func,regno));
	printk(KERN_ERR "disable gpio%d,fun%d\n", gpio_no,func);

}


#ifdef CONFIG_MTD_NAND_LS1X
#include <ls1x_nand.h>
static struct mtd_partition ls1x_nand_partitions[] = {
#ifdef CONFIG_LS1C_OPENLOONGSON_BOARD
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

#else
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
		.name	= "mini_rootfs",
		.offset	= MTDPART_OFS_APPEND,
		.size	= MTDPART_SIZ_FULL,
	},
#endif
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
#endif  /* defined(CONFIG_MMC_SPI) || defined(CONFIG_MMC_SPI_MODULE) */

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
	{
		.modalias		= "spidev",
		.bus_num 		= 0,
		.chip_select	= SPI0_CS1,
		.max_speed_hz	= 25000000,
		.mode = SPI_MODE_0,
	},
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
};
#endif

#if defined(CONFIG_I2C_LS1X)
#include <linux/i2c.h>
static struct i2c_board_info ls1x_i2c0_board_info[] = {
#ifdef CONFIG_PWM_PCA9685
	{
		I2C_BOARD_INFO("pca9685", 0x40),
	},
#endif
};

static struct i2c_board_info ls1x_i2c1_board_info[] = {
#ifdef CONFIG_PWM_PCA9685
	{
		I2C_BOARD_INFO("pca9685", 0x40),
	},
#endif
};

static struct i2c_board_info ls1x_i2c2_board_info[] = {
};
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

	/*
	 * PIN74    GPIO85    I2C_SDA0
	 * PIN75    GPIO86    I2C_SCL0
	 *
	 * PIN3    GPIO76    I2C_SDA1
	 * PIN6    GPIO77    I2C_SCL1
	 */
	gpio_func(4,50);//sda2
	gpio_func(4,51);//scl2
}
#endif

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
	/* 使能can0控制器 */
	__raw_writel(__raw_readl(LS1X_MUX_CTRL0) & (~CAN0_SHUT), LS1X_MUX_CTRL0);

	sja1000_pdata = &ls1x_sja1000_platform_data_0;
	sja1000_pdata->osc_freq = clk_get_rate(clk);
	#endif
	#ifdef CONFIG_LS1X_CAN1
	/* 使能can1控制器 */
	__raw_writel(__raw_readl(LS1X_MUX_CTRL0) & (~CAN1_SHUT), LS1X_MUX_CTRL0);

	sja1000_pdata = &ls1x_sja1000_platform_data_1;
	sja1000_pdata->osc_freq = clk_get_rate(clk);
	#endif

	/* 设置复用关系 can0 gpio54/55 */
	__raw_writel(__raw_readl(LS1X_CBUS_FIRST1) & (~0x00c00000), LS1X_CBUS_FIRST1);
	__raw_writel(__raw_readl(LS1X_CBUS_SECOND1) & (~0x00c00000), LS1X_CBUS_SECOND1);
	__raw_writel(__raw_readl(LS1X_CBUS_THIRD1) | 0x00c00000, LS1X_CBUS_THIRD1);
	__raw_writel(__raw_readl(LS1X_CBUS_FOURTHT1) & (~0x00c00000), LS1X_CBUS_FOURTHT1);
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
#ifdef CONFIG_MTD_NAND_LS1X
	&ls1x_nand_pdev,
#endif
#if defined(CONFIG_LS1X_GMAC0)
	&ls1x_eth0_pdev,
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
#ifdef CONFIG_USB_GADGET_SNPS_DWC_OTG
	&ls1x_otg_pdev,
#endif
#ifdef CONFIG_SPI_LS1X_SPI0
	&ls1x_spi0_pdev,
#endif
#ifdef CONFIG_LS1X_FB0
	&ls1x_fb0_pdev,
#endif
#ifdef CONFIG_I2C_LS1X
	&ls1x_i2c0_pdev,
	&ls1x_i2c1_pdev,
	&ls1x_i2c2_pdev,
#endif
#ifdef CONFIG_SND_LS1X_SOC_I2S
	&ls1x_i2s_pdev,
#endif
#ifdef CONFIG_SND_LS1X_SOC
	&ls1x_pcm_pdev,
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
#ifdef CONFIG_LS1C_OPENLOONGSON_BOARD
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

#endif //CONFIG_LS1C_OPENLOONGSON_BOARD

	ls1x_serial_setup(&ls1x_uart_pdev);
#ifdef CONFIG_LS1X_FB0
	/* 使能LCD控制器 */
	__raw_writel(__raw_readl(LS1X_MUX_CTRL0) & ~LCD_SHUT, LS1X_MUX_CTRL0);
#endif
#ifdef CONFIG_MTD_NAND_LS1X
	/* 使能NAND控制器 */
	__raw_writel(__raw_readl(LS1X_MUX_CTRL0) & ~DMA0_SHUT, LS1X_MUX_CTRL0);
//	__raw_writel(__raw_readl(LS1X_MUX_CTRL0) & ~DMA1_SHUT, LS1X_MUX_CTRL0);
//	__raw_writel(__raw_readl(LS1X_MUX_CTRL0) & ~DMA2_SHUT, LS1X_MUX_CTRL0);
//	__raw_writel(__raw_readl(LS1X_MUX_CTRL0) & ~ECC_SHUT, LS1X_MUX_CTRL0);
	__raw_writel(__raw_readl(LS1X_MUX_CTRL0) & ~AC97_SHUT, LS1X_MUX_CTRL0);
#endif
#if defined(CONFIG_SPI_LS1X_SPI0)
	/* 使能SPI0控制器 */
	__raw_writel(__raw_readl(LS1X_MUX_CTRL0) & ~SPI0_SHUT, LS1X_MUX_CTRL0);
	spi_register_board_info(ls1x_spi0_devices, ARRAY_SIZE(ls1x_spi0_devices));
#endif
#if defined(CONFIG_LS1X_GMAC0)
	/* 使能GMAC0控制器 */
	__raw_writel(__raw_readl(LS1X_MUX_CTRL0) & ~GMAC_SHUT, LS1X_MUX_CTRL0);
#endif
#ifdef CONFIG_USB_GADGET_SNPS_DWC_OTG
	/* 使能OTG控制器 */
	__raw_writel(__raw_readl(LS1X_MUX_CTRL0) & ~USBOTG_SHUT, LS1X_MUX_CTRL0);
#endif
#ifdef CONFIG_I2C_LS1X
	ls1x_i2c_setup();
#endif
#ifdef CONFIG_SND_LS1X_SOC_I2S
	/* 使能IIS控制器 */
	__raw_writel(__raw_readl(LS1X_MUX_CTRL0) & ~I2S_SHUT, LS1X_MUX_CTRL0);
//	__raw_writel(__raw_readl(LS1X_MUX_CTRL0) & ~DMA1_SHUT, LS1X_MUX_CTRL0);
//	__raw_writel(__raw_readl(LS1X_MUX_CTRL0) & ~DMA2_SHUT, LS1X_MUX_CTRL0);
#else
	__raw_writel(__raw_readl(LS1X_MUX_CTRL0) | I2S_SHUT, LS1X_MUX_CTRL0);
#endif
#ifdef CONFIG_CAN_SJA1000_PLATFORM
	ls1x_can_setup();
#endif

#if defined(CONFIG_MMC_SPI) || defined(CONFIG_MMC_SPI_MODULE)
	/* 轮询方式或中断方式探测card的插拔 */
	gpio_request(DETECT_GPIO, "MMC_SPI GPIO detect");
	gpio_direction_input(DETECT_GPIO);		/* 输入使能 */
#endif

	/* 根据需要修改复用引脚关系 */
#if defined(CONFIG_PWM_LS1X_PWM0) || defined(CONFIG_PWM_LS1X_PWM1)

#endif
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

#ifdef CONFIG_PWM_PCA9685
	/*
	 * GPIO75 & GPIO91 : out 0
	 */
	__raw_writel(__raw_readl(LS1X_GPIO_CFG2) | ((1 << 11) | (1 << 27)), LS1X_GPIO_CFG2);
	__raw_writel(__raw_readl(LS1X_GPIO_OE2) & ~((1 << 11) | (1 << 27)), LS1X_GPIO_OE2);
	__raw_writel(__raw_readl(LS1X_GPIO_OUT2) & ~((1 << 11) | (1 << 27)), LS1X_GPIO_OUT2);
#endif

	err = platform_add_devices(ls1c_platform_devices,
				   ARRAY_SIZE(ls1c_platform_devices));
	return err;
}

arch_initcall(ls1c_platform_init);

/*
 * Copyright (c) 2012 Tang, Haifeng <tanghaifeng-gz@loongson.cn>
 * Platform device support for GS232 SoCs.
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/resource.h>
#include <linux/serial_8250.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/delay.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <linux/spi/mmc_spi.h>
#include <linux/spi/spi_gpio.h>
#include <linux/mmc/host.h>
#include <linux/phy.h>
#include <linux/stmmac.h>
#include <linux/i2c.h>
#include <linux/videodev2.h>
#include <linux/input.h>
#include <linux/clk.h>

#include <video/ls1xfb.h>

#include <loongson1.h>
#include <irq.h>
#include <asm/gpio.h>
#include <asm-generic/sizes.h>
#include <media/soc_camera.h>
#include <media/soc_camera_platform.h>

#ifdef CONFIG_MTD_NAND_LS1X
#include <ls1x_nand.h>
static struct mtd_partition ls1x_nand_partitions[] = {
	{
		.name	= "kernel",
		.offset	= MTDPART_OFS_APPEND,
		.size	= 14*1024*1024,
	},  {
		.name	= "rootfs",
		.offset	= MTDPART_OFS_APPEND,
		.size	= 100*1024*1024,
	},  {
		.name	= "data",
		.offset	= MTDPART_OFS_APPEND,
		.size	= MTDPART_SIZ_FULL,
	},
};

static struct ls1x_nand_platform_data ls1x_nand_parts = {
	.parts		= ls1x_nand_partitions,
	.nr_parts	= ARRAY_SIZE(ls1x_nand_partitions),
};

static struct resource ls1x_nand_resources[] = {
	[0] = {
		.start          = LS1X_NAND_BASE,
		.end            = LS1X_NAND_BASE + SZ_16K - 1,
		.flags          = IORESOURCE_MEM,
	},
	[1] = {
		.start          = LS1X_DMA0_IRQ,
		.end            = LS1X_DMA0_IRQ,
		.flags          = IORESOURCE_IRQ,
	},
};

struct platform_device ls1x_nand_device = {
	.name	= "ls1x-nand",
	.id		= -1,
	.dev	= {
		.platform_data = &ls1x_nand_parts,
	},
	.num_resources	= ARRAY_SIZE(ls1x_nand_resources),
	.resource		= ls1x_nand_resources,
};
#endif //CONFIG_MTD_NAND_LS1X

#define LS1X_UART(_id)						\
	{							\
		.mapbase	= LS1X_UART ## _id ## _BASE,	\
		.irq		= LS1X_UART ## _id ## _IRQ,	\
		.iotype		= UPIO_MEM,			\
		.flags		= UPF_IOREMAP | UPF_FIXED_TYPE,	\
		.type		= PORT_16550A,			\
	}

static struct plat_serial8250_port ls1x_serial8250_port[] = {
	LS1X_UART(0),
	LS1X_UART(1),
	LS1X_UART(2),
	LS1X_UART(3),
	{},
};

struct platform_device ls1x_uart_device = {
	.name		= "serial8250",
	.id		= PLAT8250_DEV_PLATFORM,
	.dev		= {
		.platform_data = ls1x_serial8250_port,
	},
};

void __init ls1x_serial_setup(void)
{
	struct clk *clk;
	struct plat_serial8250_port *p;

	clk = clk_get(NULL, "apb");
	if (IS_ERR(clk))
		panic("unable to get apb clock, err=%ld", PTR_ERR(clk));

	for (p = ls1x_serial8250_port; p->flags != 0; ++p)
		p->uartclk = clk_get_rate(clk);

	__raw_writel(0,LS1X_CBUS_FIFTHT0);//disable p0-p31 Function5 disable
	__raw_writel(0,LS1X_CBUS_FIFTHT1);//disable p31-p63 Function5 disable
	__raw_writel(0,LS1X_CBUS_FIFTHT2);//disable p63-p95 Function5 disable
	__raw_writel(0,LS1X_CBUS_FIFTHT3);//disable p96-p127 Function5 disable
	__raw_writel(__raw_readl(LS1X_CBUS_FOURTHT1) & (~0x030C3000), LS1X_CBUS_FOURTHT1);//P44,P45,P50,P51,P56,P57 Function4 disable
	__raw_writel(__raw_readl(LS1X_CBUS_FOURTHT0) & (~0x00780000), LS1X_CBUS_FOURTHT0);//P19,P20 Function4 disable
	__raw_writel(__raw_readl(LS1X_CBUS_THIRD3) & (~0x00000006), LS1X_CBUS_THIRD3);//P97,P98 Function3 disable


	/* P0-P5 Function1-3 disable */
	__raw_writel(__raw_readl(LS1X_CBUS_FIRST0) & (~0x0000003f), LS1X_CBUS_FIRST0);//P0,P1,P2,P3,P4,P5 Function1 disable
	__raw_writel(__raw_readl(LS1X_CBUS_SECOND0) & (~0x0000003f), LS1X_CBUS_SECOND0);//P0,P1,P2,P3,P4,P5 Function1 disable
	__raw_writel(__raw_readl(LS1X_CBUS_THIRD0) & (~0x0000003f), LS1X_CBUS_THIRD0); //P0,P1,P2,P3,P4,P5 Function1 disable

	/* UART0 P74-RX0,P75-TX0 */ 
	__raw_writel(__raw_readl(LS1X_CBUS_THIRD0) & (~0x01800000), LS1X_CBUS_THIRD0);//P23,P24 Function3 disable
	__raw_writel(__raw_readl(LS1X_CBUS_THIRD3) & (~0x00000018), LS1X_CBUS_THIRD3);//P99,P100 Function3 disable
	__raw_writel(__raw_readl(LS1X_CBUS_FIRST2) & (~0x00003000), LS1X_CBUS_FIRST2);//P76,P77 Function1 disable
	__raw_writel(__raw_readl(LS1X_CBUS_SECOND2) | 0x00000C00, LS1X_CBUS_SECOND2);//P74,P75 Function2 enable

	
	/* UART1 P2-RX1 P3-TX1 */ 
	__raw_writel(__raw_readl(LS1X_CBUS_FIRST0) & (~0x00060000), LS1X_CBUS_FIRST0); //P17,P18 Function1 disable
	__raw_writel(__raw_readl(LS1X_CBUS_FIRST3) & (~0x00000060), LS1X_CBUS_FIRST3); //P101,P102 Function1 disable
	__raw_writel(__raw_readl(LS1X_CBUS_SECOND1) & (~0x00000300), LS1X_CBUS_SECOND1);//P40,P41 Function2 disable
	__raw_writel(__raw_readl(LS1X_CBUS_SECOND2) & (~0x00003000), LS1X_CBUS_SECOND2); //P76,P77 Function2 disable
	__raw_writel(__raw_readl(LS1X_CBUS_FOURTHT0) | 0x0000000c, LS1X_CBUS_FOURTHT0);//P2-RX1,P3-TX1  Function4 enable

	/* UART2  P35-RX2 P36-TX2 */
	__raw_writel(__raw_readl(LS1X_CBUS_SECOND1) & (~0x00000030), LS1X_CBUS_SECOND1);//P42,P43 Function2 disable
	__raw_writel(__raw_readl(LS1X_CBUS_THIRD0) & (~0x18000000), LS1X_CBUS_THIRD0);  //P27,P28 Function3 disable
	__raw_writel(__raw_readl(LS1X_CBUS_THIRD3) & (~0x00000180), LS1X_CBUS_THIRD3);  //P103,P104 Function3 disable
	__raw_writel(__raw_readl(LS1X_CBUS_FIRST1) & (~0x00000030), LS1X_CBUS_FIRST1);//P36,P37 Function1 disable
//	__raw_writel(__raw_readl(LS1X_CBUS_FOURTHT0) | 0x00000030, LS1X_CBUS_FOURTHT0); //P4-RX0,P5-TX0 Function4 enable 
//	__raw_writel(__raw_readl(LS1X_CBUS_SECOND0) | 0x00000030, LS1X_CBUS_SECOND0); //P4-PWM0, P4=PWM1 Function2 enable
	__raw_writel(__raw_readl(LS1X_CBUS_SECOND1) | 0x00000030, LS1X_CBUS_SECOND1);//P36-RX2,P37-TX2 Function2 , this is pmon uart port 
	

	/* UART3 P0-RX3 P1-TX3 */
	__raw_writel(__raw_readl(LS1X_CBUS_SECOND0) & (~0x00060000), LS1X_CBUS_SECOND0); //P17,P18 Function2 disable
	__raw_writel(__raw_readl(LS1X_CBUS_SECOND1) & (~0x00003006), LS1X_CBUS_SECOND1); //P33,P34,P44,P45 Function2 disable
	__raw_writel(__raw_readl(LS1X_CBUS_FOURTHT0) | 0x00000003, LS1X_CBUS_FOURTHT0); //P0-RX3,P1-TX3 Function4 enable
}

/* USB OHCI */
#ifdef CONFIG_USB_OHCI_HCD_PLATFORM
#include <linux/usb/ohci_pdriver.h>
static u64 ls1x_ohci_dmamask = DMA_BIT_MASK(32);

static struct resource ls1x_ohci_resources[] = {
	[0] = {
		.start	= LS1X_OHCI_BASE,
		.end	= LS1X_OHCI_BASE + SZ_32K - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= LS1X_OHCI_IRQ,
		.flags	= IORESOURCE_IRQ,
	},
};

static struct usb_ohci_pdata ls1x_ohci_pdata = {
};

struct platform_device ls1x_ohci_device = {
	.name		= "ohci-platform",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(ls1x_ohci_resources),
	.resource	= ls1x_ohci_resources,
	.dev		= {
		.dma_mask = &ls1x_ohci_dmamask,
		.platform_data = &ls1x_ohci_pdata,
	},
};
#endif // CONFIG_USB_OHCI_HCD_PLATFORM

/* EHCI */
#ifdef CONFIG_USB_EHCI_HCD_LS1X
static u64 ls1x_ehci_dma_mask = DMA_BIT_MASK(32);
static struct resource ls1x_ehci_resources[] = { 
	[0] = {
		.start          = LS1X_EHCI_BASE,
		.end            = LS1X_EHCI_BASE + SZ_32K - 1,
		.flags          = IORESOURCE_MEM,
	},
	[1] = {
		.start          = LS1X_EHCI_IRQ,
		.end            = LS1X_EHCI_IRQ,
		.flags          = IORESOURCE_IRQ,
	},
};

static struct platform_device ls1x_ehci_device = {
	.name           = "ls1x-ehci",
	.id             = 0,
	.dev = {
		.dma_mask = &ls1x_ehci_dma_mask,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
	},
	.num_resources  = ARRAY_SIZE(ls1x_ehci_resources),
	.resource       = ls1x_ehci_resources,
};
#endif //#ifdef CONFIG_USB_EHCI_HCD_LS1X

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


/* watchdog */
#ifdef CONFIG_LS1X_WDT
static struct resource ls1x_wdt_resource[] = {
	[0]={
		.start      = LS1X_WDT_BASE,
		.end        = (LS1X_WDT_BASE + 0x8),
		.flags      = IORESOURCE_MEM,
	},
};

static struct platform_device ls1x_wdt_device = {
	.name       = "ls1x-wdt",
	.id         = -1,
	.num_resources  = ARRAY_SIZE(ls1x_wdt_resource),
	.resource   = ls1x_wdt_resource,
};
#endif //#ifdef CONFIG_LS1X_WDT

//GMAC(is MAC, in fact) in LS1C300B
#ifdef CONFIG_STMMAC_ETH
void __init ls1x_gmac_setup(void)
{
	u32 x;
	x = __raw_readl(LS1X_MUX_CTRL1);
	x &= ~PHY_INTF_SELI;
	#if defined(CONFIG_LS1X_GMAC0_RMII)
	x |= 0x4 << PHY_INTF_SELI_SHIFT;
	#endif
	__raw_writel(x, LS1X_MUX_CTRL1);

	x = __raw_readl(LS1X_MUX_CTRL0);
	__raw_writel(x & (~GMAC_SHUT), LS1X_MUX_CTRL0);

#if defined(CONFIG_LS1X_GMAC0_RMII)
	__raw_writel(0x400, (void __iomem *)KSEG1ADDR(LS1X_GMAC0_BASE + 0x14));
#endif
	__raw_writel(0xe4b, (void __iomem *)KSEG1ADDR(LS1X_GMAC0_BASE + 0x10));
}

#ifdef CONFIG_LS1X_GMAC0
static struct resource ls1x_mac0_resources[] = {
	[0] = {
		.start  = LS1X_GMAC0_BASE,
		.end    = LS1X_GMAC0_BASE + SZ_8K - 1,
		.flags  = IORESOURCE_MEM,
	},
	[1] = {
		.name   = "macirq",
		.start  = LS1X_GMAC0_IRQ,
		.flags  = IORESOURCE_IRQ,
	},
};

static struct plat_stmmacenet_data ls1x_mac0_data = {
	.bus_id = 0,
	.pbl = 0,
	.has_gmac = 1,
	.enh_desc = 0,
};

struct platform_device ls1x_gmac0_mac = {
	.name           = "stmmaceth",
	.id             = 0,
	.num_resources  = ARRAY_SIZE(ls1x_mac0_resources),
	.resource       = ls1x_mac0_resources,
	.dev            = {
		.platform_data = &ls1x_mac0_data,
	},
};

static struct plat_stmmacphy_data  phy0_private_data = {
	.bus_id = 0,
#ifdef CONFIG_RTL8305SC
	.phy_addr = 4,
#else
	.phy_addr = -1, /* Auto Detect */
#endif
	.phy_mask = 0,
	.interface = PHY_INTERFACE_MODE_RMII,
	
};

struct platform_device ls1x_gmac0_phy = {
	.name = "stmmacphy",
	.id = 0,
	.num_resources = 1,
	.resource = (struct resource[]){
		{
			.name = "phyirq",
			.start = PHY_POLL,
			.end = PHY_POLL,
			.flags = IORESOURCE_IRQ,
		},
	},
	.dev = {
		.platform_data = &phy0_private_data,
	},
};
#endif //#ifdef CONFIG_LS1X_GMAC0
#endif //#ifdef CONFIG_STMMAC_ETH

#ifdef CONFIG_MTD_M25P80
static struct mtd_partition partitions[] = {
	[0] = {
		.name		= "pmon",
		.offset	= MTDPART_OFS_APPEND,
		.size	= MTDPART_SIZ_FULL,
	},
};

static struct flash_platform_data flash __maybe_unused = {
	.name		= "spi-flash",
	.parts		= partitions,
	.nr_parts	= ARRAY_SIZE(partitions),
	.type		= "w25x40",
};
#endif /* CONFIG_MTD_M25P80 */

#ifdef CONFIG_SPI_LS1X_SPI0
#include <linux/spi/spi_ls1x.h>
static struct spi_board_info ls1x_spi0_devices[] = {
#ifdef CONFIG_MTD_M25P80
	{
		.modalias	= "m25p80",
		.bus_num 		= 0,
		.chip_select	= SPI0_CS0,
		.max_speed_hz	= 60000000,
		.platform_data	= &flash,
	},
#endif
};

static struct resource ls1x_spi0_resource[] = {
	[0]={
		.start	= LS1X_SPI0_BASE,
		.end	= LS1X_SPI0_BASE + SZ_16K - 1,
		.flags	= IORESOURCE_MEM,
	},
#if defined(CONFIG_SPI_IRQ_MODE)
	[1]={
		.start	= LS1X_SPI0_IRQ,
		.end	= LS1X_SPI0_IRQ,
		.flags	= IORESOURCE_IRQ,
	},
#endif
};

#ifdef CONFIG_SPI_CS_USED_GPIO
static int spi0_gpios_cs[] =
	{ 81, 82, 83, 84 };
#endif

static struct ls1x_spi_platform_data ls1x_spi0_platdata = {
#ifdef CONFIG_SPI_CS_USED_GPIO
	.gpio_cs_count = ARRAY_SIZE(spi0_gpios_cs),
	.gpio_cs = spi0_gpios_cs,
#elif CONFIG_SPI_CS
	.cs_count = SPI0_CS3 + 1,
#endif
};

static struct platform_device ls1x_spi0_device = {
	.name		= "spi_ls1x",
	.id 		= 0,
	.num_resources	= ARRAY_SIZE(ls1x_spi0_resource),
	.resource	= ls1x_spi0_resource,
	.dev		= {
		.platform_data	= &ls1x_spi0_platdata,//&ls1x_spi_devices,
	},
};
#endif //#ifdef CONFIG_SPI_LS1X_SPI0

#ifdef CONFIG_SPI_LS1X_SPI1 /* SPI1 Controler */
#include <linux/spi/spi_ls1x.h>
static struct spi_board_info ls1x_spi1_devices[] = {
};

static struct resource ls1x_spi1_resource[] = {
	[0]={
		.start	= LS1X_SPI1_BASE,
		.end	= LS1X_SPI1_BASE + SZ_16K - 1,
		.flags	= IORESOURCE_MEM,
	},
#if defined(CONFIG_SPI_IRQ_MODE)
	[1]={
		.start	= LS1X_SPI1_IRQ,
		.end	= LS1X_SPI1_IRQ,
		.flags	= IORESOURCE_IRQ,
	},
#endif
};

#ifdef CONFIG_SPI_CS_USED_GPIO
static int spi1_gpios_cs[] =
	{ 38, 0, 1 };
#endif

static struct ls1x_spi_platform_data ls1x_spi1_platdata = {
#ifdef CONFIG_SPI_CS_USED_GPIO
	.gpio_cs_count = ARRAY_SIZE(spi1_gpios_cs),
	.gpio_cs = spi1_gpios_cs,
#elif CONFIG_SPI_CS
	.cs_count = SPI1_CS2 + 1,
#endif
};

static struct platform_device ls1x_spi1_device = {
	.name		= "spi_ls1x",
	.id 		= 1,
	.num_resources	= ARRAY_SIZE(ls1x_spi1_resource),
	.resource	= ls1x_spi1_resource,
	.dev		= {
		.platform_data	= &ls1x_spi1_platdata,//&ls1x_spi_devices,
	},
};
#endif	//#ifdef CONFIG_SPI_LS1X_SPI1

#ifdef CONFIG_USB_GADGET_SNPS_DWC_OTG
static u64 ls1c_otg_dma_mask = DMA_BIT_MASK(32);
static struct resource ls1c_otg_resources[] = {
	[0] = {
		.start = LS1X_OTG_BASE,
		.end   = LS1X_OTG_BASE + SZ_64K - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.start = LS1X_OTG_IRQ,
		.end   = LS1X_OTG_IRQ,
		.flags = IORESOURCE_IRQ,
	},
};

static struct platform_device ls1c_otg_device = {
	.name           = "dwc_otg",
	.id             = -1,
	.dev = {
		.dma_mask = &ls1c_otg_dma_mask,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
	},
	.num_resources  = ARRAY_SIZE(ls1c_otg_resources),
	.resource       = ls1c_otg_resources,
};
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
		.name			= "led_green",
		.gpio			= 50,
		.active_low		= 1,
		.default_trigger	= NULL,
		.default_state	= LEDS_GPIO_DEFSTATE_ON,
	}, {
		.name			= "led_yellow",
		.gpio			= 51,
		.active_low		= 1,
		.default_trigger	= NULL,
		.default_state	= LEDS_GPIO_DEFSTATE_ON,
	}, {
		.name			= "led_blue",
		.gpio			= 52,
		.active_low		= 1,
		.default_trigger	= NULL,
		.default_state	= LEDS_GPIO_DEFSTATE_ON,
	},  {
		.name			= "led_red",
		.gpio			= 53,
		.active_low		= 1,
		.default_trigger	= NULL,
		.default_state	= LEDS_GPIO_DEFSTATE_ON,
	}, {
		.name			= "led_orange",
		.gpio			= 54,
		.active_low		= 1,
		.default_trigger	= NULL,
		.default_state	= LEDS_GPIO_DEFSTATE_ON,
	},
};

static struct gpio_led_platform_data gpio_led_info = {
	.leds		= gpio_leds,
	.num_leds	= ARRAY_SIZE(gpio_leds),
};

static struct platform_device ls1x_gpio_leds = {
	.name	= "leds-gpio",
	.id	= -1,
	.dev	= {
		.platform_data	= &gpio_led_info,
	}
};
#endif //#if defined(CONFIG_LEDS_GPIO) || defined(CONFIG_LEDS_GPIO_MODULE)

static struct platform_device *ls1b_platform_devices[] __initdata = {
	&ls1x_uart_device,

#ifdef CONFIG_USB_GADGET_SNPS_DWC_OTG
	&ls1c_otg_device, 
#endif

#ifdef CONFIG_MTD_NAND_LS1X
	&ls1x_nand_device,
#endif

#ifdef CONFIG_USB_EHCI_HCD_LS1X
	&ls1x_ehci_device,
#endif

#ifdef CONFIG_USB_OHCI_HCD_PLATFORM
	&ls1x_ohci_device,
#endif

#ifdef CONFIG_STMMAC_ETH
#ifdef CONFIG_LS1X_GMAC0
	&ls1x_gmac0_mac,
	&ls1x_gmac0_phy,
#endif
#endif

#ifdef CONFIG_SPI_LS1X_SPI0
	&ls1x_spi0_device,
#endif

#ifdef CONFIG_SPI_LS1X_SPI1
	&ls1x_spi1_device,
#endif

#ifdef CONFIG_LS1X_WDT
	&ls1x_wdt_device,
#endif

#if defined(CONFIG_KEYBOARD_GPIO) || defined(CONFIG_KEYBOARD_GPIO_MODULE)
	&ls1x_gpio_keys,
#endif

#if defined(CONFIG_LEDS_GPIO) || defined(CONFIG_LEDS_GPIO_MODULE)
	&ls1x_gpio_leds,
#endif
};

int __init ls1c300b_platform_init(void)
{
	ls1x_serial_setup();

#ifdef CONFIG_STMMAC_ETH
	ls1x_gmac_setup();
#endif

#ifdef CONFIG_SPI_LS1X_SPI0
	spi_register_board_info(ls1x_spi0_devices, ARRAY_SIZE(ls1x_spi0_devices));
#endif

#ifdef CONFIG_SPI_LS1X_SPI1
	spi_register_board_info(ls1x_spi1_devices, ARRAY_SIZE(ls1x_spi1_devices));
#endif

	return platform_add_devices(ls1b_platform_devices, ARRAY_SIZE(ls1b_platform_devices));
}

arch_initcall(ls1c300b_platform_init);


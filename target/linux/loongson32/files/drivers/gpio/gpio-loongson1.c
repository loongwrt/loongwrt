/*
 *  Copyright (c) 2015 Tang Haifeng <tanghaifeng-gz@loongson.cn> or <pengren.mcu@qq.com>
 *
 *  This file is subject to the terms and conditions of the GNU General Public
 *  License. See the file COPYING in the main directory of this archive for
 *  more details.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/err.h>
#include <asm/types.h>
#include <linux/gpio.h>

#include <loongson1.h>
#include <irq.h>

/* gpio group 0 */
static inline int ls1x_gpio0_to_irq(struct gpio_chip *chip, unsigned gpio)
{
	return LS1X_GPIO_FIRST_IRQ + gpio;
}

static inline int ls1x_irq_to_gpio0(unsigned irq)
{
	return irq - LS1X_GPIO_FIRST_IRQ;
}

static int ls1x_gpio0_get_value(struct gpio_chip *chip, unsigned gpio)
{
	return (__raw_readl(LS1X_GPIO_IN0) >> gpio) & 1;
}

static void ls1x_gpio0_set_value(struct gpio_chip *chip, unsigned gpio, int value)
{
	u32 val;
	u32 mask;

	mask = 1 << gpio;
	val = __raw_readl(LS1X_GPIO_OUT0);
	if (value)
		val |= mask;
	else
		val &= ~mask;
	__raw_writel(val, LS1X_GPIO_OUT0);
}

static int ls1x_gpio0_direction_input(struct gpio_chip *chip, unsigned gpio)
{
	u32 temp;
	u32 mask;

	mask = 1 << gpio;
	temp = __raw_readl(LS1X_GPIO_CFG0);
	temp |= mask;
	__raw_writel(temp, LS1X_GPIO_CFG0);
	temp = __raw_readl(LS1X_GPIO_OE0);
	temp |= mask;
	__raw_writel(temp, LS1X_GPIO_OE0);

	return 0;
}

static int ls1x_gpio0_direction_output(struct gpio_chip *chip, unsigned gpio, int value)
{
	u32 temp;
	u32 mask;

	ls1x_gpio0_set_value(chip, gpio, value);
	mask = 1 << gpio;
	temp = __raw_readl(LS1X_GPIO_CFG0);
	temp |= mask;
	__raw_writel(temp, LS1X_GPIO_CFG0);
	temp = __raw_readl(LS1X_GPIO_OE0);
	temp &= (~mask);
	__raw_writel(temp, LS1X_GPIO_OE0);

	return 0;
}

static void ls1x_gpio0_free(struct gpio_chip *chip, unsigned gpio)
{
	u32 temp;

	temp = __raw_readl(LS1X_GPIO_CFG0);
	temp &= ~(1 << gpio);
	__raw_writel(temp, LS1X_GPIO_CFG0);
}

/* gpio group 1 */
static inline int ls1x_gpio1_to_irq(struct gpio_chip *chip, unsigned gpio)
{
	return LS1X_GPIO_FIRST_IRQ + 32 + gpio;
}

static inline int ls1x_irq_to_gpio1(unsigned irq)
{
	return irq - LS1X_GPIO_FIRST_IRQ - 32;
}

static int ls1x_gpio1_get_value(struct gpio_chip *chip, unsigned gpio)
{
	return (__raw_readl(LS1X_GPIO_IN1) >> gpio) & 1;
}

static void ls1x_gpio1_set_value(struct gpio_chip *chip, unsigned gpio, int value)
{
	u32 val;
	u32 mask;

	mask = 1 << gpio;
	val = __raw_readl(LS1X_GPIO_OUT1);
	if (value)
		val |= mask;
	else
		val &= ~mask;
	__raw_writel(val, LS1X_GPIO_OUT1);
}

static int ls1x_gpio1_direction_input(struct gpio_chip *chip, unsigned gpio)
{
	u32 temp;
	u32 mask;

	mask = 1 << gpio;
	temp = __raw_readl(LS1X_GPIO_CFG1);
	temp |= mask;
	__raw_writel(temp, LS1X_GPIO_CFG1);
	temp = __raw_readl(LS1X_GPIO_OE1);
	temp |= mask;
	__raw_writel(temp, LS1X_GPIO_OE1);

	return 0;
}

static int ls1x_gpio1_direction_output(struct gpio_chip *chip, unsigned gpio, int value)
{
	u32 temp;
	u32 mask;

	ls1x_gpio1_set_value(chip, gpio, value);
	mask = 1 << gpio;
	temp = __raw_readl(LS1X_GPIO_CFG1);
	temp |= mask;
	__raw_writel(temp, LS1X_GPIO_CFG1);
	temp = __raw_readl(LS1X_GPIO_OE1);
	temp &= (~mask);
	__raw_writel(temp, LS1X_GPIO_OE1);

	return 0;
}

static void ls1x_gpio1_free(struct gpio_chip *chip, unsigned gpio)
{
	u32 temp;

	temp = __raw_readl(LS1X_GPIO_CFG1);
	temp &= ~(1 << gpio);
	__raw_writel(temp, LS1X_GPIO_CFG1);
}

/* gpio group 2 */
#if defined(CONFIG_LOONGSON1_LS1A) || defined(CONFIG_LOONGSON1_LS1C)
static inline int ls1x_gpio2_to_irq(struct gpio_chip *chip, unsigned gpio)
{
	return LS1X_GPIO_FIRST_IRQ + 64 + gpio;
}

static inline int ls1x_irq_to_gpio2(unsigned irq)
{
	return irq - LS1X_GPIO_FIRST_IRQ - 64;
}

static int ls1x_gpio2_get_value(struct gpio_chip *chip, unsigned gpio)
{
	return (__raw_readl(LS1X_GPIO_IN2) >> gpio) & 1;
}

static void ls1x_gpio2_set_value(struct gpio_chip *chip, unsigned gpio, int value)
{
	u32 val;
	u32 mask;

	mask = 1 << gpio;
	val = __raw_readl(LS1X_GPIO_OUT2);
	if (value)
		val |= mask;
	else
		val &= ~mask;
	__raw_writel(val, LS1X_GPIO_OUT2);
}

static int ls1x_gpio2_direction_input(struct gpio_chip *chip, unsigned gpio)
{
	u32 temp;
	u32 mask;

	mask = 1 << gpio;
	temp = __raw_readl(LS1X_GPIO_CFG2);
	temp |= mask;
	__raw_writel(temp, LS1X_GPIO_CFG2);
	temp = __raw_readl(LS1X_GPIO_OE2);
	temp |= mask;
	__raw_writel(temp, LS1X_GPIO_OE2);

	return 0;
}

static int ls1x_gpio2_direction_output(struct gpio_chip *chip, unsigned gpio, int value)
{
	u32 temp;
	u32 mask;

	ls1x_gpio2_set_value(chip, gpio, value);
	mask = 1 << gpio;
	temp = __raw_readl(LS1X_GPIO_CFG2);
	temp |= mask;
	__raw_writel(temp, LS1X_GPIO_CFG2);
	temp = __raw_readl(LS1X_GPIO_OE2);
	temp &= (~mask);
	__raw_writel(temp, LS1X_GPIO_OE2);

	return 0;
}

static void ls1x_gpio2_free(struct gpio_chip *chip, unsigned gpio)
{
	u32 temp;

	temp = __raw_readl(LS1X_GPIO_CFG2);
	temp &= ~(1 << gpio);
	__raw_writel(temp, LS1X_GPIO_CFG2);
}
#endif

/* gpio group 3 */
#if defined(CONFIG_LOONGSON1_LS1C)
static inline int ls1x_gpio3_to_irq(struct gpio_chip *chip, unsigned gpio)
{
	return LS1X_GPIO_FIRST_IRQ + 96 + gpio;
}

static inline int ls1x_irq_to_gpio3(unsigned irq)
{
	return irq - LS1X_GPIO_FIRST_IRQ - 96;
}

static int ls1x_gpio3_get_value(struct gpio_chip *chip, unsigned gpio)
{
	return (__raw_readl(LS1X_GPIO_IN3) >> gpio) & 1;
}

static void ls1x_gpio3_set_value(struct gpio_chip *chip, unsigned gpio, int value)
{
	u32 val;
	u32 mask;

	mask = 1 << gpio;
	val = __raw_readl(LS1X_GPIO_OUT3);
	if (value)
		val |= mask;
	else
		val &= ~mask;
	__raw_writel(val, LS1X_GPIO_OUT3);
}

static int ls1x_gpio3_direction_input(struct gpio_chip *chip, unsigned gpio)
{
	u32 temp;
	u32 mask;

	mask = 1 << gpio;
	temp = __raw_readl(LS1X_GPIO_CFG3);
	temp |= mask;
	__raw_writel(temp, LS1X_GPIO_CFG3);
	temp = __raw_readl(LS1X_GPIO_OE3);
	temp |= mask;
	__raw_writel(temp, LS1X_GPIO_OE3);

	return 0;
}

static int ls1x_gpio3_direction_output(struct gpio_chip *chip, unsigned gpio, int value)
{
	u32 temp;
	u32 mask;

	ls1x_gpio3_set_value(chip, gpio, value);
	mask = 1 << gpio;
	temp = __raw_readl(LS1X_GPIO_CFG3);
	temp |= mask;
	__raw_writel(temp, LS1X_GPIO_CFG3);
	temp = __raw_readl(LS1X_GPIO_OE3);
	temp &= (~mask);
	__raw_writel(temp, LS1X_GPIO_OE3);

	return 0;
}

static void ls1x_gpio3_free(struct gpio_chip *chip, unsigned gpio)
{
	u32 temp;

	temp = __raw_readl(LS1X_GPIO_CFG3);
	temp &= ~(1 << gpio);
	__raw_writel(temp, LS1X_GPIO_CFG3);
}
#endif

static struct gpio_chip ls1x_chip[] = {
	[0] = {
		.label                  = "ls1x-gpio0",
		.direction_input        = ls1x_gpio0_direction_input,
		.direction_output       = ls1x_gpio0_direction_output,
		.get                    = ls1x_gpio0_get_value,
		.set                    = ls1x_gpio0_set_value,
		.free					= ls1x_gpio0_free,
		.to_irq					= ls1x_gpio0_to_irq,
		.base                   = 0,
		.ngpio                  = 32,
	},
	[1] = {
		.label                  = "ls1x-gpio1",
		.direction_input        = ls1x_gpio1_direction_input,
		.direction_output       = ls1x_gpio1_direction_output,
		.get                    = ls1x_gpio1_get_value,
		.set                    = ls1x_gpio1_set_value,
		.free					= ls1x_gpio1_free,
		.to_irq					= ls1x_gpio1_to_irq,
		.base                   = 32,
		.ngpio                  = 32,
	},
#if defined(CONFIG_LOONGSON1_LS1A) || defined(CONFIG_LOONGSON1_LS1C)
	[2] = {
		.label                  = "ls1x-gpio2",
		.direction_input        = ls1x_gpio2_direction_input,
		.direction_output       = ls1x_gpio2_direction_output,
		.get                    = ls1x_gpio2_get_value,
		.set                    = ls1x_gpio2_set_value,
		.free					= ls1x_gpio2_free,
		.to_irq					= ls1x_gpio2_to_irq,
		.base                   = 64,
		.ngpio                  = 32,
	},
#endif
#if defined(CONFIG_LOONGSON1_LS1C)
	[3] = {
		.label                  = "ls1x-gpio3",
		.direction_input        = ls1x_gpio3_direction_input,
		.direction_output       = ls1x_gpio3_direction_output,
		.get                    = ls1x_gpio3_get_value,
		.set                    = ls1x_gpio3_set_value,
		.free					= ls1x_gpio3_free,
		.to_irq					= ls1x_gpio3_to_irq,
		.base                   = 96,
		.ngpio                  = 32,
	},
#endif
};

static int __init ls1x_gpio_setup(void)
{
	gpiochip_add(&ls1x_chip[0]);
	gpiochip_add(&ls1x_chip[1]);
#if defined(CONFIG_LOONGSON1_LS1A) || defined(CONFIG_LOONGSON1_LS1C)
	gpiochip_add(&ls1x_chip[2]);
#endif
#if defined(CONFIG_LOONGSON1_LS1C)
	gpiochip_add(&ls1x_chip[3]);
#endif
	return 0;
}
postcore_initcall(ls1x_gpio_setup);

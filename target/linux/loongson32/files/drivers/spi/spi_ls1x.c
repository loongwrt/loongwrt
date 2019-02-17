/*
 * Loongson1 ls1x SPI master driver
 *
 * Copyright (C) 2013 Tang, Haifeng <tanghaifeng-gz@loongson.cn> 
 * <pengren.mcu@qq.com>
 *
 * Based on spi_oc_tiny.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_bitbang.h>
#include <linux/spi/spi_ls1x.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/clk.h>

#define DRV_NAME "spi_ls1x"

#define REG_SPCR		0x00	//控制寄存器
#define REG_SPSR		0x01	//状态寄存器
#define REG_TXFIFO		0x02	//数据传输寄存器 输出
#define REG_RXFIFO		0x02	//数据传输寄存器 输入
#define REG_SPER		0x03	//外部寄存器
#define REG_PARAM		0x04	//SPI Flash参数控制寄存器
#define REG_SOFTCS		0x05	//SPI Flash片选控制寄存器
#define REG_TIMING		0x06	//SPI Flash时序控制寄存器

struct ls1x_spi {
	/* bitbang has to be first */
	struct spi_bitbang bitbang;
	struct completion done;

	void __iomem *base;
	int irq;
	unsigned int div;
	unsigned int speed_hz;
	unsigned int mode;
	unsigned int len;
	unsigned int txc, rxc;
	const u8 *txp;
	u8 *rxp;
#if defined(CONFIG_SPI_CS_USED_GPIO)
	unsigned int gpio_cs_count;
	int *gpio_cs;
#endif

	struct clk *clk;
};

static inline struct ls1x_spi *ls1x_spi_to_hw(struct spi_device *sdev)
{
	return spi_master_get_devdata(sdev->master);
}

static unsigned int ls1x_spi_div(struct spi_device *spi, unsigned int hz)
{
	struct ls1x_spi *hw = ls1x_spi_to_hw(spi);
	unsigned int div, div_tmp, bit;
	unsigned long clk;

	clk = clk_get_rate(hw->clk);
	div = DIV_ROUND_UP(clk, hz);

	if (div < 2)
		div = 2;

	if (div > 4096)
		div = 4096;

	bit = fls(div) - 1;
	switch(1 << bit) {
		case 16: 
			div_tmp = 2;
			if (div > (1<<bit)) {
				div_tmp++;
			}
			break;
		case 32:
			div_tmp = 3;
			if (div > (1<<bit)) {
				div_tmp += 2;
			}
			break;
		case 8:
			div_tmp = 4;
			if (div > (1<<bit)) {
				div_tmp -= 2;
			}
			break;
		default:
			div_tmp = bit - 1;
			if (div > (1<<bit)) {
				div_tmp++;
			}
			break;
	}
	dev_dbg(&spi->dev, "clk = %ld hz = %d div_tmp = %d bit = %d\n", 
			clk, hz, div_tmp, bit);

	return div_tmp;
}

static void ls1x_spi_chipselect(struct spi_device *spi, int is_active)
{
	struct ls1x_spi *hw = ls1x_spi_to_hw(spi);

#if defined(CONFIG_SPI_CS_USED_GPIO)
	int i, num_chipselect;

	num_chipselect = hw->bitbang.master->num_chipselect;

	/* 使其他片选失效，spi sd卡会使片选一直有效，spi sd卡驱动的问题？ */
	if (unlikely(spi->mode & SPI_CS_HIGH)) {
		if (is_active) {
			for (i=0; i<num_chipselect; i++) {
				gpio_set_value(hw->gpio_cs[i], 0);
			}
			gpio_set_value(hw->gpio_cs[spi->chip_select], 1);
		} else {
			for (i=0; i<num_chipselect; i++) {
				gpio_set_value(hw->gpio_cs[i], 0);
			}
		}
	} else {
		if (is_active) {
			for (i=0; i<num_chipselect; i++) {
				gpio_set_value(hw->gpio_cs[i], 1);
			}
			gpio_set_value(hw->gpio_cs[spi->chip_select], 0);
		} else {
			for (i=0; i<num_chipselect; i++) {
				gpio_set_value(hw->gpio_cs[i], 1);
			}
		}
	}
#elif defined(CONFIG_SPI_CS)
	u8 ret;
	ret = readb(hw->base + REG_SOFTCS);
	ret = (ret & 0xf0) | (0x01 << spi->chip_select);
	
	if (unlikely(spi->mode & SPI_CS_HIGH)) {
		if (is_active) {
			ret = ret | (0x10 << spi->chip_select);
			writeb(ret, hw->base + REG_SOFTCS);
		} else {
			ret = ret & (~(0x10 << spi->chip_select));
			writeb(ret, hw->base + REG_SOFTCS);
		}
	} else {
		if (is_active) {
			ret = ret & (~(0x10 << spi->chip_select));
			writeb(ret, hw->base + REG_SOFTCS);
		} else {
			ret = ret | (0x10 << spi->chip_select);
			writeb(ret, hw->base + REG_SOFTCS);
		}
	}
#endif
}

static int ls1x_spi_setup(struct spi_device *spi)
{
	struct ls1x_spi *hw = ls1x_spi_to_hw(spi);

	/* 注意spi bit per word 控制器支持8bit */
//	bpw = t ? t->bits_per_word : spi->bits_per_word;

	if (spi->max_speed_hz != hw->speed_hz) {
		hw->speed_hz = spi->max_speed_hz;
		hw->div = ls1x_spi_div(spi, hw->speed_hz);
	}
	hw->mode = spi->mode & (SPI_CPOL | SPI_CPHA);
	return 0;
}

static int ls1x_spi_setup_transfer(struct spi_device *spi,
				   struct spi_transfer *t)
{
	struct ls1x_spi *hw = ls1x_spi_to_hw(spi);
	u8 ret;

	ls1x_spi_setup(spi);

/*	if (t) {
		if (t->speed_hz && (t->speed_hz != hw->speed_hz))
			div = ls1x_spi_div(spi, t->speed_hz);
	}*/

	ret = readb(hw->base + REG_SPCR);
	ret = ret & 0xf0;
	ret = ret | (hw->mode << 2) | (hw->div & 0x03);
	writeb(ret, hw->base + REG_SPCR);

	ret = readb(hw->base + REG_SPER);
	ret = ret & 0xfc;
	ret = ret | (hw->div >> 2);
	writeb(ret, hw->base + REG_SPER);

	return 0;
}

static inline void ls1x_spi_wait_rxe(struct ls1x_spi *hw)
{
	u8 ret;

	ret = readb(hw->base + REG_SPSR);
	ret = ret | 0x80;
	writeb(ret, hw->base + REG_SPSR);	/* Int Clear */

	ret = readb(hw->base + REG_SPSR);
	if (ret & 0x40) {
		writeb(ret & 0xbf, hw->base + REG_SPSR);	/* Write-Collision Clear */
	}
}

static inline void ls1x_spi_wait_txe(struct ls1x_spi *hw)
{
	int timeout = 20000;

	while (timeout) {
		if (readb(hw->base + REG_SPSR) & 0x80) {
			break;
		}
		timeout--;
//		cpu_relax();
	}

//	if (timeout == 0) {
//		printk(KERN_ERR "spi transfer wait time out !\n");
//	}
}

static int ls1x_spi_txrx_bufs(struct spi_device *spi, struct spi_transfer *t)
{
	struct ls1x_spi *hw = ls1x_spi_to_hw(spi);

#if defined(CONFIG_SPI_IRQ_MODE)
	if (hw->irq >= 0) {
		/* use intrrupt driven data transfer */
		hw->len = t->len;
		hw->txp = t->tx_buf;
		hw->rxp = t->rx_buf;
		hw->txc = 0;
		hw->rxc = 0;

		/* send the first byte */
		writeb(hw->txp ? *hw->txp++ : 0, hw->base + REG_TXFIFO);
		hw->txc++;
		wait_for_completion(&hw->done);
	}
#elif CONFIG_SPI_POLL_MODE
	const u8 *txp = t->tx_buf;
	u8 *rxp = t->rx_buf;
	unsigned int i;

	if (txp && rxp) {
		for (i = 0; i < t->len; i += 1) {
			writeb(*txp++, hw->base + REG_TXFIFO);
			ls1x_spi_wait_txe(hw);
			*rxp++ = readb(hw->base + REG_RXFIFO);
			ls1x_spi_wait_rxe(hw);
		}
	} else if (rxp) {
		for (i = 0; i < t->len; i += 1) {
			writeb(0, hw->base + REG_TXFIFO);
			ls1x_spi_wait_txe(hw);
			*rxp++ = readb(hw->base + REG_RXFIFO);
			ls1x_spi_wait_rxe(hw);
		}
	} else if (txp) {
		for (i = 0; i < t->len; i += 1) {
			writeb(*txp++, hw->base + REG_TXFIFO);
			ls1x_spi_wait_txe(hw);
			readb(hw->base + REG_RXFIFO);
			ls1x_spi_wait_rxe(hw);
		}
	} else {
		for (i = 0; i < t->len; i += 1) {
			writeb(0, hw->base + REG_TXFIFO);
			ls1x_spi_wait_txe(hw);
			readb(hw->base + REG_RXFIFO);
			ls1x_spi_wait_rxe(hw);
		}
	}
#endif
	return t->len;
}

#if defined(CONFIG_SPI_IRQ_MODE)
static irqreturn_t ls1x_spi_irq(int irq, void *dev)
{
	struct ls1x_spi *hw = dev;
	u8 ret = readb(hw->base + REG_SPSR);

	writeb(ret | 0x80, hw->base + REG_SPSR);

	if (hw->rxc + 1 == hw->len) {
		if (hw->rxp) {
			*hw->rxp++ = readb(hw->base + REG_RXFIFO);
		} else {
			readb(hw->base + REG_RXFIFO);
		}
		hw->rxc++;
		complete(&hw->done);
	} else {
		if (hw->rxp) {
			*hw->rxp++ = readb(hw->base + REG_RXFIFO);
		} else {
			readb(hw->base + REG_RXFIFO);
		}
		hw->rxc++;
		if (hw->txc < hw->len) {
			writeb(hw->txp ? *hw->txp++ : 0, hw->base + REG_TXFIFO);
			hw->txc++;
		}
	}

	return IRQ_HANDLED;
}
#endif

#ifdef CONFIG_SPI_CS_USED_GPIO
#ifdef CONFIG_OF
#include <linux/of_gpio.h>

static int ls1x_spi_of_probe(struct platform_device *pdev)
{
	struct ls1x_spi *hw = platform_get_drvdata(pdev);
	struct device_node *np = pdev->dev.of_node;
	unsigned int i;
	const __be32 *val;
	int len;

	if (!np)
		return 0;
	hw->gpio_cs_count = of_gpio_count(np);
	if (hw->gpio_cs_count) {
		hw->gpio_cs = devm_kzalloc(&pdev->dev,
				hw->gpio_cs_count * sizeof(unsigned int),
				GFP_KERNEL);
		if (!hw->gpio_cs)
			return -ENOMEM;
	}
	for (i = 0; i < hw->gpio_cs_count; i++) {
		hw->gpio_cs[i] = of_get_gpio_flags(np, i, NULL);
		if (hw->gpio_cs[i] < 0)
			return -ENODEV;
	}
	hw->bitbang.master->dev.of_node = pdev->dev.of_node;
/*	val = of_get_property(pdev->dev.of_node,
			      "clock-frequency", &len);
	if (val && len >= sizeof(__be32))
		hw->freq = be32_to_cpup(val);
	val = of_get_property(pdev->dev.of_node, "div-width", &len);
	if (val && len >= sizeof(__be32))
		hw->baudwidth = be32_to_cpup(val);*/
	return 0;
}
#else /* !CONFIG_OF */
static int ls1x_spi_of_probe(struct platform_device *pdev)
{
	return 0;
}
#endif /* CONFIG_OF */
#endif

static void ls1x_spi_hw_init(struct ls1x_spi *hw)
{
	u8 val;

	/* 使能SPI控制器，master模式，使能或关闭中断 */
	if (hw->irq >= 0) {
		writeb(0xd3, hw->base + REG_SPCR);
	} else {
		writeb(0x53, hw->base + REG_SPCR);
	}
	/* 清空状态寄存器 */
	writeb(0xc0, hw->base + REG_SPSR);
	/* 1字节产生中断，采样(读)与发送(写)时机同时 */
	writeb(0x03, hw->base + REG_SPER);
#if defined(CONFIG_SPI_CS_USED_GPIO)
	writeb(0x00, hw->base + REG_SOFTCS);
#elif defined(CONFIG_SPI_CS)
	writeb(0xff, hw->base + REG_SOFTCS);
#endif
	/* 关闭SPI flash */
	val = readb(hw->base + REG_PARAM);
	val &= 0xfe;
	writeb(val, hw->base + REG_PARAM);
	/* SPI flash时序控制寄存器 */
	writeb(0x05, hw->base + REG_TIMING);
}

static int ls1x_spi_probe(struct platform_device *pdev)
{
	struct ls1x_spi_platform_data *platp = pdev->dev.platform_data;
	struct ls1x_spi *hw;
	struct spi_master *master;
	struct resource *res;
	int err = -ENODEV;
#if defined(CONFIG_SPI_CS_USED_GPIO)
	unsigned int i;
#endif

	master = spi_alloc_master(&pdev->dev, sizeof(struct ls1x_spi));
	if (!master)
		return err;

	/* setup the master state. */
	master->bus_num = pdev->id;
	master->num_chipselect = 32;
	master->mode_bits = SPI_CPOL | SPI_CPHA;
	master->setup = ls1x_spi_setup;

	hw = spi_master_get_devdata(master);
	platform_set_drvdata(pdev, hw);

	/* setup the state for the bitbang driver */
	hw->bitbang.master = spi_master_get(master);
	if (!hw->bitbang.master)
		return err;
	hw->bitbang.setup_transfer = ls1x_spi_setup_transfer;
	hw->bitbang.chipselect = ls1x_spi_chipselect;
	hw->bitbang.txrx_bufs = ls1x_spi_txrx_bufs;
	hw->bitbang.flags = SPI_CS_HIGH;

	/* find and map our resources */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		goto exit_busy;
	if (!devm_request_mem_region(&pdev->dev, res->start, resource_size(res),
				     pdev->name))
		goto exit_busy;
	hw->base = devm_ioremap_nocache(&pdev->dev, res->start,
					resource_size(res));
	if (!hw->base)
		goto exit_busy;
#if defined(CONFIG_SPI_IRQ_MODE)
	/* irq is optional */
	hw->irq = platform_get_irq(pdev, 0);
	if (hw->irq >= 0) {
		init_completion(&hw->done);
		err = devm_request_irq(&pdev->dev, hw->irq, ls1x_spi_irq, 0,
				       pdev->name, hw);
		if (err)
			goto exit;
	}
#else
	hw->irq = -1;
#endif

#if defined(CONFIG_SPI_CS_USED_GPIO)
	/* find platform data */
	if (platp) {
		hw->gpio_cs_count = platp->gpio_cs_count;
		hw->gpio_cs = platp->gpio_cs;
		if (platp->gpio_cs_count && !platp->gpio_cs)
			goto exit_busy;
	} else {
		err = ls1x_spi_of_probe(pdev);
		if (err)
			goto exit;
	}
	for (i = 0; i < hw->gpio_cs_count; i++) {
		err = gpio_request(hw->gpio_cs[i], dev_name(&pdev->dev));
		if (err)
			goto exit_gpio;
		gpio_direction_output(hw->gpio_cs[i], 1);
	}
	hw->bitbang.master->num_chipselect = max(1U, hw->gpio_cs_count);
#elif defined(CONFIG_SPI_CS)
	hw->bitbang.master->num_chipselect = platp->cs_count;
#endif

	ls1x_spi_hw_init(hw);

	hw->clk = clk_get(&pdev->dev, "apb_clk");
	if (IS_ERR(hw->clk)) {
		dev_err(&pdev->dev, "No clock for device\n");
		err = PTR_ERR(hw->clk);
		goto err_no_clk;
	}

	/* register our spi controller */
	err = spi_bitbang_start(&hw->bitbang);
	if (err)
		goto exit;
	dev_info(&pdev->dev, "base %p, irq %d\n", hw->base, hw->irq);

	return 0;

err_no_clk:
#if defined(CONFIG_SPI_CS_USED_GPIO)
exit_gpio:
	while (i-- > 0)
		gpio_free(hw->gpio_cs[i]);
#endif
exit_busy:
	err = -EBUSY;
exit:
	platform_set_drvdata(pdev, NULL);
	spi_master_put(master);
	return err;
}

static int ls1x_spi_remove(struct platform_device *pdev)
{
	struct ls1x_spi *hw = platform_get_drvdata(pdev);
	struct spi_master *master = hw->bitbang.master;
#if defined(CONFIG_SPI_CS_USED_GPIO)
	unsigned int i;
#endif

	spi_bitbang_stop(&hw->bitbang);
#if defined(CONFIG_SPI_CS_USED_GPIO)
	for (i = 0; i < hw->gpio_cs_count; i++)
		gpio_free(hw->gpio_cs[i]);
#endif
	platform_set_drvdata(pdev, NULL);
	spi_master_put(master);
	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id ls1x_spi_match[] = {
	{ .compatible = "loongson1,ls1x-spi", },
	{},
};
MODULE_DEVICE_TABLE(of, ls1x_spi_match);
#else /* CONFIG_OF */
#define ls1x_spi_match NULL
#endif /* CONFIG_OF */

static struct platform_driver ls1x_spi_driver = {
	.probe = ls1x_spi_probe,
	.remove = ls1x_spi_remove,
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
		.pm = NULL,
		.of_match_table = of_match_ptr(ls1x_spi_match),
	},
};
module_platform_driver(ls1x_spi_driver);

MODULE_DESCRIPTION("Loongson1 SPI driver");
MODULE_AUTHOR("Tang, Haifeng <tanghaifeng-gz@loongson.cn>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" DRV_NAME);

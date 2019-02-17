/*
 * Loongson1 ls1x SPI master driver
 *
 * Copyright (C) 2013 Tang Haifeng <tanghaifeng-gz@loongson.cn>
 * Copyright (C) 2019 Rosy Song <rosysong@rosinson.com>
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

#define DRV_NAME "ls1x-spi"

#define LS1X_SPI_SPCR		0x00
#define LS1X_SPI_SPSR		0x01
#define LS1X_SPI_TXFIFO		0x02
#define LS1X_SPI_RXFIFO		0x02
#define LS1X_SPI_SPER		0x03
#define LS1X_SPI_PARAM		0x04
#define LS1X_SPI_SOFTCS		0x05
#define LS1X_SPI_TIMING		0x06

struct ls1x_spi {
	/* bitbang has to be first */
	struct spi_bitbang bitbang;
	struct completion done;
	struct clk *clk;

	void __iomem *base;
	int irq;
	unsigned int div;
	unsigned int speed_hz;
	unsigned int mode;
	unsigned int len;
	unsigned int txc, rxc;
	const u8 *txp;
	u8 *rxp;
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
	else if (div > 4096)
		div = 4096;

	bit = fls(div) - 1;
	switch (1 << bit) {
		case 16:
			div_tmp = 2;
			if (div > (1 << bit))
				div_tmp++;
			break;
		case 32:
			div_tmp = 3;
			if (div > (1 << bit))
				div_tmp += 2;
			break;
		case 8:
			div_tmp = 4;
			if (div > (1 << bit))
				div_tmp -= 2;
			break;
		default:
			div_tmp = bit - 1;
			if (div > ( 1<< bit))
				div_tmp++;
			break;
	}
	dev_dbg(&spi->dev, "clk = %ld hz = %d div_tmp = %d bit = %d\n",
			clk, hz, div_tmp, bit);

	return div_tmp;
}

static void ls1x_spi_chipselect(struct spi_device *spi, int is_active)
{
	struct ls1x_spi *hw = ls1x_spi_to_hw(spi);
	u8 val = (readb(hw->base + LS1X_SPI_SOFTCS) & 0xf0) |\
		(1 << spi->chip_select);

	if (unlikely(spi->mode & SPI_CS_HIGH)) {
		if (is_active)
			val |= (0x10 << spi->chip_select);
		else
			val &= (~(0x10 << spi->chip_select));
	} else {
		if (is_active)
			val &= (~(0x10 << spi->chip_select));
		else
			val |= (0x10 << spi->chip_select);
	}
	writeb(val, hw->base + LS1X_SPI_SOFTCS);
}

static int ls1x_spi_setup(struct spi_device *spi)
{
	struct ls1x_spi *hw = ls1x_spi_to_hw(spi);

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
	u8 val;

	ls1x_spi_setup(spi);

	val = readb(hw->base + LS1X_SPI_SPCR) & 0xf0;
	val |= (hw->mode << 2) | (hw->div & 0x03);
	writeb(val, hw->base + LS1X_SPI_SPCR);

	val = readb(hw->base + LS1X_SPI_SPER) & 0xfc;
	val |= (hw->div >> 2);
	writeb(val, hw->base + LS1X_SPI_SPER);

	return 0;
}

static inline void ls1x_spi_wait_rxe(struct ls1x_spi *hw)
{
	u8 val;

	/* Int Clear */
	writeb(readb(hw->base + LS1X_SPI_SPSR) | 0x80,\
		hw->base + LS1X_SPI_SPSR);

	val = readb(hw->base + LS1X_SPI_SPSR);
	if (val & 0x40)
		writeb(val & 0xbf, hw->base + LS1X_SPI_SPSR);	/* Write-Collision Clear */
}

static inline void ls1x_spi_wait_txe(struct ls1x_spi *hw)
{
	unsigned int timeout = 20000;
	while (timeout) {
		if (readb(hw->base + LS1X_SPI_SPSR) & 0x80)
			break;
		timeout--;
	}
}

static int ls1x_spi_txrx_bufs(struct spi_device *spi, struct spi_transfer *t)
{
	struct ls1x_spi *hw = ls1x_spi_to_hw(spi);
	const u8 *txp = t->tx_buf;
	u8 *rxp = t->rx_buf;
	unsigned int i;

	if (txp && rxp) {
		for (i = 0; i < t->len; i += 1) {
			writeb(*txp++, hw->base + LS1X_SPI_TXFIFO);
			ls1x_spi_wait_txe(hw);
			*rxp++ = readb(hw->base + LS1X_SPI_RXFIFO);
			ls1x_spi_wait_rxe(hw);
		}
	} else if (rxp) {
		for (i = 0; i < t->len; i += 1) {
			writeb(0, hw->base + LS1X_SPI_TXFIFO);
			ls1x_spi_wait_txe(hw);
			*rxp++ = readb(hw->base + LS1X_SPI_RXFIFO);
			ls1x_spi_wait_rxe(hw);
		}
	} else if (txp) {
		for (i = 0; i < t->len; i += 1) {
			writeb(*txp++, hw->base + LS1X_SPI_TXFIFO);
			ls1x_spi_wait_txe(hw);
			readb(hw->base + LS1X_SPI_RXFIFO);
			ls1x_spi_wait_rxe(hw);
		}
	} else {
		for (i = 0; i < t->len; i += 1) {
			writeb(0, hw->base + LS1X_SPI_TXFIFO);
			ls1x_spi_wait_txe(hw);
			readb(hw->base + LS1X_SPI_RXFIFO);
			ls1x_spi_wait_rxe(hw);
		}
	}

	return t->len;
}

static void ls1x_spi_hw_init(struct ls1x_spi *hw)
{
	u8 val;

	/* enable spi flash, master mode */
	if (hw->irq >= 0) {
		writeb(0xd3, hw->base + LS1X_SPI_SPCR);
	} else {
		writeb(0x53, hw->base + LS1X_SPI_SPCR);
	}

	/* clear status reg */
	writeb(0xc0, hw->base + LS1X_SPI_SPSR);

	writeb(0x03, hw->base + LS1X_SPI_SPER);
	writeb(0xff, hw->base + LS1X_SPI_SOFTCS);

	/* disable spi flash */
	val = readb(hw->base + LS1X_SPI_PARAM) & 0xfe;
	writeb(val, hw->base + LS1X_SPI_PARAM);

	writeb(0x05, hw->base + LS1X_SPI_TIMING);
}

static int ls1x_spi_probe(struct platform_device *pdev)
{
	struct plat_ls1x_spi *platp = pdev->dev.platform_data;
	struct ls1x_spi *hw;
	struct spi_master *master;
	struct resource *res;
	int err = -ENODEV;

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
		goto err_busy;
	if (!devm_request_mem_region(&pdev->dev, res->start, resource_size(res),
				     pdev->name))
		goto err_busy;
	hw->base = devm_ioremap_nocache(&pdev->dev, res->start,
					resource_size(res));
	if (!hw->base)
		goto err_busy;

	hw->irq = -1;
	hw->bitbang.master->num_chipselect = platp->cs_count;

	ls1x_spi_hw_init(hw);

	hw->clk = clk_get(&pdev->dev, "apb_clk");
	if (IS_ERR(hw->clk)) {
		dev_err(&pdev->dev, "No clock for device\n");
		err = PTR_ERR(hw->clk);
		goto err_exit;
	}

	/* register our spi controller */
	err = spi_bitbang_start(&hw->bitbang);
	if (err)
		goto err_exit;
	dev_info(&pdev->dev, "base %p, irq %d\n", hw->base, hw->irq);

	return 0;

err_busy:
	err = -EBUSY;
err_exit:
	platform_set_drvdata(pdev, NULL);
	spi_master_put(master);
	return err;
}

static int ls1x_spi_remove(struct platform_device *pdev)
{
	struct ls1x_spi *hw = platform_get_drvdata(pdev);
	struct spi_master *master = hw->bitbang.master;

	spi_bitbang_stop(&hw->bitbang);
	platform_set_drvdata(pdev, NULL);
	spi_master_put(master);
	return 0;
}

static struct platform_driver ls1x_spi_driver = {
	.probe = ls1x_spi_probe,
	.remove = ls1x_spi_remove,
	.driver = {
		.name = DRV_NAME,
		.owner = THIS_MODULE,
		.pm = NULL,
		.of_match_table = of_match_ptr(NULL),
	},
};
module_platform_driver(ls1x_spi_driver);

MODULE_DESCRIPTION("Loongson1 SPI driver");
MODULE_AUTHOR("Tang, Haifeng <tanghaifeng-gz@loongson.cn>");
MODULE_AUTHOR("Rosy Song <rosysong@rosinson.com>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" DRV_NAME);

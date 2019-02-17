/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_SPI_SPI_LS1X_H
#define _LINUX_SPI_SPI_LS1X_H

/**
 * struct ls1x_spi_platform_data - platform data of the Loongson1 SPI
 * @cs_count:	number of pins used for chipselect, control via SOFTCS register.
 */
struct ls1x_spi_platform_data {
	unsigned int cs_count;
};

#endif /* _LINUX_SPI_SPI_LS1X_H */

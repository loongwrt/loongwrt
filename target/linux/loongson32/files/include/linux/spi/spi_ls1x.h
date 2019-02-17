#ifndef _LINUX_SPI_SPI_LS1X_H
#define _LINUX_SPI_SPI_LS1X_H

/**
 * struct ls1x_spi_platform_data - platform data of the Loongson SPI
 * @freq:	input clock freq to the core.
 * @baudwidth:	baud rate divider width of the core.
 * @gpio_cs_count:	number of gpio pins used for chipselect.
 * @gpio_cs:	array of gpio pins used for chipselect.
 * @cs_count:	number of pins used for chipselect, control by SOFTCS reg.
 *
 * freq and baudwidth are used only if the divider is programmable.
 */
struct ls1x_spi_platform_data {
//	unsigned int freq;
//	unsigned int baudwidth;
#if defined(CONFIG_SPI_CS_USED_GPIO)
	unsigned int gpio_cs_count;
	int *gpio_cs;
#elif defined(CONFIG_SPI_CS)
	unsigned int cs_count;
#endif
};

#endif /* _LINUX_SPI_SPI_LS1X_H */

#include "wiznet_ethernet.h"

/**
 * 
 * SPI callback functions
 * 
 */

void wiznet_critical_section_enter(void)
{
	mcu_disable_global_isr();
}

void wiznet_critical_section_exit(void)
{
	mcu_enable_global_isr();
}

void wiznet_cs_select(void)
{
	io_clear_output(WIZNET_CS);
	softspi_start(WIZNET_SPI);
}

void wiznet_cs_deselect(void)
{
	softspi_stop(WIZNET_SPI);
	io_set_output(WIZNET_CS);
}

uint8_t wiznet_getc(void)
{
	return softspi_xmit(WIZNET_SPI, 0xFF);
}

void wiznet_putc(uint8_t c)
{
	softspi_xmit(WIZNET_SPI, c);
}

void wiznet_read(uint8_t *buff, uint16_t len)
{
	while (len--)
	{
		*buff = softspi_xmit(WIZNET_SPI, 0xFF);
		buff++;
	}
}

void wiznet_write(uint8_t *buff, uint16_t len)
{
	while (len--)
	{
		softspi_xmit(WIZNET_SPI, *buff);
		buff++;
	}
}
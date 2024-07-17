#include "wizchip_spi.h"

/**
 * 
 * SPI callback functions
 * 
 */

void w5XXX_critical_section_enter(void)
{
	mcu_disable_global_isr();
}

void w5XXX_critical_section_exit(void)
{
	mcu_enable_global_isr();
}

void w5XXX_cs_select(void)
{
	io_clear_output(WIZNET_CS);
	softspi_start(WIZNET_SPI);
}

void w5XXX_cs_deselect(void)
{
	softspi_stop(WIZNET_SPI);
	io_set_output(WIZNET_CS);
}

uint8_t w5XXX_getc(void)
{
	return softspi_xmit(WIZNET_SPI, 0xFF);
}

void w5XXX_putc(uint8_t c)
{
	softspi_xmit(WIZNET_SPI, c);
}

void w5XXX_read(uint8_t *buff, uint16_t len)
{
	while (len--)
	{
		*buff = softspi_xmit(WIZNET_SPI, 0xFF);
		buff++;
	}
}

void w5XXX_write(uint8_t *buff, uint16_t len)
{
	while (len--)
	{
		softspi_xmit(WIZNET_SPI, *buff);
		buff++;
	}
}
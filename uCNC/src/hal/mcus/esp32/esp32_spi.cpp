/*
	Name: esp32_arduino.cpp
	Description: Contains all Arduino ESP32 C++ to C functions used by µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 27-07-2022

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#ifdef ESP32
#include <Arduino.h>
#include "esp_task_wdt.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <SPI.h>

extern "C"
{
#include "../../../cnc.h"

#if defined(MCU_HAS_SPI) && defined(USE_ARDUINO_SPI_LIBRARY)

	SPIClass *esp32spi = NULL;
	uint32_t esp32spifreq = SPI_FREQ;
	uint8_t esp32spimode = SPI_MODE0;

	void mcu_spi_config(uint8_t mode, uint32_t freq)
	{
		if (esp32spi != NULL)
		{
			esp32spi->end();
			esp32spi = NULL;
		}

#if (SPI_CLK_BIT == 14 || SPI_CLK_BIT == 25)
		esp32spi = new SPIClass(HSPI);
#else
		esp32spi = new SPIClass(VSPI);
#endif
		esp32spi->begin(SPI_CLK_BIT, SPI_SDI_BIT, SPI_SDO_BIT, SPI_CS_BIT);
		esp32spifreq = freq;
		esp32spimode = mode;
	}

	uint8_t mcu_spi_xmit(uint8_t data)
	{

		esp32spi->beginTransaction(SPISettings(esp32spifreq, MSBFIRST, esp32spimode));
		data = esp32spi->transfer(data);
		esp32spi->endTransaction();
		return data;
	}
#endif
}

#endif

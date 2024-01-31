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
#include <EEPROM.h>

/**
 *
 * This handles EEPROM simulation on flash memory
 *
 * **/

extern "C"
{
#include "../../../cnc.h"

#if !defined(RAM_ONLY_SETTINGS) && defined(USE_ARDUINO_EEPROM_LIBRARY)
	void esp32_eeprom_init(int size)
	{
		EEPROM.begin(size);
	}

	uint8_t mcu_eeprom_getc(uint16_t address)
	{
		return EEPROM.read(address);
	}

	void mcu_eeprom_putc(uint16_t address, uint8_t value)
	{
		EEPROM.write(address, value);
	}

	void mcu_eeprom_flush(void)
	{
		if (!EEPROM.commit())
		{
			protocol_send_feedback(" EEPROM write error");
		}
	}

#endif
}

#endif

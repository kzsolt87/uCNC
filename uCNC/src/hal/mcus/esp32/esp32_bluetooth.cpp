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
#include <BluetoothSerial.h>

extern "C"
{
#include "../../../cnc.h"

#ifdef MCU_HAS_BLUETOOTH

	BluetoothSerial SerialBT;

	uint8_t bt_on;
	uint16_t bt_settings_offset;

	uint8_t esp32_bt_cmd(uint8_t *grbl_cmd_str, uint8_t grbl_cmd_len, uint8_t next_char)
	{
#ifdef BOARD_HAS_CUSTOM_SYSTEM_COMMANDS
		if (!strncmp((const char *)grbl_cmd_str, "BTH", 3))
		{
			if (!strcmp((const char *)&grbl_cmd_str[3], "ON"))
			{
				SerialBT.begin(BOARD_NAME);
				protocol_send_feedback("Bluetooth enabled");
				bt_on = 1;
				settings_save(bt_settings_offset, &bt_on, 1);

				return STATUS_OK;
			}

			if (!strcmp((const char *)&grbl_cmd_str[3], "OFF"))
			{
				SerialBT.end();
				protocol_send_feedback("Bluetooth disabled");
				bt_on = 0;
				settings_save(bt_settings_offset, &bt_on, 1);

				return STATUS_OK;
			}
		}
#endif
		return STATUS_INVALID_STATEMENT;
	}

	void esp32_bt_init(void)
	{
		bt_settings_offset = settings_register_external_setting(1);
		if (settings_load(bt_settings_offset, &bt_on, 1))
		{
			settings_erase(bt_settings_offset, (uint8_t *)&bt_on, 1);
		}

		if (bt_on)
		{
			SerialBT.begin(BOARD_NAME);
		}
	}

#ifndef BLUETOOTH_TX_BUFFER_SIZE
#define BLUETOOTH_TX_BUFFER_SIZE 64
#endif
	DECL_BUFFER(uint8_t, bt_rx, RX_BUFFER_SIZE);
	DECL_BUFFER(uint8_t, bt_tx, BLUETOOTH_TX_BUFFER_SIZE);

	uint8_t mcu_bt_getc(void)
	{
		uint8_t c = 0;
		BUFFER_DEQUEUE(bt_rx, &c);
		return c;
	}

	uint8_t mcu_bt_available(void)
	{
		return BUFFER_READ_AVAILABLE(bt_rx);
	}

	void mcu_bt_clear(void)
	{
		BUFFER_CLEAR(bt_rx);
	}

	void mcu_bt_putc(uint8_t c)
	{
		while (BUFFER_FULL(bt_tx))
		{
			mcu_bt_flush();
		}
		BUFFER_ENQUEUE(bt_tx, &c);
	}

	void mcu_bt_flush(void)
	{
		if (SerialBT.hasClient())
		{
			while (!BUFFER_EMPTY(bt_tx))
			{
				uint8_t tmp[BLUETOOTH_TX_BUFFER_SIZE + 1];
				memset(tmp, 0, sizeof(tmp));
				uint8_t r;

				BUFFER_READ(bt_tx, tmp, BLUETOOTH_TX_BUFFER_SIZE, r);
				SerialBT.write(tmp, r);
				SerialBT.flush();
			}
		}
		else
		{
			// no client (discard)
			BUFFER_CLEAR(bt_tx);
		}
	}

	void esp32_bt_process(void)
	{
		if (SerialBT.hasClient())
		{
			while (SerialBT.available() > 0)
			{
				esp_task_wdt_reset();
#ifndef DETACH_BLUETOOTH_FROM_MAIN_PROTOCOL
				uint8_t c = SerialBT.read();
				if (mcu_com_rx_cb(c))
				{
					if (BUFFER_FULL(bt_rx))
					{
						c = OVF;
					}

					*(BUFFER_NEXT_FREE(bt_rx)) = c;
					BUFFER_STORE(bt_rx);
				}
#else
				mcu_bt_rx_cb((uint8_t)SerialBT.read());
#endif
			}
		}
	}
#else
	uint8_t esp32_bt_cmd(uint8_t *grbl_cmd_str, uint8_t grbl_cmd_len, uint8_t next_char) { return STATUS_INVALID_STATEMENT; }
	void esp32_bt_init(void) {}
	void esp32_bt_process(void) {}
#endif
}

#endif

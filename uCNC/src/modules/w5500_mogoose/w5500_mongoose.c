/*
	Name: w5500_mongoose.c
	Description: W5500 driver support with

	Copyright: Copyright (c) Patryk Mierzyński
	Author: Patryk Mierzyński
	Date: 14/01/2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../cnc.h"
#include "../softspi.h"
#include "mongoose.h"

#if (UCNC_MODULE_VERSION < 10903 || UCNC_MODULE_VERSION > 99999)
#error "This module is not compatible with the current version of µCNC"
#endif

#define WIZNET_HW_SPI 1
#define WIZNET_SW_SPI 2

#ifndef WIZNET_BUS
#define WIZNET_BUS WIZNET_HW_SPI
#endif

#ifndef WIZNET_CS
#define WIZNET_CS DOUT28
#endif

#if (WIZNET_BUS == WIZNET_HW_SPI)
#define WIZNET_SPI MCU_SPI
#endif

/**
 * use µCNC internal timer on mongoose
 */
uint64_t mg_millis(void) { return (uint64_t)mcu_millis(); }
/**
 * provide debug output for mongoose
 */
void mg_ucnc_output(char ch, void *data){
	serial_putc(ch);
}
/**
 *
 * W5500 SPI callbacks for the interface
 *
 */
void spi_begin(void *spi)
{
	io_clear_output(WIZNET_CS);
	softspi_start(WIZNET_SPI);
}
void spi_end(void *spi)
{
	softspi_stop(WIZNET_SPI);
	io_set_output(WIZNET_CS);
}

uint8_t spi_txn(void *spi, uint8_t c)
{
	return softspi_xmit(WIZNET_SPI, c);
}

struct mg_mgr mgr; // Mongoose event manager
struct mg_tcpip_spi spi;
struct mg_tcpip_if mif;

void telnet_fn(struct mg_connection *c, int ev, void *ev_data)
{
	switch (ev)
	{
	case MG_EV_ERROR:
		serial_print_str("error\n");
		break; // Error                        char *error_message
	case MG_EV_OPEN:
		serial_print_str("open\n");
		break; // Connection created           NULL
	case MG_EV_POLL:
		serial_print_str("pool\n");
		break; // mg_mgr_poll iteration        uint64_t *uptime_millis
	case MG_EV_RESOLVE:
		serial_print_str("resolve\n");
		break; // Host name is resolved        NULL
	case MG_EV_CONNECT:
		serial_print_str("connect\n");
		break; // Connection established       NULL
	case MG_EV_ACCEPT:
		serial_print_str("accept\n");
		break; // Connection accepted          NULL
	case MG_EV_TLS_HS:
		serial_print_str("tls\n");
		break; // TLS handshake succeeded      NULL
	case MG_EV_READ:
		serial_print_str("read\n");
		break; // Data received from socket    long *bytes_read
	case MG_EV_WRITE:
		serial_print_str("write\n");
		break; // Data written to socket       long *bytes_written
	case MG_EV_CLOSE:
		serial_print_str("close\n");
		break;
	default:
		serial_print_int(ev);
		serial_print_str(" other\n");
		break;
	}
}

bool w5500_mongoose_update(void *params)
{
	mg_mgr_poll(&mgr, 1000);
	return EVENT_CONTINUE;
}

CREATE_EVENT_LISTENER(cnc_dotasks, w5500_mongoose_update);

DECL_MODULE(w5500_mongoose)
{
	softspi_config(WIZNET_SPI, 0, 14000000UL);

	// configure SPI for w5500 via mongoose
	spi.spi = NULL;	 // SPI data
	spi.begin = spi_begin; // begin transation
	spi.end = spi_end;		 // end transaction
	spi.txn = spi_txn;		 // transmit data

	// set w5500 mac
	mif.mac[0] = 0x7e;
	mif.mac[1] = 0x2d;
	mif.mac[2] = 0xac;
	mif.mac[3] = 0x0a;
	mif.mac[4] = 0x49;
	mif.mac[5] = 0xfb;

	mif.driver = &mg_tcpip_driver_w5500;
	mif.driver_data = &spi; // network interface

	mg_log_set_fn(mg_ucnc_output, NULL);
	mg_mgr_init(&mgr);
	mg_tcpip_init(&mgr, &mif);

	mg_listen(&mgr, "tcp://0.0.0.0:23", &telnet_fn, NULL);
#if defined(ENABLE_MAIN_LOOP_MODULES)
	ADD_EVENT_LISTENER(cnc_dotasks, w5500_mongoose_update);
#else // !defined(ENABLE_MAIN_LOOP_MODULES)
#error "Main loop extensions are not enabled. Speaker will not work."
#endif // defined(ENABLE_MAIN_LOOP_MODULES)
}
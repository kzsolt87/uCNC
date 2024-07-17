/*
	Name: wiznet_ethernet.c
	Description: Implements a Wiznet Ethernet interface for µCNC.

	Copyright: Copyright (c) João Martins
	Author: João Martins
	Date: 09-07-2024

	µCNC is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. Please see <http://www.gnu.org/licenses/>

	µCNC is distributed WITHOUT ANY WARRANTY;
	Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
	See the	GNU General Public License for more details.
*/

#include "../../cnc.h"
#include "Ethernet/wizchip_conf.h"
#include "Internet/DHCP/wizchip_dhcp.h"
#include "Ethernet/wizchip_socket.h"

#ifndef TELNET_PORT
#define TELNET_PORT 23
#endif

#ifndef TELNET_SOCKET_N
#define TELNET_SOCKET_N 0
#endif

#ifndef WEBSERVER_PORT
#define WEBSERVER_PORT 80
#endif

#ifndef WEBSOCKET_PORT
#define WEBSOCKET_PORT 8080
#endif

#ifndef WEBSOCKET_MAX_CLIENTS
#define WEBSOCKET_MAX_CLIENTS 2
#endif

#ifdef WIZNET_HW_SPI
#define WIZNET_SPI MCU_SPI
#endif

#define ETH_USE_DHCP

#ifndef ETH_USE_DHCP
#define NETWORK_DHCP NETINFO_STATIC
#else
#define NETWORK_DHCP NETINFO_DHCP
#endif

/* Network */
static wiz_NetInfo g_net_info =
		{
				.mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56}, // MAC address
				.ip = {192, 168, 1, 90},										 // IP address
				.sn = {255, 255, 255, 0},										 // Subnet Mask
				.gw = {192, 168, 1, 1},											 // Gateway
				.dns = {8, 8, 8, 8},												 // DNS server
				.dhcp = NETWORK_DHCP												 // DHCP enable/disable
};

typedef struct client_socket_
{
	uint8_t socket;
	uint16_t port;
	ring_buffer_t *rx_buffer;
	ring_buffer_t *tx_buffer;
} client_socket_t;

/**
 *
 * Creates a raw TCP socket to stream the Grbl protocol
 * Creates a ring buffer to both TX and RX communications
 * Creates a serial stream and binds both TX and RX ring buffers
 *
 */

#ifndef ETH_TX_BUFFER_SIZE
#define ETH_TX_BUFFER_SIZE 64
#endif

// TX and RX ring buffers
DECL_BUFFER(uint8_t, eth_tx, ETH_TX_BUFFER_SIZE);
DECL_BUFFER(uint8_t, eth_rx, RX_BUFFER_SIZE);
static client_socket_t telnet_client;

// TX serial stream flush callback
// checks if the socket is opended and sends the data
// if not simply discards the TX buffer to prevent stalling (assumes there is no client or the connection failed)
void eth_serial_stream_flush(void)
{
	if (getSn_SR(telnet_client.socket) == SOCK_ESTABLISHED)
	{
		int16_t avail = 0;
		if ((avail = BUFFER_READ_AVAILABLE(eth_tx)) > 0)
		{
			uint8_t buffer[avail];
			BUFFER_READ(eth_tx, buffer, avail, avail);
			wizchip_send(telnet_client.socket, buffer, sizeof(buffer));
		}
	}
	else
	{
		BUFFER_CLEAR(eth_tx);
	}
}

// creates the serial streams calls and binds the TX and RX buffer
SERIAL_STREAM_CREATE_AND_BIND(eth_serial_stream, eth_tx, eth_rx, eth_serial_stream_flush);

/**
 *
 * DHCP client
 *
 */
#ifdef ETH_USE_DHCP

#define DATA_BUF_SIZE 2048
uint8_t gDATABUF[DATA_BUF_SIZE];

static FORCEINLINE bool wiznet_dhcp_leased(void)
{
	/**
	 * clock to the DHCP
	 */
	static uint32_t dhcp_timer = 0;

	if (dhcp_timer < mcu_millis())
	{
		dhcp_timer = mcu_millis() + 1000;
		DHCP_time_handler();
	}

	uint8_t dhcp = DHCP_run();
	if (dhcp != DHCP_IP_LEASED)
	{
		return false;
	}

	// DHCP leased. Can configure network
	getIPfromDHCP(g_net_info.ip);
	getGWfromDHCP(g_net_info.gw);
	getDNSfromDHCP(g_net_info.dns);
	getSNfromDHCP(g_net_info.sn);
	return true;
}

#endif

/**
 * Board init and status
 *
 */

static FORCEINLINE void wiznet_connect(void)
{
	ctlnetwork(CN_SET_NETINFO, (void *)&g_net_info);
}

static void socket_server_run(client_socket_t *client)
{
	int16_t ret = 0;
	int16_t avail = 0;

	// Check the socket status
	switch (getSn_SR(client->socket))
	{
	case SOCK_ESTABLISHED:
		// Check if data is available to read
		if ((ret = getSn_RX_RSR(client->socket)) > 0)
		{
			// check available space to read data
			avail = BUFFER_WRITE_AVAILABLE(client->rx_buffer);
			if (avail)
			{
				uint8_t buffer[avail];
				ret = wizchip_recv(client->socket, buffer, sizeof(buffer));
				for (int16_t i = 0; i < ret; i++)
				{
					uint8_t c = buffer[i];
					if (mcu_com_rx_cb(c))
					{
						BUFFER_ENQUEUE(eth_rx, &c);
					}
				}
			}
		}
		break;

	case SOCK_CLOSE_WAIT:
		// Close the socket when the connection is closed
		wizchip_close(client->socket);
		break;

	case SOCK_CLOSED:
		// Reopen the socket if it was closed
		if ((ret = wizchip_socket(client->socket, Sn_MR_TCP, client->port, SF_IO_NONBLOCK)) < 0)
		{
			// Handle socket error
			return;
		}

		// Put the socket into listen mode
		if ((ret = wizchip_listen(client->socket)) < 0)
		{
			// Handle listen error
			return;
		}
		break;
	}
}

static FORCEINLINE void wiznet_init(void)
{
	reg_wizchip_cris_cbfunc(w5XXX_critical_section_enter, w5XXX_critical_section_exit);
	reg_wizchip_cs_cbfunc(w5XXX_cs_select, w5XXX_cs_deselect);
	reg_wizchip_spi_cbfunc(w5XXX_getc, w5XXX_putc);
#ifdef USE_SPI_DMA
	reg_wizchip_spiburst_cbfunc(wiznet_read, wiznet_write);
#endif
	/* Deselect the FLASH : chip select high */
	w5XXX_cs_deselect();

	/* W5x00 initialize */
	uint8_t memsize[2][_WIZCHIP_SOCK_NUM_] = {{2, 2, 2, 2, 2, 2, 2, 2}, {2, 2, 2, 2, 2, 2, 2, 2}};

	if (ctlwizchip(CW_INIT_WIZCHIP, (void *)memsize) == -1)
	{
		protocol_send_feedback("Ethernet chip was not found");
		return;
	}

	ctlnetwork(CN_SET_NETINFO, (void *)&g_net_info);

#ifdef ETH_USE_DHCP
	if (g_net_info.dhcp == NETINFO_DHCP)
	{
		DHCP_init(_WIZCHIP_SOCK_NUM_ - 1, gDATABUF);
	}
#endif
}

static FORCEINLINE bool wiznet_connected(void)
{
	uint8_t temp;
	if (ctlwizchip(CW_GET_PHYLINK, (void *)&temp) < 0)
	{
		return false;
	}

	return (temp == PHY_LINK_ON);
}

bool eth_connection_ok(void)
{
	static uint32_t next_info = 30000;
	static bool connected = false;
	uint8_t str[64];

	if (!wiznet_connected())
	{
		connected = false;
		if (next_info > mcu_millis())
		{
			return false;
		}
		next_info = mcu_millis() + 30000;
		protocol_send_feedback("Disconnected from ETH");
		return false;
	}

	if (!connected)
	{
#ifdef ETH_USE_DHCP
		if (g_net_info.dhcp == NETINFO_DHCP)
		{
			if (!wiznet_dhcp_leased())
			{
				return false;
			}
		}
#endif

		wiznet_connect();
		protocol_send_feedback("Connected to ETH");
		sprintf((char *)str, "IP>%d.%d.%d.%d", g_net_info.ip[0], g_net_info.ip[1], g_net_info.ip[2], g_net_info.ip[3]);
		protocol_send_feedback((const char *)str);
		connected = true;
	}

	return true;
}

bool eth_loop(void *arg)
{
	// eth_telnet_server.statusreport();
	if (eth_connection_ok())
	{
		// handle the telnet client
		socket_server_run(&telnet_client);
	}

	return EVENT_CONTINUE;
}
CREATE_EVENT_LISTENER(cnc_alarm, eth_loop);
CREATE_EVENT_LISTENER(cnc_dotasks, eth_loop);

DECL_MODULE(wiznet_ethernet)
{

	io_config_output(WIZNET_CS);
	io_set_output(WIZNET_CS);
	softspi_config(WIZNET_SPI, 0, 14000000UL);

	wiznet_init();

	// setup telnet client
	telnet_client.socket = TELNET_SOCKET_N;
	telnet_client.port = TELNET_PORT;
	telnet_client.rx_buffer = &eth_rx;
	telnet_client.tx_buffer = &eth_tx;

	// register the ethernt stream buffer to the main protocol
	serial_stream_register(&eth_serial_stream);

	// network_initialize(g_net_info);
// 		// serial_stream_register(&web_pendant_stream);
// 		endpoint_add("/", 0, &web_pendant_request, NULL);

// ADD_EVENT_LISTENER(websocket_client_connected, web_pendant_ws_connected);
// ADD_EVENT_LISTENER(websocket_client_disconnected, web_pendant_ws_disconnected);
// ADD_EVENT_LISTENER(websocket_client_receive, web_pendant_ws_receive);

// serial_stream_register(&web_pendant_stream);
#ifdef ENABLE_MAIN_LOOP_MODULES
	ADD_EVENT_LISTENER(cnc_alarm, eth_loop);
	ADD_EVENT_LISTENER(cnc_dotasks, eth_loop);
#else
#warning "Main loop extensions are not enabled. Ethernet will not work."
#endif

	// #if (defined(ENABLE_PARSER_MODULES) || defined(BOARD_HAS_CUSTOM_SYSTEM_COMMANDS))
	// 	ADD_EVENT_LISTENER(grbl_cmd, eth_print_info);
	// #else
	// #warning "Parser extensions are not enabled. Ethernet Grbl commands will not work."
	// #endif
}

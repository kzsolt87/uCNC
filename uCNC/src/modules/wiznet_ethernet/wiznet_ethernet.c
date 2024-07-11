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
#include "Internet/DHCP/dhcp.h"
#include "Ethernet/socket.h"

#ifndef TELNET_PORT
#define TELNET_PORT 23
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

/* Network */
static wiz_NetInfo g_net_info =
		{
				.mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56}, // MAC address
				.ip = {192, 168, 1, 90},										 // IP address
				.sn = {255, 255, 255, 0},										 // Subnet Mask
				.gw = {192, 168, 1, 1},											 // Gateway
				.dns = {8, 8, 8, 8},												 // DNS server
				.dhcp = NETINFO_DHCP												 // DHCP enable/disable
};

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
#endif;

/**
 * Board init and status
 *
 */

static FORCEINLINE void wiznet_connect(void)
{
	ctlnetwork(CN_SET_NETINFO, (void *)&g_net_info);
}

static FORCEINLINE void wiznet_init(void)
{
	reg_wizchip_cris_cbfunc(wiznet_critical_section_enter, wiznet_critical_section_exit);
	reg_wizchip_cs_cbfunc(wiznet_cs_select, wiznet_cs_deselect);
	reg_wizchip_spi_cbfunc(wiznet_getc, wiznet_putc);
#ifdef USE_SPI_DMA
	reg_wizchip_spiburst_cbfunc(wiznet_read, wiznet_write);
#endif
	/* Deselect the FLASH : chip select high */
	wiznet_cs_deselect();

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
#endif;
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

bool eth_clientok(void)
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
		connected = true;
		protocol_send_feedback("Connected to ETH");
		sprintf((char *)str, "IP>%d.%d.%d.%d", g_net_info.ip[0], g_net_info.ip[1], g_net_info.ip[2], g_net_info.ip[3]);
		protocol_send_feedback((const char *)str);
		socket(0, Sn_MR_TCP, TELNET_PORT, 0);
		// eth_telnet_server.begin();
	}
	
	if(listen(0)){
		connect(0,)
	}
	// EthernetClient client = eth_telnet_server.available();
	// if (client)
	// {
	// 	if (eth_telnet_client)
	// 	{
	// 		if (eth_telnet_client.connected())
	// 		{
	// 			eth_telnet_client.stop();
	// 		}
	// 	}
	// 	eth_telnet_client = eth_telnet_server.accept();
	// 	eth_telnet_client.println("[MSG:New client connected]");
	// 	return false;
	// }
	// else if (eth_telnet_client)
	// {
	// 	if (eth_telnet_client.connected())
	// 	{
	// 		return true;
	// 	}
	// }
	return false;
}

bool eth_loop(void *arg)
{
	// eth_telnet_server.statusreport();
	if (eth_clientok())
	{
		// while (eth_telnet_client.available() > 0)
		// {
		// 	uint8_t c = eth_telnet_client.read();
		// 	if (mcu_com_rx_cb(c))
		// 	{
		// 		if (BUFFER_FULL(eth_rx))
		// 		{
		// 			c = OVF;
		// 		}

		// 		BUFFER_ENQUEUE(eth_rx, &c);
		// 	}
		// }
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

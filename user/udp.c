/*
* xPL for ESP8266
*
* UDP send/receive routines, based on the LWIP stack
* 
*
* 
* Copyright (C) 2014  Pierre Benard xsc.peteben@neverbox.com
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "esp_common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "lwip/opt.h"
#include "lwip/timers.h"
#include "lwip/udp.h"
#include "lwip/dns.h"
#include "lwip/ip_addr.h"
#include "lwip/pbuf.h"
#include "xPL.h"

xQueueHandle udpQ;		// Incoming UPD messages are stuffed into this queue for eventual consumption by the xPL device task

// Callback routine for incomping UDP packets
static void ICACHE_FLASH_ATTR udp_recv_cb(void *arg, struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr, u16_t port) {
	if (p != NULL) {
		char *packet = strdup(p->payload);	// Make our

		if (strlen(packet) > 0) {
			xQueueSendToBack(udpQ, &packet, portMAX_DELAY);
			}
		}

	pbuf_free(p);
	}

// UDP send routine
int ICACHE_FLASH_ATTR udpio_send(const char *buf, int port) {
	struct udp_pcb *pcb = udp_new();
	struct pbuf *pb = pbuf_alloc(PBUF_TRANSPORT, strlen(buf) + 1, PBUF_RAM);

	strcpy(pb->payload, buf);
	int err = udp_sendto(pcb, pb, IP_ADDR_BROADCAST, port);
	if (err != 0) printf(" Err=%d ", err);

	pbuf_free(pb);
	udp_remove(pcb);
	}

// Initialize UDP io by creating the Queue and registering the receive callback
void ICACHE_FLASH_ATTR udpio_init(void) {
	struct udp_pcb *pcb = udp_new();

	udpQ = xQueueCreate(4, sizeof(char *));

	udp_bind(pcb, IP_ADDR_ANY, XPL_UDP_PORT);
	udp_recv(pcb, udp_recv_cb, NULL);
	}




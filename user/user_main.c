/*
* xPL for ESP8266
* Main function of application
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
#include "UserConfig.h"
#include "xPL.h"


void udpio_init(void);
void debounce_init(void);

// Our IP address
struct ip_info ipinfo;

// Initial task. Waits for connection and for an IP address before starting the other tasks.
// Keeps checking for connection afterwards

void ICACHE_FLASH_ATTR connect_task(void *pvParameters) {
	portTickType xLastWakeTime = xTaskGetTickCount();
	portTickType xFrequency = 1000;				// Every 10 s
	struct station_config stationConf;

	unsigned char cnt = 0;
	unsigned char isinit = 0;

	memset(&stationConf, 0, sizeof(stationConf));
	strcpy(stationConf.ssid, MYSSID);
	strcpy(stationConf.password, MYPASSPHRASE);

	//Set station mode
	wifi_set_opmode(0x1);
	wifi_station_set_config(&stationConf);
	
	for (;;) {
		// Wait for the next cycle.
		vTaskDelayUntil(&xLastWakeTime, xFrequency);

		if (wifi_station_get_connect_status() == STATION_GOT_IP) {		// Got an IP?
			wifi_get_ip_info(0, &ipinfo);								// Get it

			if (!isinit) {												// Start the tasks once we have our IP address
				xPL_SetSource(xPL_VENDORID, xPL_DEVICEID, xPL_INSTANCEID);
				xPL_init();
				udpio_init();
				debounce_init();
				isinit = 1;
				xFrequency = 6000;					// Once a minute from now on
				}
			}
		else {										// Lost our connection?
			printf("Checking IP - %d\r", cnt);
			xFrequency = 1000;						// Check every 10s

			if (cnt++ > 50) {						// Try to reconnect
				wifi_station_disconnect();
				vTaskDelay(100);
				wifi_station_connect();
				printf("Reconnecting\n");
				cnt = 0;
				}
			}
		}
	}

//Init function
//User program begins here
void ICACHE_FLASH_ATTR user_init(void) {
	printf("SDK version:%d.%d.%d\n",
		SDK_VERSION_MAJOR,
		SDK_VERSION_MINOR,
		SDK_VERSION_REVISION);

	xTaskCreate(connect_task, "conn", 256, NULL, 2, NULL);
	}

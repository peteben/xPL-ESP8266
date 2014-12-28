/*
* xPL for ESP8266
* 
* This file holds the user functions for the ESP8266 xPL implementation.
* process_message gets called whenever a properly formated xPL message is received.
* this example checks for an X10.BASIC ON or OFF command and turns GPIO0 on or off to control an LED
*
* xPL_send_trigger gets called by the input debounce routine whenever a transition is detected on GPIO2
* it then formats an X10 ON or OFF trigger message and sends it

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

#include "xPL_utils.h"
#include "xPL_Message.h"
#include "xPL.h"
#include <string.h>
#include "UserConfig.h"


#define CMD_ALL_UNITS_OFF     0
#define CMD_ALL_LIGHTS_ON     1
#define CMD_ON                2
#define CMD_OFF               3
#define CMD_DIM               4
#define CMD_BRIGHT            5
#define CMD_ALL_LIGHTS_OFF    6
#define CMD_EXTENDED_CODE     7
#define CMD_HAIL_REQUEST      8
#define CMD_HAIL_ACKNOWLEDGE  9
#define CMD_PRE_SET_DIM_0     10
#define CMD_PRE_SET_DIM_1     11
#define CMD_EXTENDED_DATA     12
#define CMD_STATUS_ON         13
#define CMD_STATUS_OFF        14
#define CMD_STATUS_REQUEST    15


enum X10_COMMANDS {
	ALL_UNITS_OFF = 0,
	ALL_LIGHTS_ON = 1,
	ON = 2,
	OFF = 3,
	DIM = 4,
	BRIGHT = 5,
	ALL_LIGHTS_OFF = 6,
	EXTENDED = 7,
	HAIL_REQ = 8,
	HAIL_ACK = 9,
	PRESET_DIM1 = 10,
	PRESET_DIM2 = 11,
	EXTENDED_DATA = 12,
	STATUS_ON = 13,
	STATUS_OFF = 14,
	STATUS_REQUEST = 15
	};

const char ICACHE_FLASH_ATTR *X10ToString(byte cmd) {
	const char *command;
	switch ((enum X10_COMMANDS)cmd) {
		case ALL_LIGHTS_OFF: command = "ALL_LIGHTS_OFF"; break;
		case ALL_LIGHTS_ON: command = "ALL_LIGHTS_ON"; break;
		case ON: command = "ON"; break;
		case OFF: command = "OFF"; break;
		case DIM: command = "DIM"; break;
		case BRIGHT: command = "BRIGHT"; break;
		case ALL_UNITS_OFF: command = "ALL_UNITS_OFF"; break;
		case EXTENDED: command = "EXTENDED"; break;
		case HAIL_REQ: command = "HAIL_REQ"; break;
		case HAIL_ACK: command = "HAIL_ACK"; break;
		case PRESET_DIM1: command = "PREDIM1"; break;
		case PRESET_DIM2: command = "PREDIM2"; break;
		case EXTENDED_DATA: command = "EXTENDED_DATA"; break;
		case STATUS_ON: command = "STATUS_ON"; break;
		case STATUS_OFF: command = "STATUS_OFF"; break;
		case STATUS_REQUEST: command = "STATUS_REQUEST"; break;
		default: command = "unknown"; break;
		}
	return command;
	}

// ESP8266 libs lack this
int toupper(int c) {
	return c >= 'a' && c <= 'z' ? (c - 'a' + 'A') : c;
	}

// Process a received xPL message
void ICACHE_FLASH_ATTR process_message(xPL_Message *msg) {
	unsigned char house = 0, unit = 0, X10cmd = 15,  i;

	if (xPL_TargetIsMe(msg)) {
		if (xPL_Message_IsSchema(msg, "x10", "basic") && msg->type == XPL_CMND) {
			struct_command *cmd = msg->command;

			for (i = 0; i < msg->command_count; i++) {
				if (strcasecmp(cmd->name, "device") == 0) {
					house = toupper(cmd->value[0]);
					unit = atoi(cmd->value + 1);
					}

				if (strcasecmp(cmd->name, "command") == 0) {
					unsigned char j;
					for (j = 0; j < 15; j++) {
						if (strcasecmp(cmd->value, X10ToString(j)) == 0) {
							X10cmd = j;
							break;
							}
						}
					}
				cmd++;
				}

			if (house == MYHOUSE && unit == MYUNIT) {
				if (X10cmd == CMD_ON) {
					gpio_output_set(0, LED_GPIO, LED_GPIO, 0);
					}

				if (X10cmd == CMD_OFF) {
					gpio_output_set(LED_GPIO, 0, LED_GPIO, 0);
					}
				}
			}
		}
	}

// Send an X10 trigger message
void ICACHE_FLASH_ATTR xPL_send_trigger(unsigned char house, unsigned char unit, unsigned char state) {
	xPL_Message *msg = new_xPL_Message();
	char device[4];

	msg->type = XPL_TRIG;
	msg->hop = 1;

	xPL_Message_SetTarget(msg, "*", NULL, NULL);
	xPL_Message_SetSchema(msg, "x10", "basic");

	sprintf(device, "%c%d", house, unit);
	xPL_Message_AddCommand(msg, "command", state ? "on" : "off");
	xPL_Message_AddCommand(msg, "device", device);
	xPL_SendMessage(msg, true);
	free_xPL_Message(msg);
	}

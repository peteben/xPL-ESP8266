/*
 * 
 * xPL for ESP8266, converted from:
 * xPL.Arduino v0.1, xPL Implementation for Arduino
 *
 * This code is parsing a xPL message stored in 'received' buffer
 * - isolate and store in 'line' buffer each part of the message -> detection of EOL character (DEC 10)
 * - analyse 'line', function of its number and store information in xpl_header memory
 * - check for each step if the message respect xPL protocol
 * - parse each command line
 *
 * Copyright (C) 2012 johan@pirlouit.ch, olivier.lebrun@gmail.com
 * Original version by Gromain59@gmail.com
 * Converted to straight C for the ESP8266 Wifi module by Pierre Benard xsc.peteben@neverbox.com
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

#include "xPL_Message.h"
#include <stdio.h>

xPL_Message * ICACHE_FLASH_ATTR new_xPL_Message(void) {
	return zalloc(sizeof(xPL_Message));
	}

void ICACHE_FLASH_ATTR free_xPL_Message(xPL_Message *this) {
	if (this->command != NULL) {
		free(this->command);
		}
	free(this);
	}

/**
 * \brief       Set source of the message (optional)
 * \param    _vendorId         vendor id.
 * \param    _deviceId         device id.
 * \param    _instanceId      instance id.
 */
void ICACHE_FLASH_ATTR xPL_Message_SetSource(xPL_Message *this, const char * _vendorId, const char * _deviceId, const char * _instanceId) {
	strlcpy(this->source.vendor_id, _vendorId, XPL_VENDOR_ID_MAX + 1);
	strlcpy(this->source.device_id, _deviceId, XPL_DEVICE_ID_MAX + 1);
	strlcpy(this->source.instance_id, _instanceId, XPL_INSTANCE_ID_MAX + 1);
	}

/**
 * \brief       Set Target of the message
 * \details	  insert "*" into _vendorId to broadcast the message
 * \param    _vendorId         vendor id.
 * \param    _deviceId         device id.		(optional)
 * \param    _instanceId      instance id.    (optional)
 */
void ICACHE_FLASH_ATTR xPL_Message_SetTarget(xPL_Message *this, const char *_vendorId, const char *_deviceId, const char *_instanceId) {
	strlcpy(this->target.vendor_id, _vendorId, XPL_VENDOR_ID_MAX + 1);
	if (_deviceId != NULL) strlcpy(this->target.device_id, _deviceId, XPL_DEVICE_ID_MAX + 1);
	if (_instanceId != NULL) strlcpy(this->target.instance_id, _instanceId, XPL_INSTANCE_ID_MAX + 1);
	}

/**
 * \brief       Set Schema of the message
  * \param   _classId       Class
 * \param    _typeId         Type
  */
void ICACHE_FLASH_ATTR xPL_Message_SetSchema(xPL_Message *this, const char * _classId, const char * _typeId) {
	strlcpy(this->schema.class_id, _classId, XPL_CLASS_ID_MAX + 1);
	strlcpy(this->schema.type_id, _typeId, XPL_TYPE_ID_MAX + 1);
	}

/**
 * \brief       Create a new command/value pair
 * \details	  Check if maximun command is reach and add memory to command array
 */
bool ICACHE_FLASH_ATTR xPL_Message_CreateCommand(xPL_Message *this) {
	struct_command	*ncommand;
	// Maximun command reach
	// To avoid oom, we arbitrary accept only XPL_MESSAGE_COMMAND_MAX command
	if (this->command_count > XPL_MESSAGE_COMMAND_MAX)
		return false;

	ncommand = (struct_command*) malloc((this->command_count + 1) * sizeof(struct_command));
	
	if (ncommand != NULL) {
		memcpy(ncommand, this->command, this->command_count * sizeof(struct_command));
		free(this->command);

		this->command = ncommand;
		this->command_count++;
		return true;
		}
	else
		return false;
	}


/**
 * \brief       Add a command to the message's body
 * \details	  char* Version
 * \param    _name         name of the command
 * \param    _value         value of the command
 */
bool ICACHE_FLASH_ATTR xPL_Message_AddCommand(xPL_Message *this, const char* _name, const char* _value) {
	if(!xPL_Message_CreateCommand(this)) return false;

	struct_command newcmd;
	strlcpy(newcmd.name, _name, XPL_NAME_LENGTH_MAX + 1);
	strlcpy(newcmd.value, _value, XPL_VALUE_LENGTH_MAX + 1);
	this->command[this->command_count - 1] = newcmd;
	return true;
	}

/**
 * \brief       Convert xPL_Message to char* buffer
 */
void ICACHE_FLASH_ATTR xPL_Message_toString(xPL_Message *this, char message_buffer[]) {
	int pos;
	unsigned char i;

	message_buffer[0] = '\0';

	switch (this->type) {
		case (XPL_CMND):
			pos = sprintf(message_buffer, "xpl-cmnd");
			break;
		case (XPL_STAT):
			pos = sprintf(message_buffer, "xpl-stat");
			break;
		case (XPL_TRIG):
			pos = sprintf(message_buffer, "xpl-trig");
			break;
		}

	pos += sprintf(message_buffer + pos, "\n{\nhop=1\nsource=%s-%s.%s\ntarget="
		, this->source.vendor_id, this->source.device_id, this->source.instance_id);

	if (this->target.vendor_id[0] == '*') { // check if broadcast message
		pos += sprintf(message_buffer + pos, "*\n}\n");
		}
	else {
		pos += sprintf(message_buffer + pos, "%s-%s.%s\n}\n"
			, this->target.vendor_id, this->target.device_id, this->target.instance_id);
		}

	pos += sprintf(message_buffer + pos, "%s.%s\n{\n", this->schema.class_id, this->schema.type_id);

	for (i = 0; i < this->command_count; i++) {
		pos += sprintf(message_buffer + pos, "%s=%s\n", this->command[i].name, this->command[i].value);
		}

	sprintf(message_buffer + pos, "}\n");
	}


/**
 * \brief       Check the message's schema
  * \param   _classId        class
 * \param    _typeId         type
 */
bool ICACHE_FLASH_ATTR xPL_Message_IsSchema(xPL_Message *this, const char * _classId, const char* _typeId) {
	if (strcasecmp(this->schema.class_id, _classId) == 0) {
		if (strcasecmp(this->schema.type_id, _typeId) == 0) {
			return true;
			}
		}
	return false;
	}

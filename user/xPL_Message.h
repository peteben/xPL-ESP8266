/*
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

#ifndef xPLMessage_h
#define xPLMessage_h

#include "xPL_utils.h"

#define XPL_CMND 1
#define XPL_STAT 2
#define XPL_TRIG 3

#define XPL_MESSAGE_BUFFER_MAX           256  // going over 256 would mean changing index from unsigned char to int
#define XPL_MESSAGE_COMMAND_MAX          10

struct xPL_Message {
	short type;			        // 1=cmnd, 2=stat, 3=trig
	short hop;					// Hop count

	struct_id source;			// source identification
	struct_id target;			// target identification

	struct_xpl_schema schema;
	struct_command *command;
	unsigned char command_count;
	};

typedef struct xPL_Message xPL_Message;

bool xPL_Message_AddCommand(xPL_Message *this, const char* _name, const char* _value);

xPL_Message *new_xPL_Message(void);
void free_xPL_Message_(xPL_Message *this);

void xPL_Message_toString(xPL_Message *this, char message_buffer[]);

bool xPL_Message_IsSchema(xPL_Message *this, const char * _classId, const char* _typeId);
		
void xPL_Message_SetSource(xPL_Message *this, const char *x, const char *, const char *y);  // define my source
void xPL_Message_SetTarget(xPL_Message *this, const char *_vendorId, const char *_deviceId, const char *_instanceId);
void xPL_Message_SetSchema(xPL_Message *this, const char *x, const char *y);
bool xPL_Message_CreateCommand(xPL_Message *this);


#endif

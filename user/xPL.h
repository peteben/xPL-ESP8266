/*
 * xPL.Arduino v0.1, xPL Implementation for Arduino
 *
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

#ifndef xPL_h
#define xPL_h

#define ENABLE_PARSING 1

#include "xPL_utils.h"
#include "xPL_Message.h"

#define XPL_CMND 1
#define XPL_STAT 2
#define XPL_TRIG 3

#define XPL_DEFAULT_HEARTBEAT_INTERVAL   60

#define XPL_UDP_PORT 3865

#define XPL_PORT_L  0x19
#define XPL_PORT_H  0xF

typedef enum {XPL_ACCEPT_ALL, XPL_ACCEPT_SELF, XPL_ACCEPT_SELF_ANY} xpl_accepted_type;
// XPL_ACCEPT_ALL = all xpl messages
// XPL_ACCEPT_SELF = only for me
// XPL_ACCEPT_SELF_ANY = only for me and any (*)

xPL_Message *xPL_ParseInputMessage(const char *buffer);
bool xPL_TargetIsMe(xPL_Message * message);
void xPL_SendHBeat();
bool xPL_CheckHBeatRequest(xPL_Message * message);
void xPL_Parse(xPL_Message *, const char *);
unsigned char xPL_AnalyseHeaderLine(xPL_Message *, const char *, unsigned char);
unsigned char xPL_AnalyseCommandLine(xPL_Message *, const char *, unsigned char, unsigned char);
void xPL_SendMessageBuf(const char *);
void xPL_SendMessage(xPL_Message *, bool);
void xPL_SetSource(const char *x, const char *y, const char *z);  // define my source

struct xPL {
	struct_id source;  // my source
	unsigned char hbeat_interval;  // default 5
	xpl_accepted_type xpl_accepted;
	unsigned long last_heartbeat;
	};

typedef struct xPL xPL;
extern xPL xPL_device;

#endif

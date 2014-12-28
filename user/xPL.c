/*
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

#include "xPL.h"
#include "freertos/FreeRTOS.h"
#include <freertos/task.h>
#include <freertos/queue.h>
#include <stdio.h>

#define XPL_LINE_MESSAGE_BUFFER_MAX			128	// max length of a line			// maximum command in a xpl message
#define XPL_END_OF_LINE						10

// define the line number identifier
#define XPL_MESSAGE_TYPE_IDENTIFIER	        1
#define XPL_OPEN_HEADER						2
#define XPL_HOP_COUNT						3
#define XPL_SOURCE							4
#define XPL_TARGET							5
#define XPL_CLOSE_HEADER					6
#define XPL_SCHEMA_IDENTIFIER		        7
#define XPL_OPEN_SCHEMA						8

// Heartbeat request class definition
const char XPL_HBEAT_REQUEST_CLASS_ID[] = "hbeat";
const char XPL_HBEAT_REQUEST_TYPE_ID[] = "request";
#define XPL_HBEAT_ANSWER_CLASS_ID  "hbeat"
#define XPL_HBEAT_ANSWER_TYPE_ID  "app"

extern struct ip_info ipinfo;			// Struct holding our IP address
extern xQueueHandle udpQ;				// Queue with received UDP packets

extern void process_message(xPL_Message *msg);		// User routine to do something with the received xPL messages

struct xPL xPL_device;					// The device

/**
* \brief     HeartBeat task
* \details   Send heartbeat messages at "hbeat_interval" interval
*/
void ICACHE_FLASH_ATTR xPL_hbeat_task(void *pvParameters) {
	portTickType xLastWakeTime = xTaskGetTickCount();
	portTickType xFrequency = xPL_device.hbeat_interval * 100;				// Once a minute

	for (;;) {
		if (ipinfo.ip.addr != 0) {
			xPL_SendHBeat();
			}
		// Wait for the next cycle.
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
		}
	}

// This task takes the received UDP packets, parses them, then passes them to the user routine for action

void ICACHE_FLASH_ATTR xPL_recv_task(void *pvParameters) {
	char *buf;

	for (;;) {
		xQueueReceive(udpQ, &buf, portMAX_DELAY);
		if (buf != NULL && strlen(buf) > 0) {
			xPL_Message *msg = xPL_ParseInputMessage(buf);

			process_message(msg);			// User process routine

			free_xPL_Message(msg);
			free(buf);
			}
		}
	}

// Setup the xPL device object, start the hartbeat and receive tasks
void ICACHE_FLASH_ATTR xPL_init() {
	xPL_device.last_heartbeat = 0;
	xPL_device.hbeat_interval = XPL_DEFAULT_HEARTBEAT_INTERVAL;
	xPL_device.xpl_accepted = XPL_ACCEPT_ALL;

	xTaskCreate(xPL_hbeat_task, "Hbt", 512, NULL, 2, NULL);
	xTaskCreate(xPL_recv_task, "recv", 512, NULL, 2, NULL);
	}


/// Set the source of outgoing xPL messages
void ICACHE_FLASH_ATTR xPL_SetSource(const char * _vendorId, const char * _deviceId, const char * _instanceId) {
	strlcpy(xPL_device.source.vendor_id, _vendorId, XPL_VENDOR_ID_MAX);
	strlcpy(xPL_device.source.device_id, _deviceId, XPL_DEVICE_ID_MAX);
	strlcpy(xPL_device.source.instance_id, _instanceId, XPL_INSTANCE_ID_MAX);
	}

/**
 * \brief       Send an xPL message
 * \details   There is no validation of the message, it is sent as is.
 * \param    buffer         buffer containing the xPL message.
 */
void ICACHE_FLASH_ATTR xPL_SendMessageBuf(const char *_buffer) {
	udpio_send(_buffer, XPL_UDP_PORT);
	}

/**
 * \brief       Send an xPL message
 * \details   There is no validation of the message, it is sent as is.
 * \param    message         			An xPL message.
 * \param    _useDefaultSource	if true, insert the default source (defined in SetSource) on the message.
 */
void ICACHE_FLASH_ATTR xPL_SendMessage(xPL_Message *_message, bool _useDefaultSource) {
	char *xPLMessageBuff = malloc(XPL_MESSAGE_BUFFER_MAX);		// Save stack space by creating on heap

	if(_useDefaultSource) {
		xPL_Message_SetSource(_message, xPL_device.source.vendor_id, xPL_device.source.device_id, xPL_device.source.instance_id);
		}

	xPL_Message_toString(_message, xPLMessageBuff);
	//printf("Sending: %s\n", xPLMessageBuff);
	xPL_SendMessageBuf(xPLMessageBuff);
	free(xPLMessageBuff);
	}

/**
 * \brief       Parse an ingoing xPL message
 * \details   Parse a message, check for hearbeat request and call user defined callback for post processing.
 * \param    buffer         buffer of the ingoing UDP Packet
 */
xPL_Message ICACHE_FLASH_ATTR *xPL_ParseInputMessage(const char* _buffer) {
	xPL_Message *xPLMessage =  new_xPL_Message();

	//printf("message %s\n", _buffer);

	xPL_Parse(xPLMessage, _buffer);

	// check if the message is an hbeat.request to send a heartbeat
	if (xPL_CheckHBeatRequest(xPLMessage)) {
		xPL_SendHBeat();
		}
	return xPLMessage;
	}

/**
 * \brief       Check the xPL message target
 * \details   Check if the xPL message is for us
 * \param    _message         an xPL message
 */
bool ICACHE_FLASH_ATTR xPL_TargetIsMe(xPL_Message * _message) {
	if (_message->target.vendor_id[0] == '*') 
		return true;

	if (strcmp(_message->target.vendor_id, xPL_device.source.vendor_id) != 0)
		return false;

	if (strcmp(_message->target.device_id, xPL_device.source.device_id) != 0)
		return false;

	if (strcmp(_message->target.instance_id, xPL_device.source.instance_id) != 0)
		return false;

	return true;
	}

/**
 * \brief       Send a heartbeat message
  */
void ICACHE_FLASH_ATTR xPL_SendHBeat() {
	char *ipptr = (char *)&ipinfo.ip;
	char *buffer = malloc(XPL_MESSAGE_BUFFER_MAX);			// On heap, to save stack space

	sprintf(buffer, 
			"xpl-stat\n{\n"
			"hop=1\n"
			"source=%s-%s.%s\n"
			"target=*\n}\n"
			XPL_HBEAT_ANSWER_CLASS_ID "." XPL_HBEAT_ANSWER_TYPE_ID "\n{\n"
			"interval=%d\n"
			"port=3865\n"
			"remote-ip=%d.%d.%d.%d\n"
			"version=1.0\n}\n"
			, xPL_device.source.vendor_id, xPL_device.source.device_id, xPL_device.source.instance_id,
			xPL_device.hbeat_interval, ipptr[0],ipptr[1], ipptr[2], ipptr[3]);

	xPL_SendMessageBuf(buffer);
	free(buffer);
	}

/**
 * \brief       Check if the message is a heartbeat request
  * \param    _message         an xPL message
 */
bool ICACHE_FLASH_ATTR xPL_CheckHBeatRequest(xPL_Message* _message) {
	if (!xPL_TargetIsMe(_message))
		return false;

	return xPL_Message_IsSchema(_message, XPL_HBEAT_REQUEST_CLASS_ID, XPL_HBEAT_REQUEST_TYPE_ID);
	}

/**
 * \brief       Parse a buffer and generate a xPL_Message
 * \details	  Line based xPL parser
 * \param    _xPLMessage    the result xPL message
 * \param    _message         the buffer
 */
void ICACHE_FLASH_ATTR xPL_Parse(xPL_Message* _xPLMessage, const char* _buffer) {
	int len = strlen(_buffer);
	byte i, j=0;
	byte line=0;
	int result=0;
	char *lineBuffer = malloc(XPL_LINE_MESSAGE_BUFFER_MAX+1);

	// read each character of the message
	for(i = 0; i < len; i++) {
		// load byte by byte in 'line' buffer, until '\n' is detected
		if(_buffer[i] == XPL_END_OF_LINE) { // is it a linefeed (ASCII: 10 decimal)
			++line;
			lineBuffer[j]='\0';			// add the end of string id

			if(line <= XPL_OPEN_SCHEMA) {
				// first part: header and schema determination
				// we analyse the line, function of the line number in the xpl message
				result = xPL_AnalyseHeaderLine(_xPLMessage, lineBuffer ,line);
				}

			if(line > XPL_OPEN_SCHEMA) {
				// second part: command line
				// we analyse the specific command line, function of the line number in the xpl message
				result = xPL_AnalyseCommandLine(_xPLMessage, lineBuffer, line-9, j);

				if(result == _xPLMessage->command_count+1)
					break;
				}

			if (result < 0) break;

			j = 0; // reset the buffer pointer
			lineBuffer[0] = '\0'; // clear the buffer
			}
		else {
			// next character
			lineBuffer[j++] = _buffer[i];
			}
		}
	free(lineBuffer);
	}

/**
 * \brief       Parse the header part of the xPL message line by line
 * \param    _xPLMessage    the result xPL message
 * \param    _buffer         	   the line to parse
 * \param    _line         	       the line number
 */
byte ICACHE_FLASH_ATTR xPL_AnalyseHeaderLine(xPL_Message* _xPLMessage, const char* _buffer, byte _line) {
	int hopval;

	switch (_line) {
		case XPL_MESSAGE_TYPE_IDENTIFIER:				//message type identifier
			if (strncmp(_buffer,"xpl-", 4) == 0) {		//xpl
				if (strncmp(_buffer+4, "cmnd", 4)==0) { //command type
					_xPLMessage->type = XPL_CMND;			//xpl-cmnd
					}
				else if (strncmp(_buffer+4, "stat", 4) == 0) { //statut type
					_xPLMessage->type = XPL_STAT;			//xpl-stat
					}
				else if (strncmp(_buffer+4, "trig", 4) == 0) { // trigger type
					_xPLMessage->type = XPL_TRIG;			//xpl-trig
					}
				}
			else {
				return 0;  //unknown message
				}

			return 1;
			break;

		case XPL_OPEN_HEADER: //header begin
			if (_buffer[0] == '{') {
				return 2;
				}
			else {
				return -2;
				}
			break;

		case XPL_HOP_COUNT: //hop
			if (sscanf(_buffer, XPL_HOP_COUNT_PARSER, &hopval)) {
				_xPLMessage->hop = hopval;
				return 3;
				}
			else {
				return -3;
				}
			break;

		case XPL_SOURCE: //source
			if (sscanf(_buffer, XPL_SOURCE_PARSER, &_xPLMessage->source.vendor_id, &_xPLMessage->source.device_id, &_xPLMessage->source.instance_id) == 3) {
				return 4;
				}
			else {
				return -4;
				}
			break;

		case XPL_TARGET: //target
			if (sscanf(_buffer, XPL_TARGET_PARSER, &_xPLMessage->target.vendor_id, &_xPLMessage->target.device_id, &_xPLMessage->target.instance_id) == 3) {
				return 5;
				}
			else {
				if(_xPLMessage->target.vendor_id[0] == '*') { // check if broadcast message
					return 5;
					}
				else {
					return -5;
					}
				}
			break;

		case XPL_CLOSE_HEADER: //header end
			if (_buffer[0] == '}') {
				return 6;
				}
			else {
				return -6;
				}
			break;

		case XPL_SCHEMA_IDENTIFIER: //schema			
			sscanf(_buffer, XPL_SCHEMA_PARSER, &_xPLMessage->schema.class_id, &_xPLMessage->schema.type_id);
			return 7;
			break;

		case XPL_OPEN_SCHEMA: //header begin
			if (_buffer[0] == '{') {
				return 8;
				}
			else {
				return -8;
				}
			break;
		}
	return -100;
	}

/**
 * \brief       Parse the body part of the xPL message line by line
 * \param    _xPLMessage    				   the result xPL message
 * \param    _buffer         	  				   the line to parse
 * \param    _command_line       	       the line number
 */
byte ICACHE_FLASH_ATTR xPL_AnalyseCommandLine(xPL_Message * _xPLMessage, const char *_buffer, byte _command_line, byte line_length) {
	if (_buffer[0] == '}') { // End of schema
		return _xPLMessage->command_count+1;
		}
	else {	// parse the next command
		struct_command newcmd;

		sscanf(_buffer, XPL_COMMAND_PARSER, &newcmd.name, &newcmd.value);
		xPL_Message_AddCommand(_xPLMessage, newcmd.name, newcmd.value);

		return _command_line;
		}
	}

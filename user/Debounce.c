/*
* xPL for ESP8266
*
* This file holds a simple debouncing routine
* See below for original author
* 
* It runs as a task under FreeRTOS, and gets called every 100ms 
* A counter is incremented or decremented depending on the value of the input GPIO pin
* When the counter reaches 0, or a Maximun count, the input is deemed to have changed and
* a trigger is sent.

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

#include <esp_common.h>
#include "freertos/FreeRTOS.h"
#include <freertos/task.h>
#include <freertos/queue.h>
#include <stdio.h>
#include <string.h>
#include "UserConfig.h"


/******************************************************************************

debounce.c
written by Kenneth A. Kuhn
version 1.00

This is an algorithm that debounces or removes random or spurious
transistions of a digital signal read as an input by a computer.  This is
particularly applicable when the input is from a mechanical contact.  An
integrator is used to perform a time hysterisis so that the signal must
persistantly be in a logical state (0 or 1) in order for the output to change
to that state.  Random transitions of the input will not affect the output
except in the rare case where statistical clustering is longer than the
specified integration time.

The following example illustrates how this algorithm works.  The sequence
labeled, real signal, represents the real intended signal with no noise.  The
sequence labeled, corrupted, has significant random transitions added to the
real signal.  The sequence labled, integrator, represents the algorithm
integrator which is constrained to be between 0 and 3.  The sequence labeled,

output, only makes a transition when the integrator reaches either 0 or 3.
Note that the output signal lags the input signal by the integration time but
is free of spurious transitions.

real signal 0000111111110000000111111100000000011111111110000000000111111100000

corrupted   0100111011011001000011011010001001011100101111000100010111011100010
integrator  0100123233233212100012123232101001012321212333210100010123233321010
output      0000001111111111100000001111100000000111111111110000000001111111000

I have been using this algorithm for years and I show it here as a code
fragment in C.  The algorithm has been around for many years but does not seem
to be widely known.  Once in a rare while it is published in a tech note.  It
is notable that the algorithm uses integration as opposed to edge logic
(differentiation).  It is the integration that makes this algorithm so robust
in the presence of noise.
******************************************************************************/


// Threshold for trigger detection
#define THRESHOLD 4

extern void xPL_send_trigger(unsigned char house, unsigned char unit, unsigned char state);

// No ICACHE_FLASH_ATTR here, we want this routine to reside in RAM since it gets called so often

void DebounceTask(void *Stuff) {
	unsigned char b0 = 0, Button0 = 0;
	portTickType xLastWakeTime = xTaskGetTickCount();

	gpio_output_set(0, 0, 0, INPUT_GPIO);			// Set pin for input

	for (;;) {
		// Wait for the next cycle.
		vTaskDelayUntil(&xLastWakeTime, 10);		// 1/10 s

		if (GPIO_REG_READ(GPIO_IN_ADDRESS) & INPUT_GPIO) {
			if (b0 > 0) b0--;						// Decrement the counter if contact is open
			}
		else {
			if (b0 < THRESHOLD) b0++;				// Increment if closed
			}

		if (b0 == 0 && Button0 != 0) {				
			Button0 = 0;
			xPL_send_trigger(MYHOUSE, MYUNIT, 0);			// Send off trigger
			}

		if (b0 >= THRESHOLD && Button0 != 1) {
			Button0 = 1;
			xPL_send_trigger(MYHOUSE, MYUNIT, 1);			// Send on trigger
			}
		}
    }
    
void ICACHE_FLASH_ATTR debounce_init(void) {
	xTaskCreate(DebounceTask, "Deb", 256, NULL, 2, NULL);
	}

xPL-ESP8266
===========

##Sample xPL program for the ESP8266

This small program illustrates how to setup a development environment under Windows, optionally using Microsoft Visual Studio as an IDE.
For more details, refer to the wiki document, [Getting started](../../wiki/Getting-started)

###What is an ESP8266?
The ESP8266 is a small, inexpensive (~$3) chipset designed to connect embedded processors to a Wifi network. Its small size and low price make it ideal for IoT (Internet of Things) applications. Out of the box, it is designed to communicate with a host processor using a simple serial interface, using modem-like ‘AT’ commands. Many users have used this to interface with Arduinos and similar processors.
What has the hacker community excited though, is the onboard 32 bit processor, which is user accessible. With the hard work of the [pioneers from the ESP8266 online community](http://www.esp8266.com/), some tools and an SDK have been made available.
Most [ESP8266 modules](http://l0l.org.uk/2014/12/esp8266-modules-hardware-guide-gotta-catch-em-all/) have 4mb of flash memory onboard, with about 240kB-440kB available for programs, and about 80kB of RAM, which makes this module more powerful than most Arduinos. Also, anywhere between 0 and 9 GPIOs are available, with SPI, I2C available in addition to the standard serial port.

###What is xPL?

[xPL](http://xplproject.org.uk/) is an open protocol intended to permit the control and monitoring of home automation devices. The primary design goal of xPL is to provide a rich set of features and functionality, whilst maintaining an elegant, uncomplicated message structure. The protocol includes complete discovery and auto-configuration capabilities which support a fully “plug-n-play” architecture – essential to ensure a good end-user experience.
Many developers have contributed interfaces for many Home Automation devices and systems.

###About xPL-ESP8266
This small application for the ESP8266 is written in C and runs as firmware uploaded to the ESP8266, replacing the default AT firmware. 
Using the ESP-01 variant, the application uses GPIO0 as an output, controlled by xPL messages, to turn an LED on or off.
GPIO2 is used as an input, sending xPL trigger messages depending on closed/open status of GPIO2.

 

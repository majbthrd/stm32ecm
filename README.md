USB CDC-ECM example for STM32F072 microcontroller
=================================================

This is a second generation (and hopefully more useful) version of this project.  The first was originally written due to the apparent dearth of open-source implementations of this class of device.

However, unlike the previous, this new version has embedded virtual DHCP, DNS, and web servers.

Utilizing a copy of [lwIP 1.4.1](https://savannah.nongnu.org/projects/lwip/) from the [lrndis](https://github.com/fetisov/lrndis) project, this is a CDC-ECM implementation for the STM32F072.

This is related to [D21ecm](https://github.com/majbthrd/D21ecm/), another CDC-ECM implementation (but for the Atmel SAMD21).

Testing was done with both Linux and Mac OS X Lion 10.7.5 hosts.

## Alternatives

Note that my priority going forward is to add the same and additional capabilities to [TinyUSB](https://github.com/hathach/tinyusb); TinyUSB has the added benefit of supporting many different processors.

## Usage

TCP/IP is inherently a high memory user (at least in traditional embedded terms).  However, by tweaking the options of [lwIP](https://savannah.nongnu.org/projects/lwip/), I have the implementation fitting within the 16kBytes of RAM and 128kBytes of FLASH available to the STM32F072.

## Specifics

Look at main.c to get an idea of how the code could be modified.  As written, one quantity (systick) is shown in real-time as "Device Time" on the embedded web server (192.168.7.1) and another three quantities (alpha, bravo, and charlie) are "User Controls" on the web page that cause app.c code to be executed.

## Requirements for compiling

[Rowley Crossworks for ARM](http://www.rowley.co.uk/arm/) is presently needed to compile this code.

All the code is gcc/clang compatible, and as time permits, other options may be added.

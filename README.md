USB CDC-ECM for STM32F072 microcontroller
=========================================

This USB CDC-ECM (USB to Ethernet) implementation was written due to the apparent dearth of open-source implementations of this class of device.

A standalone implementation would be difficult to appreciate without some sort of IP network stack implemented on the microcontroller to communicate with.  So, this code leverages [FreeRTOS/TCP](http://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/).

As presently written, the microcontroller has the static IP address 192.168.200.1/24, and the host must manually configure its IP address to be on the same subset.  There are no services running on the microcontroller, but it can be pinged.

Testing was done with a Linux host.

## Requirements

[Rowley Crossworks for ARM](http://www.rowley.co.uk/arm/) is needed to compile this code.  The source code is gcc-friendly, but you must adapt the code yourself if you wish to adopt a different tool chain.

## Sanity Checklist If Customizing

USBD\_MAX\_NUM\_INTERFACES in usbd\_conf.h must conform to the NUM\_OF\_ECM value in usbd\_ecm.h (nominally 2x NUM\_OF\_ECM since each ECM has a Notify and Data interface).

The Notify and Data Interface numbers in the USB descriptor in usbd\_desc.c must be continguous and start from zero.

An understanding of USB descriptors is important when modifying usb_desc.c.  This data conveys the configuration of the device (including endpoint, etc.) to the host PC.

The parameters array values in usbd\_ecm.c must be consistent with the Notify and Data Interface numbers in the USB descriptor in usbd\_desc.c.

USB transfers are handled via a distinct section of memory called "PMA".  Read the ST documentation on this.  At most, there is 1kBytes that must be shared across all endpoints.  Consider the usage of this PMA memory when scaling up the number of UARTs and buffer sizes.


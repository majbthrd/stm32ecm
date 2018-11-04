/*
    USB descriptor structs

    Copyright (C) 2015,2016 Peter Lawrence

    Permission is hereby granted, free of charge, to any person obtaining a 
    copy of this software and associated documentation files (the "Software"), 
    to deal in the Software without restriction, including without limitation 
    the rights to use, copy, modify, merge, publish, distribute, sublicense, 
    and/or sell copies of the Software, and to permit persons to whom the 
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in 
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
    DEALINGS IN THE SOFTWARE.
*/

#ifndef __USB_MAGIC_H
#define __USB_MAGIC_H

#include <stdint.h>

/*
IMPORTANT note to self:
all of these helper structs must *solely* use uint8_t-derived members
GCC doesn't seem to support packed structs any more, so to only way to avoid padding is to be uint8_t-only
*/

struct usb_uint16
{
  uint8_t lo;
  uint8_t hi;
};

#define USB_UINT16(x) { .lo = LOBYTE(x), .hi = HIBYTE(x) }

struct device_descriptor
{
  uint8_t bLength;
  uint8_t bDescriptorType;
  struct usb_uint16 bcdUSB;
  uint8_t bDeviceClass;
  uint8_t bDeviceSubclass;
  uint8_t bDeviceProtocol;
  uint8_t bMaxPacketSize0;
  struct usb_uint16 idVendor;
  struct usb_uint16 idProduct;
  struct usb_uint16 bcdDevice;
  uint8_t  iManufacturer;
  uint8_t  iProduct;
  uint8_t  iSerialNumber;
  uint8_t  bNumConfigurations;
};

struct configuration_descriptor
{
  uint8_t bLength;
  uint8_t bDescriptorType;
  struct usb_uint16 wTotalLength;
  uint8_t bNumInterfaces;
  uint8_t bConfigurationValue;
  uint8_t iConfiguration;
  uint8_t bmAttributes;
  uint8_t bMaxPower;
};

struct interface_descriptor
{
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bInterfaceNumber;
  uint8_t bAlternateSetting;
  uint8_t bNumEndpoints;
  uint8_t bInterfaceClass;
  uint8_t bInterfaceSubclass;
  uint8_t bInterfaceProtocol;
  uint8_t iInterface;
};

struct endpoint_descriptor
{
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bEndpointAddress;
  uint8_t bmAttributes;
  struct usb_uint16 wMaxPacketSize;
  uint8_t bInterval;
};

struct cdc_functional_descriptor_header
{
  uint8_t bFunctionLength;
  uint8_t bDescriptorType;
  uint8_t bDescriptorSubtype;
  struct usb_uint16 bcdCDC;
};

struct cdc_acm_functional_descriptor
{
  uint8_t bFunctionLength;
  uint8_t bDescriptorType;
  uint8_t bDescriptorSubtype;
  uint8_t bmCapabilities;
};

struct cdc_cm_functional_descriptor
{
  uint8_t bFunctionLength;
  uint8_t bDescriptorType;
  uint8_t bDescriptorSubtype;
  uint8_t bmCapabilities;
  uint8_t bDataInterface;
};

struct cdc_union_functional_descriptor
{
  uint8_t bFunctionLength;
  uint8_t bDescriptorType;
  uint8_t bDescriptorSubtype;
  uint8_t bMasterInterface;
  uint8_t bSlaveInterface0;
};

struct cdc_enet_functional_descriptor
{
  uint8_t bFunctionLength;
  uint8_t bDescriptorType;
  uint8_t bDescriptorSubtype;
  uint8_t iMACAddress;
  uint8_t bmEthernetStatistics[4];
  struct usb_uint16 wMaxSegmentSize;
  struct usb_uint16 wMCFilters;
  uint8_t bNumberPowerFilters;
};

struct interface_association_descriptor
{
  uint8_t bLength;
  uint8_t bDescriptorType;
  uint8_t bFirstInterface;
  uint8_t bInterfaceCount;
  uint8_t bFunctionClass;
  uint8_t bFunctionSubClass;
  uint8_t bFunctionProtocol;
  uint8_t iFunction;
};

#endif /* __USB_MAGIC_H */

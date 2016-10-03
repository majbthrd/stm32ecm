/*
    USB descriptor macros for CDC-ECM

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

#ifndef __ECM_HELPER_H
#define __ECM_HELPER_H

#include <stdint.h>
#include "usbhelper.h"

struct ecm_interface
{
  struct interface_descriptor             ctl_interface;
  struct cdc_functional_descriptor_header cdc_ecm_header;
  struct cdc_union_functional_descriptor  cdc_ecm_union;
  struct cdc_enet_functional_descriptor   cdc_ecm_functional;
  struct endpoint_descriptor              ctl_ep;
  struct interface_descriptor             dat_interface;
  struct endpoint_descriptor              ep_in;
  struct endpoint_descriptor              ep_out;
};

/* macro to help generate CDC ECM USB descriptors */

#define ECM_DESCRIPTOR(NOTIFY_ITF, DATA_ITF, NOTIFY_EP, DATAOUT_EP, DATAIN_EP, IMAC_STRING) \
    { \
      { \
        /*Interface Descriptor */ \
        sizeof(struct interface_descriptor),             /* bLength: Interface Descriptor size */ \
        USB_DESC_TYPE_INTERFACE,                         /* bDescriptorType: Interface */ \
        NOTIFY_ITF,                                      /* bInterfaceNumber: Number of Interface */ \
        0x00,                                            /* bAlternateSetting: Alternate setting */ \
        0x01,                                            /* bNumEndpoints: One endpoints used */ \
        0x02,                                            /* bInterfaceClass: Communication Interface Class */ \
        0x06,                                            /* bInterfaceSubClass: Ethernet Control Model */ \
        0x00,                                            /* bInterfaceProtocol: No specific protocol */ \
        0x00,                                            /* iInterface */ \
      }, \
 \
      { \
        /*Header Functional Descriptor*/ \
        sizeof(struct cdc_functional_descriptor_header), /* bLength: Endpoint Descriptor size */ \
        0x24,                                            /* bDescriptorType: CS_INTERFACE */ \
        0x00,                                            /* bDescriptorSubtype: Header Func Desc */ \
        USB_UINT16(0x0120),                              /* bcdCDC: spec release number */ \
      }, \
 \
      { \
        /*Union Functional Descriptor*/ \
        sizeof(struct cdc_union_functional_descriptor),  /* bFunctionLength */ \
        0x24,                                            /* bDescriptorType: CS_INTERFACE */ \
        0x06,                                            /* bDescriptorSubtype: Union func desc */ \
        NOTIFY_ITF,                                       /* bMasterInterface: Notify interface */ \
        DATA_ITF,                                        /* bSlaveInterface0: Data Interface */ \
      }, \
 \
      { \
        /*Ethernet Networking Functional Descriptor*/ \
        sizeof(struct cdc_enet_functional_descriptor),   /* bFunctionLength */ \
        0x24,                                            /* bDescriptorType: CS_INTERFACE */ \
        0x0F,                                            /* bDescriptorSubtype: Ethernet Networking */ \
        IMAC_STRING,                                     /* iMACAddress: mandatory string providing MAC address */ \
        {0,0,0,0},                                       /* bmEthernetStatistics */ \
        USB_UINT16(ECM_MAX_SEGMENT_SIZE),                /* wMaxSegmentSize: in bytes, usually 1514 */ \
        USB_UINT16(0),                                   /* wMCFilters: no filters */ \
        0,                                               /* bNumberPowerFilters: no filters */ \
      }, \
 \
      { \
        /* Notify Endpoint Descriptor*/ \
        sizeof(struct endpoint_descriptor),              /* bLength: Endpoint Descriptor size */ \
        USB_DESC_TYPE_ENDPOINT,                          /* bDescriptorType: Endpoint */ \
        NOTIFY_EP,                                       /* bEndpointAddress */ \
        0x03,                                            /* bmAttributes: Interrupt */ \
        USB_UINT16(ECM_MAX_USB_NOTIFY_PACKET_SIZE),      /* wMaxPacketSize: */ \
        0xFF,                                            /* bInterval: */  \
      }, \
 \
      { \
        /*Data class interface descriptor*/ \
        sizeof(struct interface_descriptor),             /* bLength: Endpoint Descriptor size */ \
        USB_DESC_TYPE_INTERFACE,                         /* bDescriptorType: */ \
        DATA_ITF,                                        /* bInterfaceNumber: Number of Interface */ \
        0x00,                                            /* bAlternateSetting: Alternate setting */ \
        0x02,                                            /* bNumEndpoints: Two endpoints used */ \
        0x0A,                                            /* bInterfaceClass: CDC */ \
        0x00,                                            /* bInterfaceSubClass: */ \
        0x00,                                            /* bInterfaceProtocol: */ \
        0x00,                                            /* iInterface: */ \
      }, \
 \
      { \
        /* Data Endpoint OUT Descriptor */ \
        sizeof(struct endpoint_descriptor),              /* bLength: Endpoint Descriptor size */ \
        USB_DESC_TYPE_ENDPOINT,                          /* bDescriptorType: Endpoint */ \
        DATAOUT_EP,                                      /* bEndpointAddress */ \
        0x02,                                            /* bmAttributes: Bulk */ \
        USB_UINT16(ECM_MAX_USB_DATA_PACKET_SIZE),        /* wMaxPacketSize: */ \
        0x00,                                            /* bInterval: ignore for Bulk transfer */ \
      }, \
 \
      { \
        /* Data Endpoint IN Descriptor*/ \
        sizeof(struct endpoint_descriptor),              /* bLength: Endpoint Descriptor size */ \
        USB_DESC_TYPE_ENDPOINT,                          /* bDescriptorType: Endpoint */ \
        DATAIN_EP,                                       /* bEndpointAddress */ \
        0x02,                                            /* bmAttributes: Bulk */ \
        USB_UINT16(ECM_MAX_USB_DATA_PACKET_SIZE),        /* wMaxPacketSize: */ \
        0x00                                             /* bInterval: ignore for Bulk transfer */ \
      }, \
    },

#endif /* __ECM_HELPER_H */

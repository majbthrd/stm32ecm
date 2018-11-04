/*
    USB descriptor macros for CDC-ECM

    Copyright (C) 2015,2016,2018 Peter Lawrence

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
#include "usbd_conf.h"

#define ECM_MAX_SEGMENT_SIZE           1514

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
      .ctl_interface = { \
        /*Interface Descriptor */ \
        .bLength            = sizeof(struct interface_descriptor),             /* Interface Descriptor size */ \
        .bDescriptorType    = USB_DESC_TYPE_INTERFACE,                         /* Interface */ \
        .bInterfaceNumber   = NOTIFY_ITF,                                      /* Number of Interface */ \
        .bAlternateSetting  = 0x00,                                            /* Alternate setting */ \
        .bNumEndpoints      = 0x01,                                            /* One endpoints used */ \
        .bInterfaceClass    = 0x02,                                            /* Communication Interface Class */ \
        .bInterfaceSubclass = 0x06,                                            /* Ethernet Control Model */ \
        .bInterfaceProtocol = 0x00,                                            /* No specific protocol */ \
        .iInterface         = 0x00,                                            \
      }, \
 \
      .cdc_ecm_header = { \
        /*Header Functional Descriptor*/ \
        .bFunctionLength    = sizeof(struct cdc_functional_descriptor_header), /* bLength: Endpoint Descriptor size */ \
        .bDescriptorType    = 0x24,                                            /* bDescriptorType: CS_INTERFACE */ \
        .bDescriptorSubtype = 0x00,                                            /* bDescriptorSubtype: Header Func Desc */ \
        .bcdCDC             = USB_UINT16(0x0120),                              /* bcdCDC: spec release number */ \
      }, \
 \
      .cdc_ecm_union = { \
        /*Union Functional Descriptor*/ \
        .bFunctionLength    = sizeof(struct cdc_union_functional_descriptor),  \
        .bDescriptorType    = 0x24,                                            /* CS_INTERFACE */ \
        .bDescriptorSubtype = 0x06,                                            /* Union func desc */ \
        .bMasterInterface   = NOTIFY_ITF,                                      /* Notify interface */ \
        .bSlaveInterface0   = DATA_ITF,                                        /* Data Interface */ \
      }, \
 \
      .cdc_ecm_functional = { \
        /*Ethernet Networking Functional Descriptor*/ \
        .bFunctionLength    = sizeof(struct cdc_enet_functional_descriptor),   \
        .bDescriptorType    = 0x24,                                            /* CS_INTERFACE */ \
        .bDescriptorSubtype = 0x0F,                                            /* Ethernet Networking */ \
        .iMACAddress        = IMAC_STRING,                                     /* mandatory string providing MAC address */ \
        .bmEthernetStatistics = {0,0,0,0},                                     \
        .wMaxSegmentSize    = USB_UINT16(ECM_MAX_SEGMENT_SIZE),                /* in bytes, usually 1514 */ \
        .wMCFilters         = USB_UINT16(0),                                   /* no filters */ \
        .bNumberPowerFilters = 0,                                              /* no filters */ \
      }, \
 \
      .ctl_ep = { \
        /* Notify Endpoint Descriptor*/ \
        .bLength            = sizeof(struct endpoint_descriptor),              /* Endpoint Descriptor size */ \
        .bDescriptorType    = USB_DESC_TYPE_ENDPOINT,                          /* Endpoint */ \
        .bEndpointAddress   = NOTIFY_EP,                                       \
        .bmAttributes       = 0x03,                                            /* Interrupt */ \
        .wMaxPacketSize     = USB_UINT16(ECM_NOTIFICATION_IN_SZ),              \
        .bInterval          = 0xFF,                                            \
      }, \
 \
      .dat_interface = { \
        /*Data class interface descriptor*/ \
        .bLength            = sizeof(struct interface_descriptor),             /* Endpoint Descriptor size */ \
        .bDescriptorType    = USB_DESC_TYPE_INTERFACE,                         \
        .bInterfaceNumber   = DATA_ITF,                                        /* Number of Interface */ \
        .bAlternateSetting  = 0x00,                                            /* Alternate setting */ \
        .bNumEndpoints      = 0x02,                                            /* Two endpoints used */ \
        .bInterfaceClass    = 0x0A,                                            /* CDC */ \
        .bInterfaceSubclass = 0x00,                                            \
        .bInterfaceProtocol = 0x00,                                            \
        .iInterface         = 0x00,                                            \
      }, \
 \
      .ep_in = { \
        /* Data Endpoint IN Descriptor*/ \
        .bLength            = sizeof(struct endpoint_descriptor),              /* Endpoint Descriptor size */ \
        .bDescriptorType    = USB_DESC_TYPE_ENDPOINT,                          /* Endpoint */ \
        .bEndpointAddress   = DATAIN_EP,                                       \
        .bmAttributes       = 0x02,                                            /* Bulk */ \
        .wMaxPacketSize     = USB_UINT16(ECM_DATA_OUT_SZ),                     \
        .bInterval          = 0x00                                             /* ignore for Bulk transfer */ \
      }, \
 \
      .ep_out = { \
        /* Data Endpoint OUT Descriptor */ \
        .bLength            = sizeof(struct endpoint_descriptor),              /* Endpoint Descriptor size */ \
        .bDescriptorType    = USB_DESC_TYPE_ENDPOINT,                          /* Endpoint */ \
        .bEndpointAddress   = DATAOUT_EP,                                      \
        .bmAttributes       = 0x02,                                            /* Bulk */ \
        .wMaxPacketSize     = USB_UINT16(ECM_DATA_IN_SZ),                      \
        .bInterval          = 0x00,                                            /* ignore for Bulk transfer */ \
      }, \
    },

#endif /* __ECM_HELPER_H */

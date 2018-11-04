/*
    USB CDC-ECM for STM32F072 microcontroller

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

#ifndef __USB_ECM_H_
#define __USB_ECM_H_

#include <stdint.h>
#include <stdbool.h>
#include "usbd_def.h"
#include "usbd_ioreq.h"
#include "netif/etharp.h"
#include "ecmhelper.h"

#define ETH_HEADER_SIZE             14
#define ECM_MTU                     (ECM_MAX_SEGMENT_SIZE - ETH_HEADER_SIZE)

/* array of callback functions invoked by USBD_RegisterClass() in main.c */
extern const USBD_ClassTypeDef USBD_ECM;

uint8_t USBD_ECM_RegisterInterface(USBD_HandleTypeDef *pdev);
void USBD_ECM_PMAConfig(PCD_HandleTypeDef *hpcd, uint32_t *pma_address);

void usb_ecm_recv_callback(const uint8_t *data, int size);
void usb_ecm_recv_renew(void);

bool usb_ecm_can_xmit(void);
void usb_ecm_xmit_packet(struct pbuf *p);
  
#endif  // __USB_ECM_H_

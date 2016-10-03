/*
    USB CDC-ECM for STM32F072 microcontroller

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

#ifndef __USB_ECM_H_
#define __USB_ECM_H_

#include "usbd_def.h"
#include "usbd_ioreq.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#define NUM_OF_ECM 1

#define ECM_MAX_USB_DATA_PACKET_SIZE   USB_FS_MAX_PACKET_SIZE
#define ECM_MAX_USB_NOTIFY_PACKET_SIZE USB_FS_MAX_PACKET_SIZE
#define ECM_MAX_SEGMENT_SIZE           1514

/* struct type used for each instance of a ECM */
typedef struct
{
  /*
  ST's example code partially used 32-bit alignment of individual buffers with no explanation as to why;
  word alignment is relevant for DMA, so this practice was used (albeit in a more consistent manner) in this struct
  */
  uint32_t                   OutboundBuffer[(ECM_MAX_SEGMENT_SIZE + ECM_MAX_USB_DATA_PACKET_SIZE - 1)/sizeof(uint32_t)];
  volatile uint32_t          OutboundTransferNeedsRenewal;
  volatile uint32_t          OutboundTransferPosition;
  volatile uint32_t          OutboundTransferInhibited;
  xSemaphoreHandle           xmitSemaphore;
  volatile uint32_t          xmitReady;
} USBD_ECM_HandleTypeDef;

/* array of callback functions invoked by USBD_RegisterClass() in main.c */
extern const USBD_ClassTypeDef USBD_ECM;

uint8_t USBD_ECM_RegisterInterface(USBD_HandleTypeDef *pdev);
void USBD_ECM_PMAConfig(PCD_HandleTypeDef *hpcd, uint32_t *pma_address);
  
#endif  // __USB_ECM_H_

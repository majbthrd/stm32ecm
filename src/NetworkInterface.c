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

/*
Receive theory of operation:

The function ECM_ReceivePacket() is called by the CDC-ECM driver.
When called with a NULL pointer, it is interpreted as network status.

When called with a pointer to a packet, it notes the specifics, gives a semaphore, and returns.
The immediate return is to ensure that the device driver is not blocked.
The thread prvEMACHandlerTask() takes this semaphore and provides the packet to the network stack.
It then calls ECM_RenewReceivePacket() to allow the CDC-ECM to receive the next packet.

Transmit theory of operation:

Whenever the network stack provides another packet to transmit, ECM_TransmitPacket() is called.
*/

#include "usbd_desc.h"
#include "usbd_ecm.h" 
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_IP_Private.h"
#include "NetworkBufferManagement.h"
#include "NetworkInterface.h"

/* in principle, the ECM implementation can support multiple virtual network interfaces; only one is serviced by FreeRTOS/TCP */
#define ECM_INTERFACE_INDEX_USED_BY_FREERTOS_TCP 0

#ifndef configEMAC_TASK_STACK_SIZE
	#define configEMAC_TASK_STACK_SIZE ( 1 * configMINIMAL_STACK_SIZE )
#endif

static void prvEMACHandlerTask( void *pvParameters );

static TaskHandle_t xEMACTaskHandle = NULL;

static xSemaphoreHandle rcvSemaphore;

static uint8_t *rcv_data;
static uint16_t rcv_length;

static BaseType_t network_active = 0;

extern PCD_HandleTypeDef hpcd;

portBASE_TYPE USBISRhaswokenxHigherPriorityTask;

void USB_IRQHandler(void)
{
  USBISRhaswokenxHigherPriorityTask = pdFALSE; 

  HAL_PCD_IRQHandler(&hpcd);

  portYIELD_FROM_ISR(USBISRhaswokenxHigherPriorityTask);
}

/* API into the ECM driver */
extern void ECM_TransmitPacket(unsigned index, uint8_t *data, uint16_t length);
extern void ECM_RenewReceivePacket(unsigned index);

static void prvEMACHandlerTask(void *pvParameters)
{
  xNetworkBufferDescriptor_t *pxDescriptor;
  const TickType_t xDescriptorWaitTime = pdMS_TO_TICKS(250);
  xIPStackEvent_t xRxEvent = { eNetworkRxEvent, NULL };

  vSemaphoreCreateBinary(rcvSemaphore);

  for (;;)
  {
    xSemaphoreTake(rcvSemaphore, portMAX_DELAY);

    /* Create a buffer of the required length. */
    pxDescriptor = pxGetNetworkBufferWithDescriptor( rcv_length, xDescriptorWaitTime );

    if (pxDescriptor)
    {
      /* cap the length to the destination buffer's length */
      if( rcv_length > pxDescriptor->xDataLength )
      {
        rcv_length = pxDescriptor->xDataLength;
      }

      /* copy the data */
      memcpy(pxDescriptor->pucEthernetBuffer, rcv_data, rcv_length);

      xRxEvent.pvData = (void *)pxDescriptor;

      if( xSendEventStructToIPTask( &xRxEvent, xDescriptorWaitTime ) == pdFALSE )
      {
        /* Could not send the descriptor into the TCP/IP stack; it must be released */
        vReleaseNetworkBufferAndDescriptor( pxDescriptor );
      }
    }

    /* inform the ECM driver that it is free to start receiving again */
    ECM_RenewReceivePacket(ECM_INTERFACE_INDEX_USED_BY_FREERTOS_TCP);  
  }
}

void ECM_ReceivePacket(unsigned index, uint8_t *data, uint16_t length)
{
  if (index != ECM_INTERFACE_INDEX_USED_BY_FREERTOS_TCP)
    return;

  if (!data)
  {
    /* when NULL pointer, this serves to indicate whether the network is active */
    network_active = length;
    return;
  }

  /* make a copy of the buffer specifics so that prvEMACHandlerTask can later use it */
  rcv_data = data;
  rcv_length = length;

  /* signal serviceTask() that another packet has been received */
  xSemaphoreGive(rcvSemaphore);
}

BaseType_t xNetworkInterfaceInitialise( void )
{
  BaseType_t outcome = network_active;
  BaseType_t xReturn;

  if( xEMACTaskHandle == NULL )
  {
    /* create task responsible for passing Ethernet frames from USB ECM to FreeRTOS/TCP */
    xReturn = xTaskCreate(prvEMACHandlerTask, "EMAC", configEMAC_TASK_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, &xEMACTaskHandle);

    if (pdFALSE == xReturn)
      outcome = 0;
  }

  return outcome;
}

BaseType_t xNetworkInterfaceOutput( xNetworkBufferDescriptor_t * const pxDescriptor, BaseType_t bReleaseAfterSend )
{
  /* hand a pointer of the buffer to the ECM driver */
  ECM_TransmitPacket(ECM_INTERFACE_INDEX_USED_BY_FREERTOS_TCP, pxDescriptor->pucEthernetBuffer, pxDescriptor->xDataLength);

  if( bReleaseAfterSend != pdFALSE )
  {
    vReleaseNetworkBufferAndDescriptor( pxDescriptor );
  }

  return pdPASS;
}

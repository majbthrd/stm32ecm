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
Theory of operation:

Each incoming Ethernet packet from the host is passed to the externally-provided function ECM_ReceivePacket().
At this point, the USB driver does not accept any more data.
When the network stack has finished handling the packet, it calls ECM_RenewReceivePacket(), which restores reception.

The network stack sends an Ethernet packet to the host using ECM_TransmitPacket().
This function blocks until the packet has been completely transferred to the host.
Once ECM_TransmitPacket() returns, it can be called again.
*/

#include "usbd_ecm.h"
#include "usbd_desc.h"

/* USB handle declared in main.c */
extern USBD_HandleTypeDef USBD_Device;

extern void ECM_ReceivePacket(unsigned index, uint8_t *data, uint16_t length);

/* local function prototyping */

static uint8_t USBD_ECM_Init (USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_ECM_DeInit (USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_ECM_Setup (USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_ECM_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_ECM_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_ECM_EP0_RxReady (USBD_HandleTypeDef *pdev);
static const uint8_t *USBD_ECM_GetFSCfgDesc (uint16_t *length);
static uint8_t USBD_ECM_SOF (USBD_HandleTypeDef *pdev);

static USBD_StatusTypeDef USBD_ECM_ReceivePacket (USBD_HandleTypeDef *pdev, unsigned index);

/* CDC interface class callbacks structure that is used by main.c */
const USBD_ClassTypeDef USBD_ECM = 
{
  .Init                  = USBD_ECM_Init,
  .DeInit                = USBD_ECM_DeInit,
  .Setup                 = USBD_ECM_Setup,
  .EP0_TxSent            = NULL,
  .EP0_RxReady           = USBD_ECM_EP0_RxReady,
  .DataIn                = USBD_ECM_DataIn,
  .DataOut               = USBD_ECM_DataOut,
  .SOF                   = USBD_ECM_SOF,
  .IsoINIncomplete       = NULL,
  .IsoOUTIncomplete      = NULL,     
  .GetFSConfigDescriptor = USBD_ECM_GetFSCfgDesc,    
};

/* endpoint numbers and "instance" (base register address) for each UART */
static const struct
{
  uint8_t data_in_ep, data_out_ep, notify_in_ep, notify_itf;
} parameters[NUM_OF_ECM] = 
{
#if (NUM_OF_ECM > 0)
  {
    .data_in_ep   = 0x82,
    .data_out_ep  = 0x01,
    .notify_in_ep = 0x81,
    .notify_itf   = 0x00,
  },
#endif
#if (NUM_OF_ECM > 1)
  {
    .data_in_ep   = 0x84,
    .data_out_ep  = 0x03,
    .notify_in_ep = 0x83,
    .notify_itf   = 0x01,
  },
#endif
};

/* context for each and every UART managed by this CDC implementation */
static USBD_ECM_HandleTypeDef context[NUM_OF_ECM];

static USBD_HandleTypeDef *registered_pdev;

static uint8_t USBD_ECM_Init (USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  USBD_ECM_HandleTypeDef *hecm = context;
  unsigned index;

  registered_pdev = pdev;

  for (index = 0; index < NUM_OF_ECM; index++,hecm++)
  {
    /* Open EP IN */
    USBD_LL_OpenEP(pdev, parameters[index].data_in_ep, USBD_EP_TYPE_BULK, ECM_MAX_USB_DATA_PACKET_SIZE);
    
    /* Open EP OUT */
    USBD_LL_OpenEP(pdev, parameters[index].data_out_ep, USBD_EP_TYPE_BULK, ECM_MAX_USB_DATA_PACKET_SIZE);

    /* Open Command IN EP */
    USBD_LL_OpenEP(pdev, parameters[index].notify_in_ep, USBD_EP_TYPE_INTR, ECM_MAX_USB_NOTIFY_PACKET_SIZE);
  
    hecm->OutboundTransferNeedsRenewal = 0;
    hecm->OutboundTransferPosition = 0;
    hecm->OutboundTransferInhibited = 0;
     
    /* Prepare Out endpoint to receive next packet */
    USBD_ECM_ReceivePacket(pdev, index);

    xSemaphoreGive(context[index].xmitSemaphore);

    /* signal Init() has happened */
    ECM_ReceivePacket(index, NULL, 1);
  }

  return USBD_OK;
}

static uint8_t USBD_ECM_DeInit (USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  USBD_ECM_HandleTypeDef *hecm = context;
  unsigned index;

  registered_pdev = NULL;

  for (index = 0; index < NUM_OF_ECM; index++,hecm++)
  {
    /* Close EP IN */
    USBD_LL_CloseEP(pdev, parameters[index].data_in_ep);
  
    /* Close EP OUT */
    USBD_LL_CloseEP(pdev, parameters[index].data_out_ep);
  
    /* Close Command IN EP */
    USBD_LL_CloseEP(pdev, parameters[index].notify_in_ep);

    /* signal DeInit() has happened */
    ECM_ReceivePacket(index, NULL, 0);
  }
  
  return USBD_OK;
}

static uint8_t USBD_ECM_Setup (USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  return USBD_OK;
}

extern portBASE_TYPE USBISRhaswokenxHigherPriorityTask;

static uint8_t USBD_ECM_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  USBD_ECM_HandleTypeDef *hecm = context;
  unsigned index;

  for (index = 0; index < NUM_OF_ECM; index++,hecm++)
  {
    if (parameters[index].data_in_ep == (epnum | 0x80))
    {
      xSemaphoreGiveFromISR(context[index].xmitSemaphore, &USBISRhaswokenxHigherPriorityTask);
      break;
    }
  }

  return USBD_OK;
}

static uint8_t USBD_ECM_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum)
{      
  USBD_ECM_HandleTypeDef *hecm = context;
  uint32_t RxLength;
  unsigned index;

  for (index = 0; index < NUM_OF_ECM; index++,hecm++)
  {
    if (parameters[index].data_out_ep == epnum)
    {
      /* Get the received data length */
      RxLength = USBD_LL_GetRxDataSize (pdev, epnum);

      if (!context[index].OutboundTransferInhibited)
        context[index].OutboundTransferPosition += RxLength;

      if (RxLength < ECM_MAX_USB_DATA_PACKET_SIZE)
      {
        if (!context[index].OutboundTransferInhibited)
        {
          /* do something with the data */
          ECM_ReceivePacket(index, (uint8_t *)context[index].OutboundBuffer, context[index].OutboundTransferPosition);
        }

        context[index].OutboundTransferPosition = 0;
        context[index].OutboundTransferInhibited = 0;
        
        /*
        since we've called ECM_ReceivePacket(), we bail the loop now
        normally, execution would continue below to cause the USBD_ECM_ReceivePacket(), but we 
        defer this in this case until the higher-level networking code has finished with the buffer
        */
        break;
      }
      else if (context[index].OutboundTransferPosition > ECM_MAX_SEGMENT_SIZE)
      {
        context[index].OutboundTransferPosition = 0;
        context[index].OutboundTransferInhibited = 1;
      }

      /* Initiate next USB packet transfer */
      USBD_ECM_ReceivePacket(pdev, index);

      break;
    }
  }

  return USBD_OK;
}

static uint8_t USBD_ECM_SOF (USBD_HandleTypeDef *pdev)
{
  USBD_ECM_HandleTypeDef *hecm = context;
  unsigned index;

  for (index = 0; index < NUM_OF_ECM; index++,hecm++)
  {
    if (hecm->OutboundTransferNeedsRenewal) /* if there is a lingering request needed due to a HAL_BUSY, retry it */
      USBD_ECM_ReceivePacket(pdev, index);
  }

  return USBD_OK;
}

static uint8_t USBD_ECM_EP0_RxReady (USBD_HandleTypeDef *pdev)
{ 
  return USBD_OK;
}

static const uint8_t *USBD_ECM_GetFSCfgDesc (uint16_t *length)
{
  *length = USBD_CfgFSDesc_len;
  return USBD_CfgFSDesc_pnt;
}

uint8_t USBD_ECM_RegisterInterface(USBD_HandleTypeDef *pdev)
{
  unsigned index;

  /* semaphores are created here, as this function is only called once (by main) */
  for (index = 0; index < NUM_OF_ECM; index++)
  {
    vSemaphoreCreateBinary(context[index].xmitSemaphore);
  }

  return USBD_OK;
}

static uint8_t USBD_ECM_ReceivePacket(USBD_HandleTypeDef *pdev, unsigned index)
{
  USBD_StatusTypeDef outcome;

  outcome = USBD_LL_PrepareReceive(pdev, parameters[index].data_out_ep, (uint8_t *)context[index].OutboundBuffer + context[index].OutboundTransferPosition, ECM_MAX_USB_DATA_PACKET_SIZE);

  context[index].OutboundTransferNeedsRenewal = (USBD_OK != outcome); /* set if the HAL was busy so that we know to retry it */

  return outcome;
}

void USBD_ECM_PMAConfig(PCD_HandleTypeDef *hpcd, uint32_t *pma_address)
{
  unsigned index;

  /* allocate PMA memory for all endpoints associated with ECM */
  for (index = 0; index < NUM_OF_ECM; index++)
  {
    HAL_PCDEx_PMAConfig(hpcd, parameters[index].data_in_ep,  PCD_SNG_BUF, *pma_address =+ ECM_MAX_USB_DATA_PACKET_SIZE);
    HAL_PCDEx_PMAConfig(hpcd, parameters[index].data_out_ep, PCD_SNG_BUF, *pma_address =+ ECM_MAX_USB_DATA_PACKET_SIZE);
    HAL_PCDEx_PMAConfig(hpcd, parameters[index].notify_in_ep,  PCD_SNG_BUF, *pma_address =+ ECM_MAX_USB_NOTIFY_PACKET_SIZE);
  }
}

void ECM_TransmitPacket(unsigned index, uint8_t *data, uint16_t length)
{
  USBD_StatusTypeDef outcome;
  const TickType_t timeout = pdMS_TO_TICKS(250);

  if (index >= NUM_OF_ECM)
    return;

  if (!registered_pdev)
    return;

  taskENTER_CRITICAL();
  {
    /* Transmit next packet */
    outcome = USBD_LL_Transmit(registered_pdev, parameters[index].data_in_ep, data, length);
  }
  taskEXIT_CRITICAL();

  /* block until the ISR has finished transmitting packet or the timeout has elapsed */
  xSemaphoreTake(context[index].xmitSemaphore, timeout);
}

void ECM_RenewReceivePacket(unsigned index)
{
  if (index >= NUM_OF_ECM)
    return;

  if (!registered_pdev)
    return;

  /* Initiate next USB packet transfer */
  taskENTER_CRITICAL();
  {
    /*
    Ideally, we would invoke USBD_LL_PrepareReceive() here, but data flow intermittently stops in the bowels of ST's stack.
    So, we set a flag that the ISR will act upon; this same flag already exists to workaround when ST's stack has the HAL locked.
    */
    context[index].OutboundTransferNeedsRenewal = 1;
  }
  taskEXIT_CRITICAL();
}

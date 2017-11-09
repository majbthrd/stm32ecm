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

Leveraging the structs in usbhelper.h and ecmhelper.h, a USB descriptor for this device is created.
*/

#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_conf.h"
#include "usbhelper.h"
#include "usbd_ecm.h"
#include "ecmhelper.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define USBD_VID                      0x1209 /* temporary assignment */
#define USBD_PID                      0x0001 /* temporary assignment */
#define USBD_LANGID_STRING            0x409
#define USBD_MANUFACTURER_STRING      "Acme"
#define USBD_PRODUCT_FS_STRING        "ECM"

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static uint8_t *USBD_VCP_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *USBD_VCP_LangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *USBD_VCP_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *USBD_VCP_ProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *USBD_VCP_SerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *USBD_VCP_ConfigStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *USBD_VCP_InterfaceStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static void IntToUnicode (uint32_t value, uint8_t *pbuf, uint8_t len);

/* Private variables ---------------------------------------------------------*/
const USBD_DescriptorsTypeDef VCP_Desc =
{
  USBD_VCP_DeviceDescriptor,
  USBD_VCP_LangIDStrDescriptor, 
  USBD_VCP_ManufacturerStrDescriptor,
  USBD_VCP_ProductStrDescriptor,
  USBD_VCP_SerialStrDescriptor,
};

/* USB Standard Device Descriptor */
static const struct device_descriptor hUSBDDeviceDesc =
{
  sizeof(hUSBDDeviceDesc),    /* bLength */
  USB_DESC_TYPE_DEVICE,       /* bDescriptorType */
  USB_UINT16(0x0200),         /* bcdUSB */
  0x00,                       /* bDeviceClass */
  0x00,                       /* bDeviceSubClass */
  0x00,                       /* bDeviceProtocol */
  USB_MAX_EP0_SIZE,           /* bMaxPacketSize */
  USB_UINT16(USBD_VID),       /* idVendor */
  USB_UINT16(USBD_PID),       /* idProduct */
  USB_UINT16(0x0200),         /* bcdDevice */
  USBD_IDX_MFC_STR,           /* Index of manufacturer string */
  USBD_IDX_PRODUCT_STR,       /* Index of product string */
  USBD_IDX_SERIAL_STR,        /* Index of serial number string */
  USBD_MAX_NUM_CONFIGURATION  /* bNumConfigurations */
};

/* bespoke struct for this device; struct members are added and removed as needed */
struct configuration_1
{
  struct configuration_descriptor config;
  struct ecm_interface ecm[NUM_OF_ECM];
};

/* fully initialize the bespoke struct as a const */
__ALIGN_BEGIN static const struct configuration_1 USBD_ECM_CfgFSDesc __ALIGN_END =
{
  {
    /*Configuration Descriptor*/
    sizeof(struct configuration_descriptor),         /* bLength */
    USB_DESC_TYPE_CONFIGURATION,                     /* bDescriptorType */
    USB_UINT16(sizeof(USBD_ECM_CfgFSDesc)),          /* wTotalLength */
    USBD_MAX_NUM_INTERFACES,                         /* bNumInterfaces */
    0x01,                                            /* bConfigurationValue */
    0x00,                                            /* iConfiguration */
    0x80,                                            /* bmAttributes */
    50,                                              /* MaxPower */
  },

  {
#if (NUM_OF_ECM > 0)
    ECM_DESCRIPTOR(/* Notify ITF */ 0x00, /* Data ITF */ 0x01, /* Notify EP */ 0x81, /* DataOut EP */ 0x01, /* DataIn EP */ 0x82, /* iMACstring */ USBD_IDX_SERIAL_STR)
#endif
#if (NUM_OF_ECM > 1)
    ECM_DESCRIPTOR(/* Notify ITF */ 0x02, /* Data ITF */ 0x03, /* Notify EP */ 0x83, /* DataOut EP */ 0x03, /* DataIn EP */ 0x84, /* iMACstring */ USBD_IDX_SERIAL_STR)
#endif
  },
};

const uint8_t *const USBD_CfgFSDesc_pnt = (const uint8_t *)&USBD_ECM_CfgFSDesc;
const uint16_t USBD_CfgFSDesc_len = sizeof(USBD_ECM_CfgFSDesc);

/* USB Standard Device Descriptor */
static const uint8_t USBD_LangIDDesc[USB_LEN_LANGID_STR_DESC]= 
{
  USB_LEN_LANGID_STR_DESC,         
  USB_DESC_TYPE_STRING,       
  LOBYTE(USBD_LANGID_STRING),
  HIBYTE(USBD_LANGID_STRING), 
};

static uint8_t USBD_StrDesc[USBD_MAX_STR_DESC_SIZ];

/**  * @brief  Returns the device descriptor. 
  * @param  speed: Current device speed
  * @param  length: Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
static uint8_t *USBD_VCP_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  *length = sizeof(hUSBDDeviceDesc);
  return (uint8_t*)&hUSBDDeviceDesc;
}

/**
  * @brief  Returns the LangID string descriptor.        
  * @param  speed: Current device speed
  * @param  length: Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
static uint8_t *USBD_VCP_LangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  *length = sizeof(USBD_LangIDDesc);  
  return (uint8_t*)USBD_LangIDDesc;
}

/**
  * @brief  Returns the product string descriptor. 
  * @param  speed: Current device speed
  * @param  length: Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
static uint8_t *USBD_VCP_ProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  USBD_GetString((uint8_t *)USBD_PRODUCT_FS_STRING, USBD_StrDesc, length);    
  return USBD_StrDesc;
}

/**
  * @brief  Returns the manufacturer string descriptor. 
  * @param  speed: Current device speed
  * @param  length: Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
static uint8_t *USBD_VCP_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  USBD_GetString((uint8_t *)USBD_MANUFACTURER_STRING, USBD_StrDesc, length);
  return USBD_StrDesc;
}

/**
  * @brief  Returns the serial number string descriptor.        
  * @param  speed: Current device speed
  * @param  length: Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
static uint8_t *USBD_VCP_SerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  uint32_t deviceserial;
  
  /*
  for some peculiar reason, ST doesn't define the unique ID registers in the HAL include files
  the DEVICE_ID registers are documented in Chapter 33 of the RM0091 Reference Manual
  */
  deviceserial = *(uint32_t*)(0x1FFFF7B0); /*DEVICE_ID2*/
  
  USBD_StrDesc[0] = *length = 2 + 8*2 + 4*2;
  USBD_StrDesc[1] = USB_DESC_TYPE_STRING;
  /* set upper bits to ensure classification as locally administered */
  IntToUnicode (0x02020000, &USBD_StrDesc[2], 4);
  /* set lower 32-bits using silicon serial number */
  IntToUnicode (deviceserial, &USBD_StrDesc[10], 8);
  return USBD_StrDesc;  
}

/**
  * @brief  Convert Hex 32Bits value into char 
  * @param  value: value to convert
  * @param  pbuf: pointer to the buffer 
  * @param  len: buffer length
  * @retval None
  */
static void IntToUnicode (uint32_t value, uint8_t *pbuf, uint8_t len)
{
  uint8_t idx = 0;
  
  for( idx = 0 ; idx < len ; idx ++)
  {
    if( ((value >> 28)) < 0xA )
    {
      pbuf[ 2* idx] = (value >> 28) + '0';
    }
    else
    {
      pbuf[2* idx] = (value >> 28) + 'A' - 10; 
    }
    
    value = value << 4;
    
    pbuf[ 2* idx + 1] = 0;
  }
}

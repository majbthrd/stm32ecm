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

A virtual CDC-ECM Ethernet adapter is created through a CDC-ECM implementation and FreeRTOS/TCP.

In the present implementation, the microcontroller is hard-coded with IP address 192.168.200.1/24.
The host must manually configure its IP address to be on the same subnet.
*/

#include "usbd_desc.h"
#include "usbd_ecm.h" 
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOS_IP.h"

/* main USB handle used throughout the USB driver code */
USBD_HandleTypeDef USBD_Device;

/* helper function in stm32f0xx_hal_helper.c */
void SystemClock_Config(void);

/* variable that is initialized with the device MAC address */
static uint8_t ucMACAddress[6];

/* helper function to initialize ucMACAddress[] */
static void build_mac_address(void);

/* hard-coded network parameters */
static const uint8_t ucIPAddress[ 4 ] = { 192, 168, 200, 1 };
static const uint8_t ucNetMask[ 4 ] = { 255, 255, 255, 0 };
static const uint8_t ucGatewayAddress[ 4 ] = { 0, 0, 0, 0 };
static const uint8_t ucDNSServerAddress[ 4 ] = { 0, 0, 0, 0 };

int main(void)
{
  /* initialize ST's HAL */
  HAL_Init();
  
  /* Configure the system clock to get correspondent USB clock source */
  SystemClock_Config();
  
  /* craft locally administered MAC address from 32-bits of silicon serial number */
  memcpy(ucMACAddress + 2, (void *)0x1FFFF7B4 /*DEVICE_ID3*/, 4);

  /* Init Device Library */
  USBD_Init(&USBD_Device, &VCP_Desc, 0);
  
  /* Add Supported Class */
  USBD_RegisterClass(&USBD_Device, &USBD_ECM);
  
  /* Add ECM Interface Class */
  USBD_ECM_RegisterInterface(&USBD_Device);
  
  /* Start Device Process */
  USBD_Start(&USBD_Device);
  
  build_mac_address();

  FreeRTOS_IPInit( ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress );

  vTaskStartScheduler();

  /* should never reach here */
  for (;;);
}

void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
  ( void ) pcTaskName;
  ( void ) pxTask;
  taskDISABLE_INTERRUPTS();
  for(;;);
}

static void build_mac_address(void)
{
  /*
  for some peculiar reason, ST doesn't define the unique ID registers in the HAL include files
  the DEVICE_ID registers are documented in Chapter 33 of the RM0091 Reference Manual
  */
  uint8_t *device_id = (void *)0x1FFFF7B0 /*DEVICE_ID2*/;

  /* craft locally administered MAC address from 32-bits of silicon serial number */
  ucMACAddress[5] = *device_id++;
  ucMACAddress[4] = *device_id++;
  ucMACAddress[3] = *device_id++;
  ucMACAddress[2] = *device_id++;
  ucMACAddress[1] = 0x01; /* different value to ensure a MAC distinct from the one assigned to the host */
  ucMACAddress[0] = 0x02; /* set/clear bits to ensure classification as locally administered */
}

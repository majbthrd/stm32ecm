/*
modified:
replaced handler naming conventions
added #include
added support for CMSIS SystemInit
changed from SAMC21 to STM32F0xx
*/
/*
 * Copyright (c) 2016, Alex Taradov <alex@taradov.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stm32f0xx.h>

//-----------------------------------------------------------------------------
#define DUMMY __attribute__ ((weak, alias ("irq_handler_dummy")))

//-----------------------------------------------------------------------------
void Reset_Handler(void);
DUMMY void NMI_Handler(void);
DUMMY void HardFault_Handler(void);
DUMMY void SVC_Handler(void);
DUMMY void PendSV_Handler(void);
DUMMY void SysTick_Handler(void);

DUMMY void WWDG_IRQHandler(void);
DUMMY void PVD_VDDIO2_IRQHandler(void);
DUMMY void RTC_IRQHandler(void);
DUMMY void FLASH_IRQHandler(void);
DUMMY void RCC_CRS_IRQHandler(void);
DUMMY void EXTI0_1_IRQHandler(void);
DUMMY void EXTI2_3_IRQHandler(void);
DUMMY void EXTI4_15_IRQHandler(void);
DUMMY void TSC_IRQHandler(void);
DUMMY void DMA1_Channel1_IRQHandler(void);
DUMMY void DMA1_Channel2_3_IRQHandler(void);
DUMMY void DMA1_Channel4_5_6_7_IRQHandler(void);
DUMMY void ADC1_COMP_IRQHandler(void);
DUMMY void TIM1_BRK_UP_TRG_COM_IRQHandler(void);
DUMMY void TIM1_CC_IRQHandler(void);
DUMMY void TIM2_IRQHandler(void);
DUMMY void TIM3_IRQHandler(void);
DUMMY void TIM6_DAC_IRQHandler(void);
DUMMY void TIM7_IRQHandler(void);
DUMMY void TIM14_IRQHandler(void);
DUMMY void TIM15_IRQHandler(void);
DUMMY void TIM16_IRQHandler(void);
DUMMY void TIM17_IRQHandler(void);
DUMMY void I2C1_IRQHandler(void);
DUMMY void I2C2_IRQHandler(void);
DUMMY void SPI1_IRQHandler(void);
DUMMY void SPI2_IRQHandler(void);
DUMMY void USART1_IRQHandler(void);
DUMMY void USART2_IRQHandler(void);
DUMMY void USART3_4_IRQHandler(void);
DUMMY void CEC_CAN_IRQHandler(void);
DUMMY void USB_IRQHandler(void);

extern int main(void);

extern void _stack_top(void);
extern unsigned int _etext;
extern unsigned int _data;
extern unsigned int _edata;
extern unsigned int _bss;
extern unsigned int _ebss;

//-----------------------------------------------------------------------------
__attribute__ ((used, section(".vectors")))
void (* const vectors[])(void) =
{
  &_stack_top,                   // 0 - Initial Stack Pointer Value

  // Cortex-M0+ handlers
  Reset_Handler,                 // 1 - Reset
  NMI_Handler,                   // 2 - NMI
  HardFault_Handler,             // 3 - Hard Fault
  0,                             // 4 - Reserved
  0,                             // 5 - Reserved
  0,                             // 6 - Reserved
  0,                             // 7 - Reserved
  0,                             // 8 - Reserved
  0,                             // 9 - Reserved
  0,                             // 10 - Reserved
  SVC_Handler,                   // 11 - SVCall
  0,                             // 12 - Reserved
  0,                             // 13 - Reserved
  PendSV_Handler,                // 14 - PendSV
  SysTick_Handler,               // 15 - SysTick

  // Peripheral handlers
  WWDG_IRQHandler,
  PVD_VDDIO2_IRQHandler,
  RTC_IRQHandler,
  FLASH_IRQHandler,
  RCC_CRS_IRQHandler,
  EXTI0_1_IRQHandler,
  EXTI2_3_IRQHandler,
  EXTI4_15_IRQHandler,
  TSC_IRQHandler,
  DMA1_Channel1_IRQHandler,
  DMA1_Channel2_3_IRQHandler,
  DMA1_Channel4_5_6_7_IRQHandler,
  ADC1_COMP_IRQHandler,
  TIM1_BRK_UP_TRG_COM_IRQHandler,
  TIM1_CC_IRQHandler,
  TIM2_IRQHandler,
  TIM3_IRQHandler,
  TIM6_DAC_IRQHandler,
  TIM7_IRQHandler,
  TIM14_IRQHandler,
  TIM15_IRQHandler,
  TIM16_IRQHandler,
  TIM17_IRQHandler,
  I2C1_IRQHandler,
  I2C2_IRQHandler,
  SPI1_IRQHandler,
  SPI2_IRQHandler,
  USART1_IRQHandler,
  USART2_IRQHandler,
  USART3_4_IRQHandler,
  CEC_CAN_IRQHandler,
  USB_IRQHandler,
};

//-----------------------------------------------------------------------------
void Reset_Handler(void)
{
  unsigned int *src, *dst;

#ifndef DONT_USE_CMSIS_INIT
  SystemInit();
#endif

  src = &_etext;
  dst = &_data;
  while (dst < &_edata)
    *dst++ = *src++;

  dst = &_bss;
  while (dst < &_ebss)
    *dst++ = 0;

  main();
  while (1);
}

//-----------------------------------------------------------------------------
void irq_handler_dummy(void)
{
  while (1);
}

//-----------------------------------------------------------------------------
void _exit(int status)
{
  (void)status;
  while (1);
}

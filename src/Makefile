##############################################################################
BUILD = build
BIN = stm32cdcuart

##############################################################################
.PHONY: all directory clean size

CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

CFLAGS += -W -Wall --std=gnu99 -Os
CFLAGS += -fno-diagnostics-show-caret
CFLAGS += -fdata-sections -ffunction-sections
CFLAGS += -funsigned-char -funsigned-bitfields
CFLAGS += -mcpu=cortex-m0 -mthumb
CFLAGS += -MD -MP -MT $(BUILD)/$(*F).o -MF $(BUILD)/$(@F).d
CFLAGS += -g3
CFLAGS += -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable # suppress warnings that happen OFTEN with STM32 library code

LDFLAGS += -mcpu=cortex-m0 -mthumb
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,--script=./linker/stm32f072xB.ld

INCLUDES += \
  -I./include \
  -I./FreeRTOS/Source/include \
  -I./FreeRTOS/Source/portable/GCC/ARM_CM0 \
  -I./FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/include \
  -I./FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/portable/Compiler/GCC \
  -I.

SRCS += \
  ./FreeRTOS/Source/event_groups.c \
  ./FreeRTOS/Source/portable/MemMang/heap_2.c \
  ./FreeRTOS/Source/list.c \
  ./FreeRTOS/Source/portable/GCC/ARM_CM0/port.c \
  ./FreeRTOS/Source/queue.c \
  ./FreeRTOS/Source/tasks.c \
  ./FreeRTOS/Source/timers.c \
  ./FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/portable/BufferManagement/BufferAllocation_2.c \
  ./FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_ARP.c \
  ./FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_DHCP.c \
  ./FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_DNS.c \
  ./FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_IP.c \
  ./FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_Sockets.c \
  ./FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_Stream_Buffer.c \
  ./FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_TCP_IP.c \
  ./FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_TCP_WIN.c \
  ./FreeRTOS-Plus/Source/FreeRTOS-Plus-TCP/FreeRTOS_UDP_IP.c \
  ./main.c \
  ./NetworkInterface.c \
  ./stm32f0xx_hal.c \
  ./stm32f0xx_hal_cortex.c \
  ./stm32f0xx_hal_gpio.c \
  ./stm32f0xx_hal_helper.c \
  ./stm32f0xx_hal_pcd.c \
  ./stm32f0xx_hal_pcd_ex.c \
  ./stm32f0xx_hal_rcc.c \
  ./stm32f0xx_hal_rcc_ex.c \
  ./system_stm32f0xx.c \
  ./usbd_conf.c \
  ./usbd_core.c \
  ./usbd_ctlreq.c \
  ./usbd_desc.c \
  ./usbd_ecm.c \
  ./usbd_ioreq.c \
  ./startup_stm32f0xx.c

DEFINES += \
  -DSTM32F072xB

CFLAGS += $(INCLUDES) $(DEFINES)

OBJS = $(addprefix $(BUILD)/, $(notdir %/$(subst .c,.o, $(SRCS))))

all: directory $(BUILD)/$(BIN).elf $(BUILD)/$(BIN).hex $(BUILD)/$(BIN).bin size

$(BUILD)/$(BIN).elf: $(OBJS)
	@echo LD $@
	@$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o $@

$(BUILD)/$(BIN).hex: $(BUILD)/$(BIN).elf
	@echo OBJCOPY $@
	@$(OBJCOPY) -O ihex $^ $@

$(BUILD)/$(BIN).bin: $(BUILD)/$(BIN).elf
	@echo OBJCOPY $@
	@$(OBJCOPY) -O binary $^ $@

%.o:
	@echo CC $@
	@$(CC) $(CFLAGS) $(filter %/$(subst .o,.c,$(notdir $@)), $(SRCS)) -c -o $@

directory:
	@mkdir -p $(BUILD)

size: $(BUILD)/$(BIN).elf
	@echo size:
	@$(SIZE) -t $^

clean:
	@echo clean
	@-rm -rf $(BUILD)

-include $(wildcard $(BUILD)/*.d)

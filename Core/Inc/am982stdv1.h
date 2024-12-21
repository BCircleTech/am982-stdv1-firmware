#ifndef AM982STDV1_H
#define AM982STDV1_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "cmsis_os.h"
#include "dma.h"
#include "fdcan.h"
#include "i2c.h"
#include "memorymap.h"
#include "rtc.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"

extern UART_HandleTypeDef *rtkCOM1Ptr;
extern UART_HandleTypeDef *rtkCOM3Ptr;
extern UART_HandleTypeDef *boardUARTPtr;

void LedRunOn();
void LedRunOff();
void LedErrOn();
void LedErrOff();

void ResetRTK();
void SetRTKBaseWithPosition(double latitude, double longitude, double altitude);
void SetRTKBaseWithTime(unsigned int seconds);
void SetRTKRover(unsigned int freq);
void SetRTKConf(uint8_t *cmd, uint16_t size);

void ResetIMU();

#endif

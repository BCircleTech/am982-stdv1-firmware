#include "am982stdv1.h"
#include "usbd_cdc_if.h"

uint8_t rtkCOM1RxBuff[512];
uint8_t rtkCOM3RxBuff[512];

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
    if (huart->Instance == rtkCOM1Ptr->Instance)
    {
        HAL_UART_Transmit_DMA(boardUARTPtr, rtkCOM1RxBuff, size);
        HAL_UARTEx_ReceiveToIdle_DMA(rtkCOM1Ptr, rtkCOM1RxBuff, sizeof(rtkCOM1RxBuff));
    }
    else if (huart->Instance == rtkCOM3Ptr->Instance)
    {
        HAL_UART_Transmit_DMA(boardUARTPtr, rtkCOM3RxBuff, size);
        HAL_UARTEx_ReceiveToIdle_DMA(rtkCOM3Ptr, rtkCOM3RxBuff, sizeof(rtkCOM3RxBuff));
    }
}

void USB_CDC_RxHandler(uint8_t *data, uint32_t size)
{
}

void StartMain(void *argument)
{
    MX_USB_DEVICE_Init();

    HAL_UARTEx_ReceiveToIdle_DMA(rtkCOM1Ptr, rtkCOM1RxBuff, sizeof(rtkCOM1RxBuff));
    HAL_UARTEx_ReceiveToIdle_DMA(rtkCOM3Ptr, rtkCOM3RxBuff, sizeof(rtkCOM3RxBuff));

    ResetRTK();
    ResetIMU();
    osDelay(100);
    InitIMU(MPU6050_CLOCK_PLL_XGYRO, MPU6050_ACCEL_FS_2, MPU6050_GYRO_FS_250);
    SetIMUDigitalLowPassFilter(MPU6050_DLPF_BW_5);
    SetIMUSampleRate(100);

    while (1)
    {
        osDelay(100);
    }
}

void StartRTKCOM1(void *argument)
{
    while (1)
    {
        osDelay(100);
    }
}

void StartRTKCOM3(void *argument)
{
    while (1)
    {
        osDelay(100);
    }
}

void StartIMU(void *argument)
{
    while (1)
    {
        osDelay(100);
    }
}

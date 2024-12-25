#include "protocol.h"

extern "C"
{
#include "am982stdv1.h"
#include "message_buffer.h"
#include "usbd_cdc_if.h"

uint8_t rtkCOM1RxBuff[512];
uint8_t rtkCOM3RxBuff[512];

// MessageBufferHandle_t mainToUSB;
MessageBufferHandle_t usbToMain;
// MessageBufferHandle_t mainToRTKCOM1;
MessageBufferHandle_t rtkCOM1ToMain;
MessageBufferHandle_t rtkCOM3ToMain;
// MessageBufferHandle_t mainToIMU;
// MessageBufferHandle_t imuToMain;

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (huart->Instance == rtkCOM1Ptr->Instance)
    {
        xMessageBufferSendFromISR(rtkCOM1ToMain, rtkCOM1RxBuff, size, &xHigherPriorityTaskWoken);
        HAL_UARTEx_ReceiveToIdle_DMA(rtkCOM1Ptr, rtkCOM1RxBuff, sizeof(rtkCOM1RxBuff));
    }
    else if (huart->Instance == rtkCOM3Ptr->Instance)
    {
        xMessageBufferSendFromISR(rtkCOM3ToMain, rtkCOM3RxBuff, size, &xHigherPriorityTaskWoken);
        HAL_UARTEx_ReceiveToIdle_DMA(rtkCOM3Ptr, rtkCOM3RxBuff, sizeof(rtkCOM3RxBuff));
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void USB_CDC_RxHandler(uint8_t *data, uint32_t size)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xMessageBufferSendFromISR(usbToMain, data, size, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void StartMain(void *argument)
{
    MX_USB_DEVICE_Init();

    HAL_UARTEx_ReceiveToIdle_DMA(rtkCOM1Ptr, rtkCOM1RxBuff, sizeof(rtkCOM1RxBuff));
    HAL_UARTEx_ReceiveToIdle_DMA(rtkCOM3Ptr, rtkCOM3RxBuff, sizeof(rtkCOM3RxBuff));

    // mainToUSB = xMessageBufferCreate(1024);
    usbToMain = xMessageBufferCreate(1024);
    // mainToRTKCOM1 = xMessageBufferCreate(1024);
    rtkCOM1ToMain = xMessageBufferCreate(1024);
    rtkCOM3ToMain = xMessageBufferCreate(1024);
    // mainToIMU = xMessageBufferCreate(1024);
    // imuToMain = xMessageBufferCreate(1024);

    ResetRTK();
    ResetIMU();
    osDelay(100);
    InitIMU(MPU6050_CLOCK_PLL_XGYRO, MPU6050_ACCEL_FS_2, MPU6050_GYRO_FS_250);
    SetIMUDigitalLowPassFilter(MPU6050_DLPF_BW_5);
    SetIMUSampleRate(100);

    uint8_t usbRxBuffer[1024];
    size_t usbRxBufferLen;
    uint8_t rtkCOM1RxBuffer[1024];
    size_t rtkCOM1RxBufferLen;
    uint8_t rtkCOM3RxBuffer[1024];
    size_t rtkCOM3RxBufferLen;

    Segment segment;
    std::string line;

    while (1)
    {
        usbRxBufferLen = xMessageBufferReceive(usbToMain, usbRxBuffer, sizeof(usbRxBuffer), 10);
        if (usbRxBufferLen > 0)
        {
            // parse usbRxBuffer
            segment.Receive(usbRxBuffer, usbRxBufferLen);
        }
        rtkCOM1RxBufferLen = xMessageBufferReceive(rtkCOM1ToMain, rtkCOM1RxBuffer, sizeof(rtkCOM1RxBuffer), 10);
        if (rtkCOM1RxBufferLen > 0)
        {
            // parse rtkCOM1RxBuffer
            HAL_UART_Transmit_DMA(boardUARTPtr, rtkCOM1RxBuffer, rtkCOM1RxBufferLen);
        }
        rtkCOM3RxBufferLen = xMessageBufferReceive(rtkCOM3ToMain, rtkCOM3RxBuffer, sizeof(rtkCOM3RxBuffer), 10);
        if (rtkCOM3RxBufferLen > 0)
        {
            // parse rtkCOM1RxBuffer
            HAL_UART_Transmit_DMA(boardUARTPtr, rtkCOM3RxBuffer, rtkCOM3RxBufferLen);
        }

        if (segment.GetOne(line))
        {
        }

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
}
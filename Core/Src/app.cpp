#include "protocol.h"

extern "C"
{
#include "am982stdv1.h"
#include "message_buffer.h"
#include "usbd_cdc_if.h"

uint8_t initFlag = 0;

uint8_t rtkCOM1RxBuff[512];
uint8_t rtkCOM3RxBuff[512];

MessageBufferHandle_t usbToMain;
MessageBufferHandle_t mainToRTKCOM1;
MessageBufferHandle_t rtkCOM1ToMain;
MessageBufferHandle_t rtkCOM3ToMain;
MessageBufferHandle_t mainToIMU;

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (huart->Instance == rtkCOM1Ptr->Instance)
    {
        if (initFlag)
        {
            xMessageBufferSendFromISR(rtkCOM1ToMain, rtkCOM1RxBuff, size, &xHigherPriorityTaskWoken);
        }
        HAL_UARTEx_ReceiveToIdle_DMA(rtkCOM1Ptr, rtkCOM1RxBuff, sizeof(rtkCOM1RxBuff));
    }
    else if (huart->Instance == rtkCOM3Ptr->Instance)
    {
        if (initFlag)
        {
            xMessageBufferSendFromISR(rtkCOM3ToMain, rtkCOM3RxBuff, size, &xHigherPriorityTaskWoken);
        }
        HAL_UARTEx_ReceiveToIdle_DMA(rtkCOM3Ptr, rtkCOM3RxBuff, sizeof(rtkCOM3RxBuff));
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void USB_CDC_RxHandler(uint8_t *data, uint32_t size)
{
    if (initFlag)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        xMessageBufferSendFromISR(usbToMain, data, size, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void USB_Transmit(uint8_t cmd[2], uint8_t *data, uint32_t len)
{
    std::string res;
    Segment::Pack(res, cmd, std::string((char *)data, len));
    for (unsigned int i = 0; i < 5; i++)
    {
        if (CDC_Transmit_HS((uint8_t *)res.c_str(), res.length()) == USBD_OK)
        {
            break;
        }
        osDelay(1);
    }
}

void StartMain(void *argument)
{
    MX_USB_DEVICE_Init();

    HAL_UARTEx_ReceiveToIdle_DMA(rtkCOM1Ptr, rtkCOM1RxBuff, sizeof(rtkCOM1RxBuff));
    HAL_UARTEx_ReceiveToIdle_DMA(rtkCOM3Ptr, rtkCOM3RxBuff, sizeof(rtkCOM3RxBuff));

    usbToMain = xMessageBufferCreate(1024);
    mainToRTKCOM1 = xMessageBufferCreate(1024);
    rtkCOM1ToMain = xMessageBufferCreate(1024);
    rtkCOM3ToMain = xMessageBufferCreate(1024);
    mainToIMU = xMessageBufferCreate(1024);

    ResetRTK();
    ResetIMU();
    osDelay(100);
    InitIMU(MPU6050_CLOCK_PLL_XGYRO, MPU6050_ACCEL_FS_2, MPU6050_GYRO_FS_250);
    SetIMUDigitalLowPassFilter(MPU6050_DLPF_BW_5);
    SetIMUSampleRate(100);

    initFlag = 1;

    uint8_t usbRxBuffer[1024];
    uint32_t usbRxBufferLen;
    uint8_t rtkCOM1RxBuffer[1024];
    uint32_t rtkCOM1RxBufferLen;

    Segment segment;
    std::string line;

    while (1)
    {
        usbRxBufferLen = xMessageBufferReceive(usbToMain, usbRxBuffer, sizeof(usbRxBuffer), 0);
        if (usbRxBufferLen > 0)
        {
            // parse usbRxBuffer
            segment.Receive(usbRxBuffer, usbRxBufferLen);
        }
        rtkCOM1RxBufferLen = xMessageBufferReceive(rtkCOM1ToMain, rtkCOM1RxBuffer, sizeof(rtkCOM1RxBuffer), 0);
        if (rtkCOM1RxBufferLen > 0)
        {
            // parse rtkCOM1RxBuffer
            uint8_t cmd[2] = {0x80, 0x00};
            USB_Transmit(cmd, rtkCOM1RxBuffer, rtkCOM1RxBufferLen);
        }

        if (segment.GetOne(line))
        {
            if (line[0] == 0x00)
            {
                xMessageBufferSend(mainToRTKCOM1, (const void *)line.c_str(), line.length(), 10);
            }
            else if (line[0] == 0x01)
            {
                xMessageBufferSend(mainToIMU, (const void *)line.c_str(), line.length(), 10);
            }
        }

        osDelay(100);
    }
}

void StartRTKCOM1(void *argument)
{
    uint8_t mainRxBuffer[1024];
    uint32_t mainRxBufferLen;
    while (1)
    {
        if (initFlag)
        {
            mainRxBufferLen = xMessageBufferReceive(mainToRTKCOM1, mainRxBuffer, sizeof(mainRxBuffer), 0);
            if (mainRxBufferLen > 0)
            {
                // parse mainRxBuffer
                if (mainRxBuffer[1] == 0x00)
                {
                    SetRTKConf(mainRxBuffer + 2, mainRxBufferLen - 2);
                }
                else if (mainRxBuffer[1] == 0x01)
                {
                    if (mainRxBufferLen == 26)
                    {
                        double latitude, longitude, altitude;
                        latitude = *(double *)(mainRxBuffer + 2);
                        longitude = *(double *)(mainRxBuffer + 10);
                        altitude = *(double *)(mainRxBuffer + 18);
                        uint8_t cmd[2] = {0x80, 0x01};
                        uint8_t data = 0x00;
                        USB_Transmit(cmd, &data, 1);
                        SetRTKBaseWithPosition(latitude, longitude, altitude);
                    }
                    else if (mainRxBufferLen == 3)
                    {
                        uint8_t seconds;
                        seconds = mainRxBuffer[2];
                        uint8_t cmd[2] = {0x80, 0x01};
                        uint8_t data = 0x00;
                        USB_Transmit(cmd, &data, 1);
                        SetRTKBaseWithTime(seconds);
                    }
                    else
                    {
                        uint8_t cmd[2] = {0x80, 0x01};
                        uint8_t data = 0x01;
                        USB_Transmit(cmd, &data, 1);
                    }
                }
                else if (mainRxBuffer[1] == 0x02)
                {
                    if (mainRxBufferLen == 3)
                    {
                        uint8_t freq;
                        freq = mainRxBuffer[2];
                        uint8_t cmd[2] = {0x80, 0x02};
                        uint8_t data = 0x00;
                        USB_Transmit(cmd, &data, 1);
                        SetRTKRover(freq);
                    }
                    else
                    {
                        uint8_t cmd[2] = {0x80, 0x02};
                        uint8_t data = 0x01;
                        USB_Transmit(cmd, &data, 1);
                    }
                }
            }
        }

        osDelay(100);
    }
}

void StartRTKCOM3(void *argument)
{
    uint8_t rtkCOM3RxBuffer[1024];
    uint32_t rtkCOM3RxBufferLen;

    while (1)
    {
        if (initFlag)
        {
            rtkCOM3RxBufferLen = xMessageBufferReceive(rtkCOM3ToMain, rtkCOM3RxBuffer, sizeof(rtkCOM3RxBuffer), 0);
            if (rtkCOM3RxBufferLen > 0)
            {
                // parse rtkCOM3RxBuffer
                uint8_t cmd[2] = {0x80, 0x03};
                USB_Transmit(cmd, rtkCOM3RxBuffer, rtkCOM3RxBufferLen);
            }
        }

        osDelay(10);
    }
}

void StartIMU(void *argument)
{
    uint8_t mainRxBuffer[1024];
    uint32_t mainRxBufferLen;

    float measurements[6];

    uint32_t delay = 0;
    uint32_t delayCount = 0;

    while (1)
    {
        if (initFlag)
        {
            mainRxBufferLen = xMessageBufferReceive(mainToIMU, mainRxBuffer, sizeof(mainRxBuffer), 0);
            if (mainRxBufferLen > 0)
            {
                // parse mainRxBuffer
                if (mainRxBuffer[1] == 0x00)
                {
                    if (mainRxBufferLen == 3)
                    {
                        uint8_t value;
                        ReadIMUReg(mainRxBuffer[2], &value);
                        uint8_t cmd[2] = {0x81, 0x00};
                        uint8_t data[2];
                        data[0] = mainRxBuffer[2];
                        data[1] = value;
                        USB_Transmit(cmd, data, 2);
                    }
                }
                else if (mainRxBuffer[1] == 0x01)
                {
                    if (mainRxBufferLen == 4)
                    {
                        WriteIMUReg(mainRxBuffer[2], mainRxBuffer[3]);
                        uint8_t cmd[2] = {0x81, 0x01};
                        uint8_t data = 0x00;
                        USB_Transmit(cmd, &data, 1);
                    }
                    else
                    {
                        uint8_t cmd[2] = {0x81, 0x01};
                        uint8_t data = 0x01;
                        USB_Transmit(cmd, &data, 1);
                    }
                }
                else if (mainRxBuffer[1] == 0x02)
                {
                    if (mainRxBufferLen == 3)
                    {
                        bool valid = true;
                        switch (mainRxBuffer[2])
                        {
                        case 1:
                            delay = 100;
                            break;
                        case 5:
                            delay = 20;
                            break;
                        case 10:
                            delay = 10;
                            break;
                        case 50:
                            delay = 2;
                            break;
                        case 100:
                            delay = 1;
                            break;
                        default:
                            valid = false;
                            break;
                        }
                        uint8_t cmd[2] = {0x81, 0x02};
                        uint8_t data = 0x00;
                        if (!valid)
                        {
                            data = 0x01;
                        }
                        USB_Transmit(cmd, &data, 1);
                    }
                    else
                    {
                        uint8_t cmd[2] = {0x81, 0x02};
                        uint8_t data = 0x01;
                        USB_Transmit(cmd, &data, 1);
                    }
                }
            }
        }

        if (delayCount >= delay)
        {
            delayCount = 0;
            GetIMUAccel(measurements);
            GetIMUGyro(measurements + 3);
            uint8_t cmd[2] = {0x81, 0x03};
            USB_Transmit(cmd, (uint8_t *)measurements, 24);
        }
        delayCount++;

        osDelay(10);
    }
}
}
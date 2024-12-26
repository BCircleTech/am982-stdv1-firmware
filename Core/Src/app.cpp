#include "protocol.h"

extern "C"
{
#include "am982stdv1.h"
#include "message_buffer.h"
#include "usbd_cdc_if.h"

uint8_t initFlag = 0;

uint8_t rtkCOM1RxBuff[512];
uint8_t rtkCOM3RxBuff[512];

// MessageBufferHandle_t mainToUSB;
MessageBufferHandle_t usbToMain;
MessageBufferHandle_t mainToRTKCOM1;
MessageBufferHandle_t rtkCOM1ToMain;
MessageBufferHandle_t rtkCOM3ToMain;
MessageBufferHandle_t mainToIMU;
// MessageBufferHandle_t imuToMain;

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

void StartMain(void *argument)
{
    MX_USB_DEVICE_Init();

    HAL_UARTEx_ReceiveToIdle_DMA(rtkCOM1Ptr, rtkCOM1RxBuff, sizeof(rtkCOM1RxBuff));
    HAL_UARTEx_ReceiveToIdle_DMA(rtkCOM3Ptr, rtkCOM3RxBuff, sizeof(rtkCOM3RxBuff));

    // mainToUSB = xMessageBufferCreate(1024);
    usbToMain = xMessageBufferCreate(1024);
    mainToRTKCOM1 = xMessageBufferCreate(1024);
    rtkCOM1ToMain = xMessageBufferCreate(1024);
    rtkCOM3ToMain = xMessageBufferCreate(1024);
    mainToIMU = xMessageBufferCreate(1024);
    // imuToMain = xMessageBufferCreate(1024);

    ResetRTK();
    ResetIMU();
    osDelay(100);
    InitIMU(MPU6050_CLOCK_PLL_XGYRO, MPU6050_ACCEL_FS_2, MPU6050_GYRO_FS_250);
    SetIMUDigitalLowPassFilter(MPU6050_DLPF_BW_5);
    SetIMUSampleRate(100);

    initFlag = 1;

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
            std::string res;
            uint8_t cmd[2] = {0x80, 0x00};
            Segment::Pack(res, cmd, std::string((char *)rtkCOM1RxBuffer, rtkCOM1RxBufferLen));
            CDC_Transmit_HS((uint8_t *)res.c_str(), res.length());
        }
        rtkCOM3RxBufferLen = xMessageBufferReceive(rtkCOM3ToMain, rtkCOM3RxBuffer, sizeof(rtkCOM3RxBuffer), 0);
        if (rtkCOM3RxBufferLen > 0)
        {
            // parse rtkCOM3RxBuffer
            std::string res;
            uint8_t cmd[2] = {0x80, 0x03};
            Segment::Pack(res, cmd, std::string((char *)rtkCOM3RxBuffer, rtkCOM3RxBufferLen));
            CDC_Transmit_HS((uint8_t *)res.c_str(), res.length());
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
    size_t mainRxBufferLen;
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
                        std::string res;
                        uint8_t cmd[2] = {0x80, 0x01};
                        std::string data = {0x00};
                        Segment::Pack(res, cmd, data);
                        CDC_Transmit_HS((uint8_t *)res.c_str(), res.length());
                        SetRTKBaseWithPosition(latitude, longitude, altitude);
                    }
                    else if (mainRxBufferLen == 3)
                    {
                        uint8_t seconds;
                        seconds = mainRxBuffer[2];
                        std::string res;
                        uint8_t cmd[2] = {0x80, 0x01};
                        std::string data = {0x00};
                        Segment::Pack(res, cmd, data);
                        CDC_Transmit_HS((uint8_t *)res.c_str(), res.length());
                        SetRTKBaseWithTime(seconds);
                    }
                    else
                    {
                        std::string res;
                        uint8_t cmd[2] = {0x80, 0x01};
                        std::string data = {0x01};
                        Segment::Pack(res, cmd, data);
                        CDC_Transmit_HS((uint8_t *)res.c_str(), res.length());
                    }
                }
                else if (mainRxBuffer[1] == 0x02)
                {
                    if (mainRxBufferLen == 3)
                    {
                        uint8_t freq;
                        freq = mainRxBuffer[2];
                        std::string res;
                        uint8_t cmd[2] = {0x80, 0x02};
                        std::string data = {0x00};
                        Segment::Pack(res, cmd, data);
                        CDC_Transmit_HS((uint8_t *)res.c_str(), res.length());
                        SetRTKRover(freq);
                    }
                    else
                    {
                        std::string res;
                        uint8_t cmd[2] = {0x80, 0x02};
                        std::string data = {0x01};
                        Segment::Pack(res, cmd, data);
                        CDC_Transmit_HS((uint8_t *)res.c_str(), res.length());
                    }
                }
            }
        }

        osDelay(100);
    }
}

void StartRTKCOM3(void *argument)
{
    while (1)
    {
        if (initFlag)
        {
        }

        osDelay(100);
    }
}

void StartIMU(void *argument)
{
    uint8_t mainRxBuffer[1024];
    size_t mainRxBufferLen;

    float accel[3];
    float gyro[3];

    // std::string measurements;
    // std::string accelAndGyro(48);
    uint16_t delay = 0;
    uint16_t delayCount = 0;

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
                        std::string res;
                        uint8_t cmd[2] = {0x81, 0x00};
                        std::string data = {0x00, 0x00};
                        data[0] = mainRxBuffer[2];
                        data[1] = value;
                        Segment::Pack(res, cmd, data);
                        CDC_Transmit_HS((uint8_t *)res.c_str(), res.length());
                    }
                }
                else if (mainRxBuffer[1] == 0x01)
                {
                    if (mainRxBufferLen == 4)
                    {
                        WriteIMUReg(mainRxBuffer[2], mainRxBuffer[3]);
                        std::string res;
                        uint8_t cmd[2] = {0x81, 0x01};
                        std::string data = {0x00};
                        Segment::Pack(res, cmd, data);
                        CDC_Transmit_HS((uint8_t *)res.c_str(), res.length());
                    }
                }
                else if (mainRxBuffer[1] == 0x02)
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
                    std::string res;
                    uint8_t cmd[2] = {0x81, 0x02};
                    std::string data = {0x00};
                    if (!valid)
                    {
                        data[0] = 0x01;
                    }
                    Segment::Pack(res, cmd, data);
                    CDC_Transmit_HS((uint8_t *)res.c_str(), res.length());
                }
            }
        }

        if (delayCount == delay)
        {
            delayCount = 0;
            GetIMUAccel(accel);
            GetIMUGyro(gyro);
            // for (unsigned int i = 0; i < 8; i++)
            // {
            //     accelAndGyro[i] = (accel[0] >> (i * 8)) & 0xff;
            // }
            // for (unsigned int i = 0; i < 8; i++)
            // {
            //     accelAndGyro[i + 8] = (accel[1] >> (i * 8)) & 0xff;
            // }
            // for (unsigned int i = 0; i < 8; i++)
            // {
            //     accelAndGyro[i + 16] = (accel[2] >> (i * 8)) & 0xff;
            // }
            // for (unsigned int i = 0; i < 8; i++)
            // {
            //     accelAndGyro[i + 24] = (gyro[0] >> (i * 8)) & 0xff;
            // }
            // for (unsigned int i = 0; i < 8; i++)
            // {
            //     accelAndGyro[i + 32] = (gyro[1] >> (i * 8)) & 0xff;
            // }
            // for (unsigned int i = 0; i < 8; i++)
            // {
            //     accelAndGyro[i + 40] = (gyro[2] >> (i * 8)) & 0xff;
            // }
            // Segment::Pack(measurements, 0x8103, accelAndGyro);
            // CDC_Transmit_HS((uint8_t *)measurements.c_str(), measurements.length());
        }
        delayCount++;

        osDelay(10);
    }
}
}
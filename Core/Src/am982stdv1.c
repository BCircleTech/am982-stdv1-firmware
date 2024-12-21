#include "am982stdv1.h"

UART_HandleTypeDef *rtkCOM1Ptr = &huart9;
UART_HandleTypeDef *rtkCOM3Ptr = &huart2;
UART_HandleTypeDef *boardUARTPtr = &huart4;

void LedRunOn()
{
    HAL_GPIO_WritePin(LED_RUN_GPIO_Port, LED_RUN_Pin, GPIO_PIN_SET);
}

void LedRunOff()
{
    HAL_GPIO_WritePin(LED_RUN_GPIO_Port, LED_RUN_Pin, GPIO_PIN_RESET);
}

void LedErrOn()
{
    HAL_GPIO_WritePin(LED_ERR_GPIO_Port, LED_ERR_Pin, GPIO_PIN_SET);
}

void LedErrOff()
{
    HAL_GPIO_WritePin(LED_ERR_GPIO_Port, LED_ERR_Pin, GPIO_PIN_RESET);
}

void ResetRTK()
{
    HAL_GPIO_WritePin(RTK_NRST_GPIO_Port, RTK_NRST_Pin, GPIO_PIN_RESET);
    osDelay(100);
    HAL_GPIO_WritePin(RTK_NRST_GPIO_Port, RTK_NRST_Pin, GPIO_PIN_SET);
}

void SetRTKBaseWithPosition(double latitude, double longitude, double altitude)
{
    uint8_t cmd_0[] = "freset\r\n";
    uint8_t cmd_1[128] = {0};
    uint8_t cmd_2[] = "rtcm1006 com2 10\r\n";
    uint8_t cmd_3[] = "rtcm1033 com2 10\r\n";
    uint8_t cmd_4[] = "rtcm1074 com2 1\r\n";
    uint8_t cmd_5[] = "rtcm1124 com2 1\r\n";
    uint8_t cmd_6[] = "rtcm1084 com2 1\r\n";
    uint8_t cmd_7[] = "rtcm1094 com2 1\r\n";
    uint8_t cmd_8[] = "saveconfig\r\n";
    int cmd_1_len = snprintf((char *)cmd_1, sizeof(cmd_1), "mode base %.9f %.9f %.2f\r\n", latitude, longitude, altitude);
    HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_0, sizeof(cmd_0) - 1);
    osDelay(10000);
    HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_1, cmd_1_len);
    osDelay(100);
    HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_2, sizeof(cmd_2) - 1);
    osDelay(100);
    HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_3, sizeof(cmd_3) - 1);
    osDelay(100);
    HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_4, sizeof(cmd_4) - 1);
    osDelay(100);
    HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_5, sizeof(cmd_5) - 1);
    osDelay(100);
    HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_6, sizeof(cmd_6) - 1);
    osDelay(100);
    HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_7, sizeof(cmd_7) - 1);
    osDelay(100);
    HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_8, sizeof(cmd_8) - 1);
}

void SetRTKBaseWithTime(unsigned int seconds)
{
    uint8_t cmd_0[] = "freset\r\n";
    uint8_t cmd_1[128] = {0};
    uint8_t cmd_2[] = "rtcm1006 com2 10\r\n";
    uint8_t cmd_3[] = "rtcm1033 com2 10\r\n";
    uint8_t cmd_4[] = "rtcm1074 com2 1\r\n";
    uint8_t cmd_5[] = "rtcm1124 com2 1\r\n";
    uint8_t cmd_6[] = "rtcm1084 com2 1\r\n";
    uint8_t cmd_7[] = "rtcm1094 com2 1\r\n";
    uint8_t cmd_8[] = "saveconfig\r\n";
    int cmd_1_len = snprintf((char *)cmd_1, sizeof(cmd_1), "mode base time %d\r\n", seconds);
    HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_0, sizeof(cmd_0) - 1);
    osDelay(10000);
    HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_1, cmd_1_len);
    osDelay(100);
    HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_2, sizeof(cmd_2) - 1);
    osDelay(100);
    HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_3, sizeof(cmd_3) - 1);
    osDelay(100);
    HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_4, sizeof(cmd_4) - 1);
    osDelay(100);
    HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_5, sizeof(cmd_5) - 1);
    osDelay(100);
    HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_6, sizeof(cmd_6) - 1);
    osDelay(100);
    HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_7, sizeof(cmd_7) - 1);
    osDelay(100);
    HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_8, sizeof(cmd_8) - 1);
}

void SetRTKRover(unsigned int freq)
{
    uint8_t cmd_0[] = "freset\r\n";
    uint8_t cmd_1[] = "mode rover\r\n";
    uint8_t cmd_2_1hz[] = "gpgga com3 1\r\n";
    uint8_t cmd_2_2hz[] = "gpgga com3 0.5\r\n";
    uint8_t cmd_2_5hz[] = "gpgga com3 0.2\r\n";
    uint8_t cmd_2_10hz[] = "gpgga com3 0.1\r\n";
    uint8_t cmd_2_20hz[] = "gpgga com3 0.05\r\n";
    uint8_t cmd_2_50hz[] = "gpgga com3 0.02\r\n";
    uint8_t cmd_3_1hz[] = "gpths com3 1\r\n";
    uint8_t cmd_3_2hz[] = "gpths com3 0.5\r\n";
    uint8_t cmd_3_5hz[] = "gpths com3 0.2\r\n";
    uint8_t cmd_3_10hz[] = "gpths com3 0.1\r\n";
    uint8_t cmd_3_20hz[] = "gpths com3 0.05\r\n";
    uint8_t cmd_3_50hz[] = "gpths com3 0.02\r\n";
    uint8_t cmd_4[] = "saveconfig\r\n";
    HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_0, sizeof(cmd_0) - 1);
    osDelay(10000);
    HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_1, sizeof(cmd_1) - 1);
    osDelay(100);
    switch (freq)
    {
    case 1:
        HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_2_1hz, sizeof(cmd_2_1hz) - 1);
        osDelay(100);
        HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_3_1hz, sizeof(cmd_3_1hz) - 1);
        break;
    case 2:
        HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_2_2hz, sizeof(cmd_2_2hz) - 1);
        osDelay(100);
        HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_3_2hz, sizeof(cmd_3_2hz) - 1);
        break;
    case 5:
        HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_2_5hz, sizeof(cmd_2_5hz) - 1);
        osDelay(100);
        HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_3_5hz, sizeof(cmd_3_5hz) - 1);
        break;
    case 10:
        HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_2_10hz, sizeof(cmd_2_10hz) - 1);
        osDelay(100);
        HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_3_10hz, sizeof(cmd_3_10hz) - 1);
        break;
    case 20:
        HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_2_20hz, sizeof(cmd_2_20hz) - 1);
        osDelay(100);
        HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_3_20hz, sizeof(cmd_3_20hz) - 1);
        break;
    case 50:
        HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_2_50hz, sizeof(cmd_2_50hz) - 1);
        osDelay(100);
        HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_3_50hz, sizeof(cmd_3_50hz) - 1);
        break;

    default:
        break;
    }
    osDelay(100);
    HAL_UART_Transmit_DMA(rtkCOM1Ptr, cmd_4, sizeof(cmd_4) - 1);
}

void SetRTKConf(uint8_t *cmd, uint16_t size)
{
    uint8_t *tmp = malloc(size + 2);
    memcpy(tmp, cmd, size);
    tmp[size] = '\r';
    tmp[size + 1] = '\n';
    HAL_UART_Transmit_DMA(rtkCOM1Ptr, tmp, size + 2);
    free(tmp);
}

void ResetIMU()
{
    HAL_GPIO_WritePin(IMU_RST_GPIO_Port, IMU_RST_Pin, GPIO_PIN_SET);
    osDelay(2000);
    HAL_GPIO_WritePin(IMU_RST_GPIO_Port, IMU_RST_Pin, GPIO_PIN_RESET);
}

void InitIMU()
{
}

void ReadIMUReg(uint8_t addr, uint8_t *data)
{
    HAL_I2C_Master_Transmit(&hi2c5, IMU_I2C_ADDR, &addr, 1, IMU_I2C_TIMEOUT);
    HAL_I2C_Master_Receive(&hi2c5, IMU_I2C_ADDR, data, 1, IMU_I2C_TIMEOUT);
}

void WriteIMUReg(uint8_t addr, uint8_t *data)
{
    uint8_t *tmp = malloc(2);
    tmp[0] = addr;
    tmp[1] = *data;
    HAL_I2C_Master_Transmit(&hi2c5, IMU_I2C_ADDR, tmp, 2, IMU_I2C_TIMEOUT);
    free(tmp);
}
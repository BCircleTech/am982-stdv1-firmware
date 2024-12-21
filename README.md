# AM982-STDV1 Firmware

This project is the firmware for the HederaTech AM982-STDV1 module. It is designed for both end users and developers. For developers, this project provides a good starting point. For end users, it offers basic functionality to interact with the UM982 and MPU6050 via USB.

## Connection

Use a USB Type-C cable to connect the host to the AM982-STDV1 module. A separate DC power supply is not required if you're not using the terminal block to power other devices, as the USB host can power the module.

## Protocol

The structure of a complete frame transmitted between the host and the module is as follows, in HEX.

|Header|Length|Command|Data|Checksum|Trailer|
|:---:|:---:|:---:|:---:|:---:|:---:|
|AC 53|XX XX|XX XX|XX XX ... XX|XX|35 CA|

* **Length** is the totle size of **Command** and **Data** in bytes.
* The size of **Data** is variable and can be 0, but no more than 65533.
* **Checksum** is derived from both **Command** and **Data**.

### Host to Module

|Command|Data|Description|
|:---:|:---:|:---|
|00 00|XX XX ... XX|Forward **Data** to the COM1 of the UM982.|
|00 01|Latitude Longitude Altitude|Set the module to RTK base with the given latitude, longitude, and altitude. The types of latitude, longitude, and altitude are doubles, each with 8 bytes. **Data** is optional and, if not provided, the RTK base will automatically initialize its position within the first few minutes.|
|00 02|XX|Set the module to RTK rover with frequency options 1Hz, 5Hz, 10Hz.|
|00 03||Get the RTK role.|
|01 00|XX|Read data from the register **XX** of the MPU6050.|
|01 01|XX YY|Write data **YY** to the register **XX** of the MPU6050.|
|01 02|XX XX|Set the frequency of the IMU data. Options are 1Hz, 5Hz, 10Hz, 50Hz, 100Hz.|

### Module to Host

|Command|Data|Description|
|:---:|:---:|:---|
|80 00|XX XX ... XX|Forward **Data** from the COM1 of the UM982.|
|80 03||Return the RTK role. 0 for base and 1 for rover.|
|80 04|XX XX ... XX|Forward **Data** from the COM3 of the UM982, if the module is set to RTK rover.|
|81 00|XX YY|Return data **YY** in the register **XX** of the MPU6050.|
|81 03|ACX ACY ACZ AVX AVY AVZ|Return the accelerations and angular velocities measured by the MPU6050. ACX, ACY, and ACZ represent the accelerations along the X-axis, Y-axis, and Z-axis, respectively. AVX, AVY, and AVZ represent the angular velocities along the X-axis, Y-axis, and Z-axis, respectively. Each value of acceleration and angular velocity is a double, with 8 bytes.|
|8F 00||Command parsing succeed.|
|8F 01||Command parsing Failed.|

### Protocol Summary

|Request|Response|Description|
|---|---|---|
|00 00|80 00|Send to and receive from the COM1 of the UM982.|
|00 01|8F 00 *or* 8F 01|Set the module as an RTK base.|
|00 02|8F 00 *or* 8F 01|Set the module as an RTK rover with frequency.|
|00 03|80 04|Get the RTK role.|
||80 04|Receive from the COM3 of the UM982, if the module is set to RTK rover.|
|01 00|81 00|Read data from the register of the MPU6050.|
|01 01|8F 00 *or* 8F 01|Write data to the register of the MPU6050.|
|01 02|8F 00 *or* 8F 01|Set the frequency of the IMU data.|
||81 03|Receive measurements from the IMU.|

## RTK Initialization

### Base

Send the following strings to UM982 via COM1:

```c
/* Reset UM982. */
uint8_t cmd_0[] = "freset\r\n";
/* Automatically initialize position within 60 seconds. */
uint8_t cmd_1[] = "mode base time 60\r\n";
/* Send RTK base data via COM2. */
uint8_t cmd_2[] = "rtcm1006 com2 10\r\n";
uint8_t cmd_3[] = "rtcm1033 com2 10\r\n";
uint8_t cmd_4[] = "rtcm1074 com2 1\r\n";
uint8_t cmd_5[] = "rtcm1124 com2 1\r\n";
uint8_t cmd_6[] = "rtcm1084 com2 1\r\n";
uint8_t cmd_7[] = "rtcm1094 com2 1\r\n";
/* Save */
uint8_t cmd_8[] = "saveconfig\r\n";
```

or

```c
/* Reset UM982. */
uint8_t cmd_0[] = "freset\r\n";
/* Set the latitude, longitude, and altitude if known. */
uint8_t cmd_1[] = "mode base 40.078983248 116.236601977 60.42\r\n";
/* Send RTK base data via COM2. */
uint8_t cmd_2[] = "rtcm1006 com2 10\r\n";
uint8_t cmd_3[] = "rtcm1033 com2 10\r\n";
uint8_t cmd_4[] = "rtcm1074 com2 1\r\n";
uint8_t cmd_5[] = "rtcm1124 com2 1\r\n";
uint8_t cmd_6[] = "rtcm1084 com2 1\r\n";
uint8_t cmd_7[] = "rtcm1094 com2 1\r\n";
/* Save */
uint8_t cmd_8[] = "saveconfig\r\n";
```

### Rover

Send the following strings to UM982 via COM1:

```c
/* Reset UM982. */
uint8_t cmd_0[] = "freset\r\n";
/* Set to rover. */
uint8_t cmd_1[] = "mode rover\r\n";
/* Receive latitude, longitude, and altitude message via COM3. */
uint8_t cmd_2[] = "gpgga com3 1\r\n";
/* Receive heading message via COM3. */
uint8_t cmd_3[] = "gpths com3 1\r\n";
/* Save */
uint8_t cmd_4[] = "saveconfig\r\n";
```
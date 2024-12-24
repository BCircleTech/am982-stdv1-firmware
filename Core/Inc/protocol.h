#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "stdint.h"

uint8_t Unpack(uint8_t *data, uint32_t len, uint8_t *payload, uint16_t *payloadLen);

#endif
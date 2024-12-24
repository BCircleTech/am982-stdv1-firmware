#include "protocol.h"

#define PACK_HEADER_1 0xac
#define PACK_HEADER_2 0x53
#define PACK_TRAILER_1 0x35
#define PACK_TRAILER_2 0xca

uint8_t unpackBuff[1024];
uint16_t unpackBuffLen = 0;

uint8_t Unpack(uint8_t *data, uint32_t len, uint8_t *payload, uint16_t *payloadLen)
{
    for (uint32_t i = 0; i < len; i++)
    {
    }
    return 0;
}
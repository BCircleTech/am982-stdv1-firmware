#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <cstdint>
#include <string>
#include <list>

class Segment
{
public:
    void Receive(uint8_t *data, uint32_t len);
    bool GetOne(std::string &str);

private:
    const uint8_t mHeaders[2] = {0xac, 0x53};
    const uint8_t mTrailers[2] = {0x35, 0xca};
    std::list<std::string> mSegments;
    std::string mBuffer;
    uint8_t mState = 0;
    unsigned int mTrailerBegin = 5;
};

#endif
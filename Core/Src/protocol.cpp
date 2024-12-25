#include "protocol.h"

void Segment::Receive(uint8_t *data, uint32_t len)
{
    mBuffer.append((char *)data, len);
    unsigned int bufferLen = mBuffer.length();
    unsigned int loop = bufferLen - 1;
    if (bufferLen > 1 && mState == 0)
    {
        for (unsigned int i = 0; i < loop; i++)
        {
            if (mBuffer[i] == mHeaders[0] && mBuffer[i + 1] == mHeaders[1])
            {
                mBuffer.erase(0, i);
                mState = 1;
                break;
            }
        }
        if (mState == 0)
        {
            mBuffer.erase(0, loop);
        }
    }
    if (mState == 1)
    {
        while (mTrailerBegin < loop)
        {
            if (mBuffer[mTrailerBegin] == mTrailers[0] && mBuffer[mTrailerBegin + 1] == mTrailers[1])
            {
                uint8_t checksum = 0;
                unsigned int checksumIndex = mTrailerBegin - 1;
                for (unsigned int i = 2; i < checksumIndex; i++)
                {
                    checksum += mBuffer[i];
                }
                if (mBuffer[checksumIndex] == checksum)
                {
                    mSegments.push_front(std::string(mBuffer, 2, checksumIndex - 2));
                }
                mBuffer.erase(0, mTrailerBegin + 2);
                mState = 0;
                mTrailerBegin = 5;
                break;
            }
            else
            {
                mTrailerBegin++;
            }
        }
    }
}

bool Segment::GetOne(std::string &str)
{
    if (!mSegments.empty())
    {
        str = mSegments.back();
        mSegments.pop_back();
        return true;
    }
    else
    {
        return false;
    }
}
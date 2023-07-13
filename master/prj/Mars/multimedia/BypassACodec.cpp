#include "BypassACodec.h"
#include <iostream>

using namespace std;

bool BypassACodec::encode(uint8_t* in, size_t isize, uint8_t* out, size_t* osize)
{
    memcpy(out, in, isize);
    *osize = isize;

    cout << "[BypassACodec] encode\n";

    return true;
}

bool BypassACodec::decode(uint8_t* in, size_t isize, uint8_t* out, size_t* osize)
{
    memcpy(out, in, isize);
    *osize = isize;

    cout << "[BypassACodec] decode\n";

    return true;
}

bool BypassACodec::setInputChannel(int n)
{
    return true;
}

bool BypassACodec::setInputBitrate(int r)
{
    return true;
}

bool BypassACodec::setOutputBitrate(int r)
{
    return true;
}

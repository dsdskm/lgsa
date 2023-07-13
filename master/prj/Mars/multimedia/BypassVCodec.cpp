#include "BypassVCodec.h"
#include <iostream>

using namespace std;

bool BypassVCodec::encode(uint8_t* in, size_t isize, uint8_t* out, size_t* osize)
{
    memcpy(out, in, isize);
    *osize = isize;

    cout << "[BypassVCodec] encode\n";

    return true;
}

bool BypassVCodec::decode(uint8_t* in, size_t isize, uint8_t* out, size_t* osize)
{
    memcpy(out, in, isize);
    *osize = isize;

    cout << "[BypassVCodec] decode\n";

    return true;
}

bool BypassVCodec::setInputResolution(int w, int h)
{
    return true;
}

bool BypassVCodec::setOutputResolution(int w, int h)
{
    return true;
}

bool BypassVCodec::setInputColorSpace(Colorspace cs)
{
    return true;
}

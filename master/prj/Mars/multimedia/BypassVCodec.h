#ifndef __BYPASSVCODEC_H__
#define __BYPASSVCODEC_H__

#include <vcruntime_string.h>
#include "Codec.h"

using namespace std;

class BypassVCodec : public VCodec {
public:
    bool encode(uint8_t* in, size_t isize, uint8_t* out, size_t *osize);
    bool decode(uint8_t* in, size_t isize, uint8_t* out, size_t *osize);
    bool setInputResolution(int w, int h);
    bool setOutputResolution(int w, int h);
    bool setInputColorSpace(Colorspace cs);
};

#endif

#ifndef __BYPASSACODEC_H__
#define __BYPASSACODEC_H__

#include <vcruntime_string.h>
#include "Codec.h"

using namespace std;

class BypassACodec : public ACodec {
public:
    bool encode(uint8_t* in, size_t isize, uint8_t* out, size_t* osize);
    bool decode(uint8_t* in, size_t isize, uint8_t* out, size_t* osize);

    bool setInputChannel(int n);
    bool setInputBitrate(int r);
    bool setOutputBitrate(int r);
};

#endif // !__BYPASSACODEC_H__

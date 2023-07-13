#ifndef __OPUSACODEC_H__
#define __OPUSACODEC_H__

#include <vcruntime_string.h>
#include "Codec.h"

#include "../..\\..\\LgVideoChatDemo\\opus-1.3.1\\include\opus.h"
#include "../..\\..\\LgVideoChatDemo\\opus-1.3.1\\include\opus_defines.h"

#ifdef _DEBUG
#pragma comment(lib,  "..\\..\\LgVideoChatDemo\\built-libs\\opus-d.lib")
#else
#pragma comment(lib,  "..\\..\\LgVideoChatDemo\\built-libs\\opus.lib")
#endif
#ifdef _DEBUG
#pragma comment(lib, "..\\..\\LgVideoChatDemo\\built-libs\\webrtcVAD-d.lib")
#else
#pragma comment(lib, "..\\..\\LgVideoChatDemo\\built-libs\\webrtcVAD.lib")
#endif

#define MAX_PACKET_SIZE (3*1276)

#define FRAME_SIZE 960
#define MAX_FRAME_SIZE 6*FRAME_SIZE

#define APPLICATION OPUS_APPLICATION_AUDIO
#define BITRATE 64000

#define SAMPLE_RATE 16000
#define CHANNELS 1
#define BITS_PER_SAMPLE 16
#define FRAMES_PER_BUFFER 160
#define BYTES_PER_BUFFER (FRAMES_PER_BUFFER*sizeof(short))

using namespace std;

class OPUSACodec : public ACodec {
    OpusEncoder* mEncoder;
    OpusDecoder* mDecoder;

public:
    OPUSACodec();
    bool encode(uint8_t* in, size_t isize, uint8_t* out, size_t* osize);
    bool decode(uint8_t* in, size_t isize, uint8_t* out, size_t* osize);

    bool setInputChannel(int n);
    bool setInputBitrate(int r);
    bool setOutputBitrate(int r);
};

#endif // !__OPUSACODEC_H__

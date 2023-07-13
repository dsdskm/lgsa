#include "OPUSACodec.h"
#include <iostream>
#include "../MarsLog.h"

using namespace std;

OPUSACodec::OPUSACodec()
{
    int err = 0;

    mEncoder = opus_encoder_create(SAMPLE_RATE, CHANNELS, APPLICATION, &err);
    if (err < 0)
    {
        cout << "[OPUSACodec] encoder create error : %s\n" << opus_strerror(err);
        return;
    }

    err = opus_encoder_ctl(mEncoder, OPUS_SET_BITRATE(BITRATE));
    if (err < 0)
    {
        cout << "[OPUSACodec]failed to set bitrate: %s\n" << opus_strerror(err);
        return;
    }

    mDecoder = opus_decoder_create(SAMPLE_RATE, CHANNELS, &err);
    if (err < 0)
    {
        cout << "[OPUSACodec] decoder create error : %s\n" << opus_strerror(err);
        return;
    }
}

bool OPUSACodec::encode(uint8_t* in, size_t isize, uint8_t* out, size_t* osize)
{
    int err = 0;

    static opus_int16 mInput[FRAMES_PER_BUFFER];

    /* Convert from little-endian ordering. */
    for (int j = 0; j < FRAMES_PER_BUFFER; j++)
    {
        mInput[j] = in[2 * j + 1] << 8 | in[2 * j];
    }

    opus_int32 nbBytes;
    nbBytes = opus_encode(mEncoder, mInput, FRAMES_PER_BUFFER, out, BYTES_PER_BUFFER);
    if (nbBytes < 0)
    {
        cout << "[OPUSACodec] failed to encode: %s\n" << opus_strerror(err);
    }

    *osize = (size_t)nbBytes;

    ALOG_D("[OPUSACodec] encode done\n");

    return true;
}

bool OPUSACodec::decode(uint8_t* in, size_t isize, uint8_t* out, size_t* osize)
{
    int frameSize = 0;
    static opus_int16 outBuffer[FRAMES_PER_BUFFER];
    uint8_t* pcm_bytes = out;

    frameSize = opus_decode(mDecoder, in, isize, outBuffer, FRAMES_PER_BUFFER, 0);
    if (frameSize < 0)
    {
        cout << "[OPUSACodec] decode error : %s\n" << opus_strerror(frameSize);
        return false;
    }

    /* Convert to little-endian ordering. */
    for (int i = 0; i < FRAMES_PER_BUFFER; i++)
    {
        pcm_bytes[2 * i] = outBuffer[i] & 0xFF;
        pcm_bytes[2 * i + 1] = (outBuffer[i] >> 8) & 0xFF;
    }

    *osize = BYTES_PER_BUFFER;

    ALOG_D("[OPUSACodec] decode done\n");

    return true;
}

bool OPUSACodec::setInputChannel(int n)
{
    return true;
}

bool OPUSACodec::setInputBitrate(int r)
{
    return true;
}

bool OPUSACodec::setOutputBitrate(int r)
{
    return true;
}

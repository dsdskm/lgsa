#ifndef __AUDIOCHUNK_H__
#define __AUDIOCHUNK_H__

#include <memory>

/* 1 centisec (1/100) */
struct AudioChunk {
    char data[320];
    size_t NumberOfPayloadByte;
    AudioChunk() : NumberOfPayloadByte(320) {
        memset(data, 0, sizeof(data));
    }
};

static inline void init_flusher(AudioChunk *ac)
{
    memset(ac->data, 0, sizeof(ac->data));
    ac->data[0] = 0xb;
    ac->data[1] = 0xe;
    ac->data[2] = 0xe;
    ac->data[3] = 0xf;

    ac->data[316] = 0xb;
    ac->data[317] = 0xe;
    ac->data[318] = 0xe;
    ac->data[319] = 0xf;
}

static inline bool is_flusher(AudioChunk* ac)
{
    return ac->data[0] == 0xb && ac->data[1] == 0xe && ac->data[2] == 0xe && ac->data[3] == 0xf
        && ac->data[316] == 0xb && ac->data[317] == 0xe && ac->data[318] == 0xe && ac->data[319] == 0xf;
}

#endif

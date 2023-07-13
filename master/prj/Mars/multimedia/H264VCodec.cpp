#include <vector>
#include <iostream>
#include "H264VCodec.h"
#include "../MarsLog.h"

using namespace std;

H264VCodec::H264VCodec()
    :
        width(640),
        height(480),
        maxBitrate(200000),
        minBitrate(10000),
        bitrate(150000),
        framerate(15),
        goplen(5),
        maxbframe(1)
{
    VLOG_I("[H264VCodec] construct..\n");

    /* for ENCODER */
    {
        enc_avcodec = avcodec_find_encoder(AV_CODEC_ID_H264);
        if (!enc_avcodec)
        {
            cout << "[H264VCodec] avcodec_find_encoder failed.\n";
            exit(1);
        }

        AVRational enc_tb = { 1, framerate };
        AVRational enc_fr = { framerate, 1 };

        for (int br = minBitrate; br <= maxBitrate; br += 20000)
        {
            AVCodecContext* pEnc_avcontext;

            pEnc_avcontext = avcodec_alloc_context3(enc_avcodec);
            if (!pEnc_avcontext)
            {
                cout << "[H264VCodec] avcodec_alloc_context3 for encoder failed.\n";
                exit(1);
            }

            pEnc_avcontext->bit_rate = br;
            pEnc_avcontext->width = width;
            pEnc_avcontext->height = height;
            pEnc_avcontext->time_base = enc_tb;
            pEnc_avcontext->framerate = enc_fr;
            pEnc_avcontext->gop_size = goplen;
            pEnc_avcontext->max_b_frames = maxbframe;
            pEnc_avcontext->pix_fmt = AV_PIX_FMT_YUV420P;

            if (avcodec_open2(pEnc_avcontext, enc_avcodec, NULL) < 0)
            {
                cout << "[H264VCodec] avcodec_open2() for encoder failed\n";
                exit(1);
            }

            aEnc_avcontext[br] = pEnc_avcontext;
        }
        auto it = aEnc_avcontext.lower_bound(bitrate);
        if (it != aEnc_avcontext.end())
        {
            bitrate = it->first;
            cEnc_avcontext.store(it->second);
        }
        else
        {
            VLOG_E("[H264VCodec] Invalid bitrate %d\n", bitrate);
            bitrate = aEnc_avcontext.rbegin()->first;
            cEnc_avcontext.store(aEnc_avcontext.rbegin()->second);
        }

        AVCodecContext *enc_avcontext = cEnc_avcontext.load();

        /* BGR -> NV12 software converter */
        enc_swsctx = sws_getContext(
                enc_avcontext->width, enc_avcontext->height, AV_PIX_FMT_BGR24,
                enc_avcontext->width, enc_avcontext->height, enc_avcontext->pix_fmt,
                SWS_BILINEAR, NULL, NULL, NULL);
        if (!enc_swsctx)
        {
            cout << "[H264VCodec] sws_getContext() failed\n";
            exit(1);
        }
    }

    /* for DECODER */
    {
        dec_avcodec = avcodec_find_decoder(AV_CODEC_ID_H264);
        if (!dec_avcodec)
        {
            cout << "[H264VCodec] avcodec_find_encoder failed.\n";
            exit(1);
        }

        dec_avcontext = avcodec_alloc_context3(dec_avcodec);
        if (!dec_avcontext)
        {
            cout << "[H264VCodec] avcodec_alloc_context3 for decoder failed.\n";
            exit(1);
        }

        AVRational dec_tb = { 1, framerate };
        AVRational dec_fr = { framerate, 1 };

        dec_avcontext->bit_rate = bitrate;
        dec_avcontext->width = width;
        dec_avcontext->height = height;
        dec_avcontext->time_base = dec_tb;
        dec_avcontext->framerate = dec_fr;
        dec_avcontext->gop_size = goplen;
        dec_avcontext->max_b_frames = maxbframe;
        dec_avcontext->pix_fmt = AV_PIX_FMT_YUV420P;

        if (avcodec_open2(dec_avcontext, dec_avcodec, NULL) < 0)
        {
            cout << "[H264VCodec] avcodec_open2() for decoder failed\n";
            exit(1);
        }

        /* NV12 -> BGR software converter */
        dec_swsctx = sws_getContext(
                dec_avcontext->width, dec_avcontext->height, dec_avcontext->pix_fmt,
                dec_avcontext->width, dec_avcontext->height, AV_PIX_FMT_BGR24,
                SWS_BILINEAR, NULL, NULL, NULL);
        if (!dec_swsctx)
        {
            cout << "[H264VCodec] sws_getContext() failed\n";
            exit(1);
        }
    }

    cout << "[H264VCodec] construct success.\n";
}

bool H264VCodec::encode(uint8_t* in, size_t isize, uint8_t* out, size_t* osize)
{
    VLOG_D("[H264VCodec] encode start\n");
    AVCodecContext *enc_avcontext = cEnc_avcontext.load();

    /* prepare frame */
    AVFrame* frame = av_frame_alloc();
    if (!frame)
    {
        cout << "[H264VCodec ENC] av_frame_alloc failed\n";
        return false;
    }

    frame->format = enc_avcontext->pix_fmt;
    frame->width = enc_avcontext->width;
    frame->height = enc_avcontext->height;

    if (av_frame_get_buffer(frame, 32) < 0)
    {
        cout << "[H264VCodec ENC] av_frame_get_buffer failed\n";
        av_frame_unref(frame);
        return false;
    }

    /* sws_scale() 로 BGR -> NV12 변환 수행 */
    uint8_t* srcslice[1] = { in };
    int srcstride[1] = { enc_avcontext->width * 3 };

    sws_scale(
            enc_swsctx,
            srcslice, srcstride, 0, enc_avcontext->height,
            frame->data, frame->linesize);

    /* NV12 -> encode -> H264 NAL */
    AVPacket* pkt = av_packet_alloc();
    if (!pkt)
    {
        cout << "[H264VCodec ENC] av_packet_alloc() failed\n";
        av_frame_unref(frame);
        return false;
    }

    /* set PTS (temporary */
    frame->pts = enc_avcontext->frame_number++;

    if (avcodec_send_frame(enc_avcontext, frame) < 0)
    {
        cout << "[H264VCodec ENC] avcodec_send_frame() failed\n";
        av_packet_unref(pkt);
        av_frame_unref(frame);
        return false;
    }

    /* It will success after feed enought data */
    if (avcodec_receive_packet(enc_avcontext, pkt) < 0)
    {
        cout << "[H264VCodec ENC] avcodec_receive_packet() failed\n";
        av_packet_unref(pkt);
        av_frame_unref(frame);
        return false;
    }

    VLOG_D("[H264VCodec] encode size : %d\n", pkt->size);

    if (!out | !osize)
    {
        cout << "[H264VCodec ENC] invalid output buffer\n";
        av_packet_unref(pkt);
        av_frame_unref(frame);
        return false;
    }

    if (*osize < pkt->size)
    {
        cout << "[H264VCodec ENC] output buffer too small " << pkt->size << "/" << *osize << "\n";
        av_packet_unref(pkt);
        av_frame_unref(frame);
        return false;
    }

    /* copy the encoded data to the output */
    memcpy(out, pkt->data, pkt->size);
    *osize = pkt->size;

    /* cleanup */
    av_packet_unref(pkt);
    av_frame_unref(frame);

    return true;
}

bool H264VCodec::decode(uint8_t* in, size_t isize, uint8_t* out, size_t* osize)
{
    VLOG_D("[H264VCodec DEC] decode start\n");

    AVPacket* pkt = av_packet_alloc();
    if (!pkt)
    {
        cout << "[H264VCodec DEC] av_packet_alloc() failed\n";
        return false;
    }

    /* prepare av_malloc()-ed buffer */
    uint8_t* buffer = (uint8_t*)av_malloc(isize);
    if (!buffer)
    {
        cout << "[H264VCodec DEC] av_malloc() failed\n";
        return false;
    }

    memcpy(buffer, in, isize);

    av_packet_from_data(pkt, buffer, isize);

    /* Send the packet with the compressed data to the decoder */
    if (avcodec_send_packet(dec_avcontext, pkt) < 0)
    {
        cout << "[H264VCodec DEC] avcodec_send_packet() failed\n";
        av_packet_unref(pkt);
        return false;
    }

    /* Prepare a frame that will hold the decompressed data */
    AVFrame* frame = av_frame_alloc();
    if (!frame)
    {
        cout << "[H264VCodec DEC] av_frame_alloc failed\n";
        av_packet_unref(pkt);
        return false;
    }

    if (avcodec_receive_frame(dec_avcontext, frame) < 0)
    {
        cout << "[H264VCodec DEC] avcodec_receive_frame() failed\n";
        av_frame_unref(frame);
        av_packet_unref(pkt);
        return false;
    }

    VLOG_D("[H264VCodec DEC] decode size : %d\n", frame->pkt_size);

    /* NV12 -> BGR24. */
    AVFrame* bgrFrame = av_frame_alloc();
    if (!bgrFrame)
    {
        cout << "[H264VCodec] av_frame_alloc failed\n";
        av_frame_unref(frame);
        av_packet_unref(pkt);
        return false;
    }

    bgrFrame->format = AV_PIX_FMT_BGR24;
    bgrFrame->width = dec_avcontext->width;
    bgrFrame->height = dec_avcontext->height;

    if (av_frame_get_buffer(bgrFrame, 32) < 0)
    {
        cout << "[H264VCodec DEC] av_frame_get_buffer failed\n";
        av_frame_unref(bgrFrame);
        av_frame_unref(frame);
        av_packet_unref(pkt);
        return false;
    }

    /* Do the conversion. */
    sws_scale(
            dec_swsctx,
            frame->data, frame->linesize, 0, dec_avcontext->height,
            bgrFrame->data, bgrFrame->linesize);

    if (!out | !osize)
    {
        cout << "[H264VCodec DEC] invalid output buffer\n";
        av_frame_unref(frame);
        av_packet_unref(pkt);
        return false;
    }

    int size_needed = av_image_get_buffer_size(AV_PIX_FMT_BGR24, bgrFrame->width, bgrFrame->height, 1);
    if (*osize < size_needed)
    {
        cout << "[H264VCodec DEC] output buffer too small " << size_needed << "/" << *osize << "\n";
        av_frame_unref(bgrFrame);
        av_frame_unref(frame);
        av_packet_unref(pkt);
        return false;
    }

    /* Copy the decompressed data to the output */
    av_image_copy_to_buffer(out, *osize, bgrFrame->data, bgrFrame->linesize, AV_PIX_FMT_BGR24, bgrFrame->width, bgrFrame->height, 1);
    *osize = size_needed;

    /* cleanup */
    av_frame_unref(bgrFrame);
    av_frame_unref(frame);
    av_packet_unref(pkt);

    return true;
}

bool H264VCodec::setInputResolution(int w, int h)
{
    return true;
}

bool H264VCodec::setOutputResolution(int w, int h)
{
    return true;
}

bool H264VCodec::setInputColorSpace(Colorspace cs)
{
    return true;
}

bool H264VCodec::increaseQuality(void)
{
    if (bitrate >= maxBitrate)
        return false;

    int nBitrate = min(bitrate + 40000, maxBitrate);

    auto it = aEnc_avcontext.lower_bound(nBitrate);

    if (it == aEnc_avcontext.end())
        return false;

    VLOG_I("[H264VCodec] Q %d -> %d\n", bitrate, nBitrate);

    bitrate = nBitrate;
    cEnc_avcontext.store(it->second);

    return true;
}

bool H264VCodec::decreaseQuality(void)
{
    if (bitrate <= minBitrate)
        return false;

    int nBitrate = max(bitrate - 40000, minBitrate);

    auto it = aEnc_avcontext.lower_bound(nBitrate);

    if (it == aEnc_avcontext.end())
        return false;

    VLOG_I("[H264VCodec] Q %d -> %d\n", bitrate, nBitrate);

    bitrate = nBitrate;
    cEnc_avcontext.store(it->second);

    return true;
}

#include "MultimediaStreamReceiver.h"
#include "VideoFrame.h"
#include "AudioChunk.h"
#include "../MarsLog.h"

using namespace net;

MultimediaStreamReceiver::MultimediaStreamReceiver(
    string pia, StreamController *sc,
    RemoteVideoStream* rvs, RemoteAudioStream* ras,
    MultimediaStreamAdjustment *adj)
    : peerIdentifier(0), peerIpAddress(pia), streamController(sc),
    remoteVideoStream(rvs), remoteAudioStream(ras),
    adjustment(adj)
{
}

void MultimediaStreamReceiver::OnNetworkScore(int score)
{
    if (adjustment)
        adjustment->adjustScore(score);
}

void MultimediaStreamReceiver::OnReceive(StreamType type, const int peer_id, int stream_id, size_t recv_size)
{
    size_t read_bytes = 0;
    size_t total_read_bytes = 0;

    switch (type)
    {
        case STREAM_VIDEO:
        {
            uint8_t* data = new uint8_t[recv_size];
            VideoFrame vf(640, 480, 3, data, recv_size);

            for (total_read_bytes = 0; total_read_bytes < recv_size; total_read_bytes += read_bytes)
            {
                read_bytes = streamController->Receive(
                        peer_id, stream_id, (char*)data + total_read_bytes,
                        recv_size - total_read_bytes);

                if (read_bytes <= 0)
                    break;
            }

            if (total_read_bytes == recv_size)
                remoteVideoStream->put(vf);
            else
                LOG_E("[MultimediaStreamReceiver %s] OnReceive warning !! %d/%d\n",
                        peerIpAddress.c_str(), total_read_bytes, recv_size);

            break;
        }

        case STREAM_AUDIO:
        {
            AudioChunk *ac = new AudioChunk();

            for (total_read_bytes = 0; total_read_bytes < recv_size; total_read_bytes += read_bytes)
            {
                read_bytes = streamController->Receive(
                        peer_id, stream_id, ac->data + total_read_bytes,
                        recv_size - total_read_bytes);

                if (read_bytes <= 0)
                    break;
            }

            if (total_read_bytes == recv_size)
            {
                ac->NumberOfPayloadByte = recv_size;
                remoteAudioStream->put(ac);
            }
            else
            {
                LOG_E("[MultimediaStreamReceiver %s] OnReceive warning %d/%d\n",
                        peerIpAddress.c_str(), total_read_bytes, recv_size);
            }

            break;
        }

        case STREAM_DATA:
        {
            break;
        }

        default:
        {
        }
    }
}

void MultimediaStreamReceiver::run(void)
{
    remoteVideoStream->run();
    remoteAudioStream->run();
}

void MultimediaStreamReceiver::stop(void)
{
    LOG_I("[MultimediaStreamReceiver %s] send stop\n", peerIpAddress.c_str());

    remoteVideoStream->stop();
    LOG_I("[MultimediaStreamReceiver %s] stop RemoteVideoStream\n");
    delete remoteVideoStream;
    remoteAudioStream->stop();
    LOG_I("[MultimediaStreamReceiver %s] stop RemoteAudioStream\n");
    delete remoteAudioStream;
}

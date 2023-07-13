#include <iostream>
#include <MMDeviceApi.h>
#include <AudioEngineEndPoint.h>
#include <Audioclient.h>
#include <avrt.h>
#include "SpeakerRenderer.h"
#include "../MarsLog.h"

using namespace std;

bool SpeakerRenderer::boostPriority(void)
{
    HANDLE hThread = t.native_handle();

    return SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);
}

void SpeakerRenderer::run(void)
{
    t = thread(&SpeakerRenderer::work, this);
}

void SpeakerRenderer::work(void)
{
    IMMDevice* pDevice = nullptr;
    IMMDeviceEnumerator* pDeviceEnumerator = nullptr;
    IAudioClient* pAudioClient = nullptr;
    IAudioRenderClient* pAudioRenderClient = nullptr;
    HANDLE hEvent = nullptr;
    HRESULT hr = S_OK;
    bool VoipStarted = false;
    int index;
    DWORD  dwFlags;
    DWORD avrtTaskIndex;

    boostPriority();

    try
    {
        AvSetMmThreadCharacteristicsA("Voip Audio", &avrtTaskIndex);
        hr = CoInitializeEx(NULL, COINIT_SPEED_OVER_MEMORY);
        if (FAILED(hr)) throw runtime_error("CoInitialize error");

        hr = CoCreateInstance(
                __uuidof(MMDeviceEnumerator),
                nullptr,
                CLSCTX_ALL,
                __uuidof(IMMDeviceEnumerator),
                (void**)&pDeviceEnumerator);

        if (FAILED(hr))
        {
            std::cout << "hr=0x" << std::hex << hr << std::endl;
            throw std::runtime_error("CoCreateInstance error");
        }

        hr = pDeviceEnumerator->GetDefaultAudioEndpoint(
                eRender,
                eConsole,
                &pDevice);

        if (FAILED(hr))
            throw std::runtime_error("IMMDeviceEnumerator.GetDefaultAudioEndpoint error");

        std::cout << "IMMDeviceEnumerator.GetDefaultAudioEndpoint()->OK" << std::endl;

        hr = pDevice->Activate(
            __uuidof(IAudioClient),
            CLSCTX_ALL,
            nullptr,
            (void**)&pAudioClient);

        if (FAILED(hr))
            throw std::runtime_error("IMMDevice.Activate error");

        std::cout << "IMMDevice.Activate()->OK" << std::endl;

        REFERENCE_TIME MinimumDevicePeriod = 10000000ULL / 2;

        // hr = pAudioClient->GetDevicePeriod(nullptr, &MinimumDevicePeriod);
        // if (FAILED(hr)) throw std::runtime_error("IAudioClient.GetDevicePeriod error");

        std::cout << "minimum device period=" << MinimumDevicePeriod * 100 << "[nano seconds]" << std::endl;

        WAVEFORMATEX wave_format = {};
        wave_format.wFormatTag = WAVE_FORMAT_PCM;
        wave_format.nChannels = 1;
        wave_format.nSamplesPerSec = SAMPLE_RATE;
        wave_format.wBitsPerSample = sizeof(short) * 8;
        wave_format.nBlockAlign = (wave_format.nChannels * wave_format.wBitsPerSample) / 8;
        wave_format.nAvgBytesPerSec = wave_format.nSamplesPerSec * wave_format.nBlockAlign;
        wave_format.cbSize = 0;

        hr = pAudioClient->Initialize(
                AUDCLNT_SHAREMODE_SHARED,
                AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM | AUDCLNT_STREAMFLAGS_EVENTCALLBACK | AUDCLNT_STREAMFLAGS_NOPERSIST,
                MinimumDevicePeriod,
                0,
                &wave_format,
                nullptr);

        if (FAILED(hr))
        {
            std::cout << "hr=0x" << std::hex << hr << std::endl;
            throw std::runtime_error("IAudioClient.Initialize error");
        }

        std::cout << "IAudioClient.Initialize()->OK" << std::endl;

        hEvent = CreateEvent(nullptr, false, false, nullptr);
        if (FAILED(hr))
            throw std::runtime_error("CreateEvent error");

        hr = pAudioClient->SetEventHandle(hEvent);
        if (FAILED(hr))
            throw std::runtime_error("IAudioClient.SetEventHandle error");

        UINT32 NumBufferFrames = 0;
        hr = pAudioClient->GetBufferSize(&NumBufferFrames);
        if (FAILED(hr))
            throw std::runtime_error("IAudioClient.GetBufferSize error");

        std::cout << "buffer frame size=" << NumBufferFrames << "[frames]" << std::endl;

        hr = pAudioClient->GetService(
                __uuidof(IAudioRenderClient),
                (void**)&pAudioRenderClient);
        if (FAILED(hr))
            throw std::runtime_error("IAudioClient.GetService error");

        BYTE* pData = nullptr;
        UINT32 read_count = NumBufferFrames;
        hr = pAudioRenderClient->GetBuffer(read_count, &pData);
        if (FAILED(hr))
            throw std::runtime_error("IAudioRenderClient.GetBuffer error");

        hr = pAudioRenderClient->ReleaseBuffer(read_count, AUDCLNT_BUFFERFLAGS_SILENT);
        if (FAILED(hr))
            throw std::runtime_error("IAudioRenderClient.ReleaseBuffer error");

        hr = pAudioClient->Start();
        if (FAILED(hr))
            throw std::runtime_error("IAudioClient.Start error");

        std::cout << "IAudioClient.Start()->OK" << std::endl;

        while (true)
        {
            /* wait data ...*/
            UINT32 NumPaddingFrames = 0;
            UINT32 NumQueued;
            UINT32 NumFrameSetsToBuffer;

            hr = pAudioClient->GetCurrentPadding(&NumPaddingFrames);
            if (FAILED(hr))
                throw std::runtime_error("IAudioClient.GetCurrentPadding error");

            UINT32 numAvailableFrames = NumBufferFrames - NumPaddingFrames;

            if (numAvailableFrames < FRAMES_PER_BUFFER)
                continue;

            //std::cout << "numAvailableFrames=" << numAvailableFrames << std::endl;

            dwFlags = AUDCLNT_BUFFERFLAGS_SILENT;

            /* Naive mixer start */
            {
                static int last_acnt = -1;
                static int last_tcnt = -1;

                int acnt = 0;
                int tcnt = channelQueue.size();

                vector<AudioChunk*> pAcVec(tcnt, nullptr);
                vector<bool> flow(tcnt, false);

                for (int i = 0; i < tcnt; i++)
                {
                     flow[i] = channelQueue[i]->get(&pAcVec[i]);
                     acnt += flow[i];

                     ALOG_D("[SPEKAER] channel %d, data length : %d, getFlow : %d\n", i, channelQueue[i]->getLen(), channelQueue[i]->getFlow());
                }

                if (last_acnt != acnt || last_tcnt != tcnt)
                {
                    ALOG_I("[SPEAKER] catch %d/%d channel data\n", acnt, tcnt);
                    last_acnt = acnt;
                    last_tcnt = tcnt;
                }

                NumFrameSetsToBuffer = 1;
                read_count = NumFrameSetsToBuffer * FRAMES_PER_BUFFER;
                hr = pAudioRenderClient->GetBuffer(read_count, &pData);
                dwFlags = 0;

                short* bufferData = reinterpret_cast<short*>(pData);
                for (UINT32 i = 0; i < read_count; i++)
                {
                    int sample = 0;

                    for (int j = 0; j < tcnt; j++) 
                    {
                        if (!flow[j])
                            continue;

                        short* audioData = reinterpret_cast<short*>(pAcVec[j]->data);
                        sample += audioData[i];
                    }

                    sample = min(sample, SHRT_MAX);
                    sample = max(sample, SHRT_MIN);
                    bufferData[i] = static_cast<short>(sample);
                }

  //              for (int i = 0; i < tcnt; i++)
                {
 //                   if (flow[i])
//                        delete pAcVec[i];
                }

                /* playback mix-ed sound data */
                hr = pAudioRenderClient->ReleaseBuffer(read_count, dwFlags);
            }
        }

        std::cout << "playing exiting" << std::endl;

        do
        {
            // wait for buffer to be empty
            ////////////////////////////////////

            UINT32 NumPaddingFrames = 0;
            hr = pAudioClient->GetCurrentPadding(&NumPaddingFrames);
            if (FAILED(hr)) throw std::runtime_error("IAudioClient.GetCurrentPadding error");

            if (NumPaddingFrames == 0)
            {
                std::cout << "current buffer padding=0[frames]" << std::endl;
                break;
            }
        } while (true);

        hr = pAudioClient->Stop();
        if (FAILED(hr)) throw std::runtime_error("IAudioClient.Stop error");
        std::cout << "IAudioClient.Stop()->OK" << std::endl;

    }
    catch (exception e)
    {
    }

    if (pDeviceEnumerator)
        pDeviceEnumerator->Release();

    if (pDevice)
        pDevice->Release();

    if (pAudioClient)
        pAudioClient->Release();

    if (pAudioRenderClient)
        pAudioRenderClient->Release();

    CoUninitialize();
}

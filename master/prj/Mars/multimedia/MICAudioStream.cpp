#include <iostream>
#include <windows.h>
#include <dmo.h>
#include <Mmsystem.h>
#include <objbase.h>
#include <mediaobj.h>
#include <uuids.h>
#include <propidl.h>
#include <wmcodecdsp.h>
#include <atlbase.h>
#include <ATLComCli.h>
#include <audioclient.h>
#include <MMDeviceApi.h>
#include <AudioEngineEndPoint.h>
#include <DeviceTopology.h>
#include <propkey.h>
#include <strsafe.h>
#include "MICAudioStream.h"
#include "MICBuffer.h"
#include "AecKsBinder.h"
#include "WaveWriter.h"
#include "SpeakerRenderer.h"
#include "MultimediaManager.h"
#include "OPUSACodec.h"
#include "litevad.h"
#include "../MarsLog.h"

/* windows multimedia class scheduler (MM CSS) */
#pragma comment(lib, "avrt.lib")
/* DMO (DirectX Media Object) */
#pragma comment(lib, "Msdmo.lib")
/* DMO GUIDs */
#pragma comment(lib, "dmoguids.lib")
/* COM UUID */
#pragma comment(lib, "uuid.lib")
/* DirectShow */
#pragma comment(lib, "amstrmid.lib")
/* Windows Media Audio and Video DSP interface */
#pragma comment(lib, "wmcodecdspuuid.lib")

using namespace std;

#define VAD_ENABLED

MICAudioStream::MICAudioStream(AudioStreamSender *ass, ACodec* ac)
    : ANC(true), VAD(true), isSpeaking(false),
    vadCntMax(10), vadCntCur(0), preCnt(30),
    AudioStream(ac), sender(ass)
{
    if (ac != nullptr)
        mAc = ac;

    cout << "[MICAudioStream] Create\n";
}

MICAudioStream::~MICAudioStream()
{
    cout << "[MICAudioStream] destroy\n";
}

void MICAudioStream::run(void)
{
    alive = true;
    t = thread(&MICAudioStream::work, this);
}

void MICAudioStream::stop()
{
    alive = false;
    t.join();
}
bool MICAudioStream::boostPriority(void)
{
    HANDLE hThread = t.native_handle();

    return SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);
}

static int SetBoolProperty(IPropertyStore* ptrPS, REFPROPERTYKEY key, VARIANT_BOOL value)
{
    PROPVARIANT pv;
    PropVariantInit(&pv);
    pv.vt = VT_BOOL;
    pv.boolVal = value;
    HRESULT hr = ptrPS->SetValue(key, pv);
    PropVariantClear(&pv);

    if (FAILED(hr))
    {
        //_TraceCOMError(hr);
        return -1;
    }
    return 0;
}

static int SetVtI4Property(IPropertyStore* ptrPS, REFPROPERTYKEY key, LONG value)
{
    PROPVARIANT pv;
    PropVariantInit(&pv);
    pv.vt = VT_I4;
    pv.lVal = value;
    HRESULT hr = ptrPS->SetValue(key, pv);
    PropVariantClear(&pv);
    if (FAILED(hr))
    {
        //_TraceCOMError(hr);
        return -1;
    }
    return 0;
}

bool MICAudioStream::getVadStatus() 
{
    return mCurrentVAD;
}

void MICAudioStream::sendVADStatus(bool isSpeaking)
{
    if (getVadStatus() != isSpeaking) {
        MARSMultimediaControl.sendVadStatus(isSpeaking);
        mCurrentVAD = isSpeaking;

        cout << "[AUDIO] sendVADStatus is speaking : " << isSpeaking << endl;
    }
}
#define SAFE_RELEASE(p) {if (NULL != p) {(p)->Release(); (p) = NULL;}}
#define CHECK_RET(hr, message) if (FAILED(hr)) { puts(message); goto exit;}
#define CHECKHR(x) hr = x; if (FAILED(hr)) {std::cout << __LINE__<<": "<< std::hex<< hr << std::endl; goto exit;}
#define CHECK_ALLOC(pb, message) if (NULL == pb) { puts(message); goto exit;}

void MICAudioStream::work(void)
{
    HRESULT hr = S_OK;
    IMediaObject* pDMO = NULL;
    IPropertyStore* pPS = NULL;
    DMO_MEDIA_TYPE mt = { 0 };

    MICBuffer micInputBuffer;
    DMO_OUTPUT_DATA_BUFFER MicInputBufferStruct = { 0 };
    MicInputBufferStruct.pBuffer = &micInputBuffer;
    int BytesPerFrame;
    int NumberOfFramestoSend;

    ULONG cbProduced = 0;
    DWORD dwStatus;

    DWORD cMicInputBufLen = 0;
    BYTE* pbMicInputBuffer = NULL;

#ifdef VAD_ENABLED
    litevad_handle_t vad_handle = NULL;
#endif

    // Set DMO output format
    hr = MoInitMediaType(&mt, sizeof(WAVEFORMATEX));
    WAVEFORMATEX* ptrWav = reinterpret_cast<WAVEFORMATEX*>(mt.pbFormat);
    CHECK_RET(hr, "[MICAudioStream] MoInitMediaType failed");

#ifdef VAD_ENABLED
    vad_handle = litevad_create(SAMPLE_RATE, CHANNELS, BITS_PER_SAMPLE);
    if (vad_handle == NULL)
    {
        puts("litevad_create Failed\n");
        goto exit;
    }
#endif

    hr = CoInitialize(nullptr);
    if (FAILED(hr)) throw std::runtime_error("CoInitialize error");

    boostPriority();

    // DMO initialization
    CHECKHR(CoCreateInstance(CLSID_CWMAudioAEC, NULL, CLSCTX_INPROC_SERVER, IID_IMediaObject, (void**)&pDMO));
    CHECKHR(pDMO->QueryInterface(IID_IPropertyStore, (void**)&pPS));

    if (SetVtI4Property(pPS, MFPKEY_WMAAECMA_SYSTEM_MODE, SINGLE_CHANNEL_NSAGC))
    {
        cout << "[MICAudioStream] MFPKEY_WMAAECMA_SYSTEM_MODE failed\n";
        goto exit;
    }

    // Set the AEC source mode.
    // VARIANT_TRUE - Source mode (we poll the AEC for captured data).
    if (SetBoolProperty(pPS, MFPKEY_WMAAECMA_DMO_SOURCE_MODE, VARIANT_TRUE) == -1)
    {
        cout << "[MICAudioStream] AEC soruce mode setting failed\n";
        goto exit;
    }

    // Enable the feature mode.
    // This lets us override all the default processing settings below.
    if (SetBoolProperty(pPS, MFPKEY_WMAAECMA_FEATURE_MODE, VARIANT_TRUE) == -1)
    {
        cout << "[MICAudioStream] feature mode setting failed\n";
        goto exit;
    }

    // Disable analog AGC (default enabled).
    if (SetBoolProperty(pPS, MFPKEY_WMAAECMA_MIC_GAIN_BOUNDER, VARIANT_FALSE) == -1)
    {
        cout << "[MICAudioStream] disable analog AGC setting failed\n";
        goto exit;
    }

    if (ANC)
    {
        // Disable noise suppression (default enabled).
        // 0 - Disabled, 1 - Enabled
        if (SetVtI4Property(pPS, MFPKEY_WMAAECMA_FEATR_NS, 1) == -1)
        {
            cout << "[MICAudioStream] noise suppression disable failed.\n";
            goto exit;
        }
    }
    else
    {
        // Disable noise suppression (default enabled).
        // 0 - Disabled, 1 - Enabled
        if (SetVtI4Property(pPS, MFPKEY_WMAAECMA_FEATR_NS, 0) == -1)
        {
            cout << "[MICAudioStream] noise suppression enable failed.\n";
            goto exit;
        }
    }

    mt.majortype = MEDIATYPE_Audio;
    mt.subtype = MEDIASUBTYPE_PCM;
    mt.lSampleSize = 0;
    mt.bFixedSizeSamples = TRUE;
    mt.bTemporalCompression = FALSE;
    mt.formattype = FORMAT_WaveFormatEx;

    // Supported formats
    // nChannels: 1 (in AEC-only mode)
    // nSamplesPerSec: 8000, 11025, 16000, 22050
    // wBitsPerSample: 16
    ptrWav->wFormatTag = WAVE_FORMAT_PCM;
    ptrWav->nChannels = 1;
    // 16000 is the highest we can support with our resampler.
    ptrWav->nSamplesPerSec = SAMPLE_RATE;
    ptrWav->wBitsPerSample = sizeof(short) * 8;
    ptrWav->nBlockAlign = ptrWav->nChannels * ptrWav->wBitsPerSample / 8;
    ptrWav->nAvgBytesPerSec = ptrWav->nSamplesPerSec * ptrWav->nBlockAlign;
    ptrWav->cbSize = 0;

    ofstream *outFileMic = OutputWaveOpen("out.wav", ptrWav->nChannels, ptrWav->nSamplesPerSec, ptrWav->wBitsPerSample);

    hr = pDMO->SetOutputType(0, &mt, 0);
    CHECK_RET(hr, "[MICAudioStream] SetOutputType failed");

    // Allocate streaming resources. This step is optional. If it is not called here, it
    // will be called when first time ProcessInput() is called. However, if you want to 
    // get the actual frame size being used, it should be called explicitly here.
    hr = pDMO->AllocateStreamingResources();
    CHECK_RET(hr, "[MICAudioStream] AllocateStreamingResources failed");

    // Get actually frame size being used in the DMO. (optional, do as you need)
    int iFrameSize;
    PROPVARIANT pvFrameSize;
    PropVariantInit(&pvFrameSize);
    CHECKHR(pPS->GetValue(MFPKEY_WMAAECMA_FEATR_FRAME_SIZE, &pvFrameSize));
    iFrameSize = pvFrameSize.lVal;
    PropVariantClear(&pvFrameSize);

    BytesPerFrame = iFrameSize * ptrWav->nBlockAlign;
    cout << "[MICAudioStream] Frame Size " << iFrameSize << "\n";

    /* allocate output buffer */
    cMicInputBufLen = ptrWav->nSamplesPerSec * ptrWav->nBlockAlign;
    pbMicInputBuffer = new BYTE[cMicInputBufLen];

    CHECK_ALLOC(pbMicInputBuffer, "[MICAudioStream] out of memory.\n");
    MoFreeMediaType(&mt);

    //AudioChannelQueue* queue = MARSMultimediaManager.getSpeaker()->registerProducer();

    /* main loop to get mic output from the DMO */
    while (alive)
    {
        do {
            micInputBuffer.Init((byte*)pbMicInputBuffer, cMicInputBufLen, 0);
            MicInputBufferStruct.dwStatus = 0;
            hr = pDMO->ProcessOutput(0, 1, &MicInputBufferStruct, &dwStatus);

            CHECK_RET(hr, "[MICAudioStream] ProcessOutput failed");

            if (hr == S_FALSE) {
                cbProduced = 0;
            }
            else
            {
                hr = micInputBuffer.GetBufferAndLength(NULL, &cbProduced);
                CHECK_RET(hr, "[MICAudioStream] GetBufferAndLength failed");
                NumberOfFramestoSend = cbProduced / BytesPerFrame;

                for (int i = 0; i < NumberOfFramestoSend; i++)
                {
                    if (outFileMic)
                        OutputWaveWrite(outFileMic, (const char*)(pbMicInputBuffer + (i * BytesPerFrame)), BytesPerFrame);

#ifdef VAD_ENABLED
                    static litevad_result_t last_vad_state = LITEVAD_RESULT_NOTSET;

                    //**** VAD Start *****//
                    litevad_result_t vad_state = litevad_process(
                        vad_handle,
                        (const int16_t*)(pbMicInputBuffer + (i * BytesPerFrame)),
                        FRAMES_PER_BUFFER);

                    vadCntCur = max(vadCntCur-1, 0);

                    switch (vad_state) {
                    case LITEVAD_RESULT_SPEECH_BEGIN:
                        isSpeaking = true;
                        vadCntCur = vadCntMax;
                        if (last_vad_state != vad_state)
                            ALOG_I("[MICAudioStream] Speech Begin\n");
                        break;
                    case LITEVAD_RESULT_SPEECH_END:
                        vadCntCur--;
                        if (last_vad_state != vad_state)
                            ALOG_I("[MICAudioStream] Speech End\n");
                        break;
                    case LITEVAD_RESULT_SPEECH_BEGIN_AND_END:
                        isSpeaking = true;
                        vadCntCur = vadCntMax;
                        if (last_vad_state != vad_state)
                            ALOG_I("[MICAudioStream] Speech Begin & End\n");
                        break;
                    case LITEVAD_RESULT_FRAME_SILENCE:
                        vadCntCur--;
                        if (last_vad_state != vad_state)
                            ALOG_I("[MICAudioStream] Silence\n");
                        break;
                    case LITEVAD_RESULT_FRAME_ACTIVE:
                        isSpeaking = true;
                        vadCntCur = vadCntMax;
                        if (last_vad_state != vad_state)
                            ALOG_I("[MICAudioStream] === Frame Active ===\n");
                        break;
                    case LITEVAD_RESULT_ERROR:
                        isSpeaking = false;
                        if (last_vad_state != vad_state)
                            ALOG_E("[MICAudioStream] VAD Error\n");
                        break;
                    default:
                        mCurrentVAD = false;
                        if (last_vad_state != vad_state)
                            ALOG_E("[MICAudioStream] VAD State Unknown\n");
                        break;
                    }

                    last_vad_state = vad_state;

                    bool send_flusher = false;

                    if (isSpeaking && vadCntCur <= 0)
                    {
                        isSpeaking = false;
                        send_flusher = true;
                    }

                    // Notify current VAD status to UI.
                    sendVADStatus(isSpeaking);
#endif
                    /* TODO: send to transport manager.. but now.. */

                    {
                        AudioChunk ac;

                        size_t sizeAfterEncode;
                        unsigned char cbits[MAX_PACKET_SIZE];

                        unsigned char* pcm_in;
                        pcm_in = (unsigned char*)(pbMicInputBuffer + (i * BytesPerFrame));
                        mAc->encode((uint8_t*)pcm_in, BytesPerFrame, (uint8_t*)ac.data, &sizeAfterEncode);

                        ALOG_D("[MICAudioStream %s] decode %d -> %d\n",
                                peerIpAddress.c_str(),
                                BytesPerFrame,
                                sizeAfterEncode);

                        ac.NumberOfPayloadByte = sizeAfterEncode;

                        if (isSpeaking)
                        {
                            if (!preChunks.empty())
                                ALOG_I("[MICAudioStream] start to send audio data with pre-store %d chunk\n", preChunks.size());

                            while (!preChunks.empty())
                            {
                                AudioChunk pAc = preChunks.back();
                                preChunks.pop_back();
                                sender->put(pAc);
                            }

                            sender->put(ac);
                        }
                        else
                        {
                            if (preChunks.size() >= preCnt)
                                preChunks.pop_back();

                            preChunks.push_front(ac);
                        }

                        if (send_flusher)
                        {
                            ALOG_I("[MICAudioStream] send flusher\n");
                            init_flusher(&ac);
                            sender->put(ac);
                        }
                    }

                }
            }
        } while (MicInputBufferStruct.dwStatus & DMO_OUTPUT_DATA_BUFFERF_INCOMPLETE);
    }

exit:
#ifdef VAD_ENABLED
    litevad_destroy(vad_handle);
    vad_handle = NULL;
    mCurrentVAD = false;
#endif

#if 0
    SAFE_RELEASE(pDMO);
    SAFE_RELEASE(pPS);
#endif
    if (outFileMic)
    {
        OutputWaveClose(outFileMic);
        delete outFileMic;
        outFileMic = nullptr;
    }

    CoUninitialize();
}

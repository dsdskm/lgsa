#ifndef __CAMERAVIDEOSTREAM_H__
#define __CAMERAVIDEOSTREAM_H__

#include <opencv2\highgui\highgui.hpp>
#include <opencv2\opencv.hpp>
#include "MultimediaStreamSender.h"
#include "VideoStream.h"
#include "Codec.h"

class CameraVideoStream : public VideoStream {
    bool terminate;
    VideoStreamSender *sender;
    cv::VideoCapture *camera;
    virtual void work(void);

public:
    CameraVideoStream(VideoStreamSender *vss, VCodec* c);
    ~CameraVideoStream();
    virtual void run(void);
    virtual void stop(void);
    virtual void QualityUp(void);
    virtual void QualityDown(void);
};

#endif

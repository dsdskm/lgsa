#include <iostream>

#include "CameraVideoStream.h"
#include "MultimediaControl.h"
#include "TimeRing.h"
#include "../MarsLog.h"

using namespace std;
using namespace cv;

CameraVideoStream::CameraVideoStream(VideoStreamSender *vss, VCodec *vc):
    terminate(false), sender(vss), VideoStream(vc)
{
    camera = new VideoCapture();
    if (camera)
    {
        camera->open(0);
        if (!camera->isOpened())
        {
            camera->release();
            cout << "cannot open camera";
            exit(0);
        }
    }

    cout << "[CameraVideoSteram] Create\n";
}

CameraVideoStream::~CameraVideoStream()
{
    cout << "[CameraVideoStream] Destroy\n";
    delete camera;
    delete sender;
}

void CameraVideoStream::work(void)
{
    TimeRing tr;

    this_thread::sleep_for(chrono::milliseconds(500));

    while (!terminate)
    {
        Mat cvframe;
        size_t w, h, bpp;
        size_t ibytes;

        camera->read(cvframe);

        double avgMs = tr.touch();
        if (avgMs != NAN)
            VLOG_D("[CameraVideoStream] fps %llf.2\n", 1000 / avgMs);

        w = cvframe.cols;
        h = cvframe.rows;
        bpp = cvframe.channels();
        ibytes = w * h * bpp;

        /* send media controller */
        MARSMultimediaControl.sendVideoFrame(cvframe.data, ibytes, 0);

        size_t obytes = 640*480*3;
        uint8_t *buffer = new uint8_t [obytes];

        /* encode */
        if (vcodec->encode(cvframe.data, ibytes, buffer, &obytes))
        {
            /* send transport controller */
            sender->put(buffer, obytes);
        }

        delete[] buffer;
    }
}

void CameraVideoStream::run(void)
{
    t = thread(&CameraVideoStream::work, this);
}

void CameraVideoStream::stop(void)
{
    terminate = true;
    VLOG_I("[CameraVideoStream] send stop signal\n");

    t.join();
    VLOG_I("[CameraVideoStream] stopped.\n");
}

void CameraVideoStream::QualityUp(void)
{
    vcodec->increaseQuality();
}

void CameraVideoStream::QualityDown(void)
{
    vcodec->decreaseQuality();
}

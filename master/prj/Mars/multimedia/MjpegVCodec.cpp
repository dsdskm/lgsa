#include "MjpegVCodec.h"
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\opencv.hpp>
#include <vector>
#include "../MarsLog.h"

using namespace std;
using namespace cv;

MjpegVCodec::MjpegVCodec() : Q(50) {}

bool MjpegVCodec::encode(uint8_t* in, size_t isize, uint8_t* out, size_t* osize)
{
    cout << "[MjpegVCodec] encode start\n";

    Mat image(480, 640, CV_8UC3, in);
    vector<int> param = {IMWRITE_JPEG_QUALITY, Q};
    vector<uchar> buffer;
    imencode(".jpg", image, buffer, param);

    if (!buffer.size())
    {
        cout << "[MjpegVCodec] encoding failed.\n";
        return false;
    }

    if (buffer.size() > *osize)
    {
        cout << "[MjpegVCodec] encoding output buffer too small. "
            << buffer.size() << "/" << *osize << "\n";
        return false;
    }

    copy(buffer.begin(), buffer.end(), out);
    *osize = buffer.size();

    cout << "[MjpegVCodec] encode done, " << isize << " -> " << *osize << "\n";

    return true;
}

bool MjpegVCodec::decode(uint8_t* in, size_t isize, uint8_t* out, size_t* osize)
{
    cout << "[MjpegVCodec] decode start\n";

    vector<uchar> buffer(in, in + isize);

    Mat image = imdecode(buffer, 1);
    if (image.empty())
    {
        cout << "[MjpegVCodec] decoding failed.\n";
        return false;
    }

    if (image.total() * image.elemSize() > *osize)
    {
        cout << "[MjpegVCodec] decoding output buffer too small. "
             << image.total() * image.elemSize() 
             << "/" << *osize << "\n";
        return false;
    }

    memcpy(out, image.data, image.total() * image.elemSize());
    *osize = image.total() * image.elemSize();

    cout << "[MjpegVCodec] decode done, " << isize << " -> " << *osize << "\n";

    return true;
}

bool MjpegVCodec::setInputResolution(int w, int h)
{
    return true;
}

bool MjpegVCodec::setOutputResolution(int w, int h)
{
    return true;
}

bool MjpegVCodec::setInputColorSpace(Colorspace cs)
{
    return true;
}

bool MjpegVCodec::increaseQuality(void)
{
    if (Q >= 100)
        return false;

    Q = max(Q+5, 100);
    return true;
}

bool MjpegVCodec::decreaseQuality(void)
{
    if (Q < 15)
        return false;

    Q = min(Q-5, 15);
    return true;
}

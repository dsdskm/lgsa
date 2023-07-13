// CMarsCamTestDlg.cpp : implementation file
//

#include "pch.h"
#include "Mars.h"
#include "afxdialogex.h"
#include "CMarsCamTestDlg.h"
#include "multimedia/MultimediaControl.h"

#include <opencv2\highgui\highgui.hpp>
#include <opencv2\opencv.hpp>
#include <windows.h>


static HDC hdc;
static HWND hWindowMain;
static BITMAPINFO BitmapInfo;
static bool SetupBitMapInfoSet = false;
static RECT rt;


static void SetupBitMapInfo(BITMAPINFO* BitmapInfo, cv::Mat* frame)
{
	int depth = frame->depth();
	int channels = frame->channels();
	int width = frame->cols;
	int height = frame->rows;

	unsigned int pixelSize = (8 << (depth / 2)) * channels; // pixelSize >= 8
	unsigned long bmplineSize = ((width * pixelSize + 31) >> 5) << 2;   // 

	BitmapInfo->bmiHeader.biSize = 40;
	BitmapInfo->bmiHeader.biWidth = width;
	BitmapInfo->bmiHeader.biHeight = height;
	BitmapInfo->bmiHeader.biPlanes = 1;
	BitmapInfo->bmiHeader.biBitCount = pixelSize;
	BitmapInfo->bmiHeader.biCompression = 0;
	BitmapInfo->bmiHeader.biSizeImage = height * bmplineSize;
	BitmapInfo->bmiHeader.biXPelsPerMeter = 0;
	BitmapInfo->bmiHeader.biYPelsPerMeter = 0;
	BitmapInfo->bmiHeader.biClrUsed = 0;
	BitmapInfo->bmiHeader.biClrImportant = 0;
	memset(&BitmapInfo->bmiColors, 0, sizeof(BitmapInfo->bmiColors));
}

// CMarsCamTestDlg dialog

IMPLEMENT_DYNAMIC(CMarsCamTestDlg, CDialogEx)

CMarsCamTestDlg::CMarsCamTestDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_CAM_TEST, pParent)
{
	MARSMultimediaControl.addVideoEventHandler(this);
}

CMarsCamTestDlg::~CMarsCamTestDlg()
{
}

void CMarsCamTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_CAM1, m_picture);
	DDX_Control(pDX, IDC_STATIC_CAM2, m_picture2);
	DDX_Control(pDX, IDC_STATIC_CAM3, m_picture3);
	DDX_Control(pDX, IDC_STATIC_CAM4, m_picture4);
	//m_picture4.DestroyWindow();
	DDX_Control(pDX, IDC_STATIC_BMP, m_bmp);
	//drawDefaultImage(&m_bmp);
}


BEGIN_MESSAGE_MAP(CMarsCamTestDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_START, &CMarsCamTestDlg::OnBnClickedButtonStart)
	ON_WM_TIMER()
	ON_WM_DESTROY(&CMarsCamTestDlg::OnDestroy)
END_MESSAGE_MAP()


// CMarsCamTestDlg message handlers


void CMarsCamTestDlg::drawDefaultImage(CStatic* mCam) {
	CStatic* m_pic;
	CImage* c_img_mfc;
	cv::Mat image = cv::imread(".\\res\\Call.bmp", cv::IMREAD_COLOR);
	m_pic = &m_bmp;
	c_img_mfc = &cimage_mfc;

	RECT r;
	m_pic->GetClientRect(&r);

	/* align image (opencv constraint) */
	r.right = r.right - r.right % 4;
	r.bottom = r.bottom - r.bottom % 4;

	cv::Size winSize(r.right, r.bottom);

	Mat mat_temp;
	cv::resize(image, mat_temp, winSize);

	c_img_mfc->Create(winSize.width, winSize.height, 24);

	BITMAPINFO* bitInfo = (BITMAPINFO*)malloc(sizeof(BITMAPINFOHEADER));
	bitInfo->bmiHeader.biBitCount = 24;
	bitInfo->bmiHeader.biWidth = mat_temp.cols;
	bitInfo->bmiHeader.biHeight = -mat_temp.rows;
	bitInfo->bmiHeader.biPlanes = 1;
	bitInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitInfo->bmiHeader.biCompression = BI_RGB;
	bitInfo->bmiHeader.biClrImportant = 0;
	bitInfo->bmiHeader.biClrUsed = 0;
	bitInfo->bmiHeader.biSizeImage = 0;
	bitInfo->bmiHeader.biXPelsPerMeter = 0;
	bitInfo->bmiHeader.biYPelsPerMeter = 0;

	// source and destination have same size
	// transfer memory block
	// NOTE: the padding border will be shown here. Anyway it will be max 3px width
	SetDIBitsToDevice(c_img_mfc->GetDC(),
		//destination rectangle
		0, 0, winSize.width, winSize.height,
		0, 0, 0, mat_temp.rows,
		mat_temp.data, bitInfo, DIB_RGB_COLORS);

	HDC dc = ::GetDC(m_pic->m_hWnd);
	c_img_mfc->BitBlt(dc, 0, 0);

	::ReleaseDC(m_pic->m_hWnd, dc);

	c_img_mfc->ReleaseDC();
	c_img_mfc->Destroy();
}

void CMarsCamTestDlg::OnDestroy()
{
    MARSMultimediaControl.delVideoEventHandler();
    CDialog::OnDestroy();
}

void CMarsCamTestDlg::OnBnClickedButtonStart()
{
	// TODO: Add your control notification handler code here

	/*
	capture = new VideoCapture(0);

	if (!capture->isOpened())
	{
		MessageBox(_T("cannot open camera \n"));
	}

	// webcam size 320x240    
	capture->set(CAP_PROP_FRAME_WIDTH, 320);
	capture->set(CAP_PROP_FRAME_HEIGHT, 240);

	SetTimer(1000, 30, NULL);
	*/
}


void CMarsCamTestDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default

	CDialogEx::OnTimer(nIDEvent);
}


bool CMarsCamTestDlg::onFrameArrive(uint8_t* frame, size_t size, int peerid) {
    Mat mat(480, 640, CV_8UC3, const_cast<uint8_t*>(frame));
	//drawDefaultImage(&m_bmp);

	CStatic* m_pic;
	CImage* c_img_mfc;
	switch (peerid) {
	case 0:
		m_pic = &m_picture;
		c_img_mfc = &cimage_mfc;
		break;
	case 1:
		m_pic = &m_picture2;
		c_img_mfc = &cimage_mfc2;
		break;
	case 2:
		m_pic = &m_picture3;
		c_img_mfc = &cimage_mfc3;
		break;
	case 3:
		m_pic = &m_picture4;
		c_img_mfc = &cimage_mfc4;
		break;
	default:
		m_pic = &m_picture;
		c_img_mfc = &cimage_mfc;
	}

//    if (peerid != 0)
//        return true;

    RECT r;
    m_pic->GetClientRect(&r);

    /* align image (opencv constraint) */
    r.right = r.right - r.right%4;
    r.bottom = r.bottom - r.bottom%4;

    cv::Size winSize(r.right, r.bottom);

    Mat mat_temp;
    cv::resize(mat, mat_temp, winSize);

	c_img_mfc->Create(winSize.width, winSize.height, 24);

    BITMAPINFO* bitInfo = (BITMAPINFO*)malloc(sizeof(BITMAPINFOHEADER));
    bitInfo->bmiHeader.biBitCount = 24;
    bitInfo->bmiHeader.biWidth = mat_temp.cols;
    bitInfo->bmiHeader.biHeight = -mat_temp.rows;
    bitInfo->bmiHeader.biPlanes = 1;
    bitInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitInfo->bmiHeader.biCompression = BI_RGB;
    bitInfo->bmiHeader.biClrImportant = 0;
    bitInfo->bmiHeader.biClrUsed = 0;
    bitInfo->bmiHeader.biSizeImage = 0;
    bitInfo->bmiHeader.biXPelsPerMeter = 0;
    bitInfo->bmiHeader.biYPelsPerMeter = 0;

    // source and destination have same size
    // transfer memory block
    // NOTE: the padding border will be shown here. Anyway it will be max 3px width
    SetDIBitsToDevice(c_img_mfc->GetDC(),
            //destination rectangle
            0, 0, winSize.width, winSize.height,
            0, 0, 0, mat_temp.rows,
            mat_temp.data, bitInfo, DIB_RGB_COLORS);

    HDC dc = ::GetDC(m_pic->m_hWnd);
	c_img_mfc->BitBlt(dc, 0, 0);

    ::ReleaseDC(m_pic->m_hWnd, dc);

	c_img_mfc->ReleaseDC();
	c_img_mfc->Destroy();

    return true;
}

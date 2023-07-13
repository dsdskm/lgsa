#pragma once
#include "afxdialogex.h"
#include "opencv2/opencv.hpp" 

#include "multimedia/VideoEventHandler.h"

using namespace cv;

// CMarsCamTestDlg dialog

class CMarsCamTestDlg : public CDialogEx, public VideoEventHandler
{
	DECLARE_DYNAMIC(CMarsCamTestDlg)

public:
	CMarsCamTestDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CMarsCamTestDlg();
	bool onFrameArrive(uint8_t* frame, size_t size, int peerid);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CAM_TEST };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	CStatic m_picture;

	Mat mat_frame;
	CImage cimage_mfc;
	CImage cimage_mfc2;
	CImage cimage_mfc3;
	CImage cimage_mfc4;
	VideoCapture* capture;
	CStatic m_picture2;
	CStatic m_picture3;
	CStatic m_picture4;
	CStatic m_bmp;
	void drawDefaultImage(CStatic* mCam);
};
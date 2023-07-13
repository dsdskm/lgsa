#pragma once
#include "afxdialogex.h"
#include "CUserInfo.h"
#include "CConferenceInfo.h"
#include "CParticipant.h"
#include "opencv2/opencv.hpp" 
#include "multimedia/VideoEventHandler.h"
#include "net/SessionController.h"
#include "multimedia/MultimediaManager.h"

using namespace cv;

// CMarsMeetingDlg dialog

class CMarsMeetingDlg : public CDialogEx, public VideoEventHandler, public net::SessionEventHandler
{
	DECLARE_DYNAMIC(CMarsMeetingDlg)
	
public:
	CMarsMeetingDlg(CUserInfo& userInfo, string conferenceId, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CMarsMeetingDlg();

	bool onFrameArrive(uint8_t* frame, size_t size, int peerid);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_MEETING };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CStatic m_cam[4];
	CStatic m_camName[4];
	CListBox m_active;
	CListBox m_inactive;
	CStatic m_topic;
	CButton m_next;

	CUserInfo& userInfo;
	string conferenceId;

	map<string, CParticipant> mapParticipant;
	deque<string> activeUserIdQueue;
	deque<string> inactiveUserIdQueue;
	map<int, pair<string, string>> mIdxIdIp;



	void initUI();
	void updateUI();

	void initMapParticipant();

	void drawDefaultImage(CStatic* mCam);

	void OnCallReservation(const string& call_id, list<string> participant_id_list) override;
	void OnCallRequest(const string& call_id) override;
	void OnCallJoined(const string& call_id, const string& joined_uid) override;
	void OnCallLeft(const string& call_id, const string& left_uid) override;
	void OnCallReject(const string& call_id, const string& rejected_uid) override;
	void OnUserUpdate(const string& updated_uid) override;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnBnClickedButtonEnd();
	afx_msg void OnBnClickedButtonNext();
	afx_msg void OnDestroy();
};

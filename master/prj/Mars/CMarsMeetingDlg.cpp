// CMarsMeetingDlg.cpp : implementation file
//

#include "pch.h"
#include "Mars.h"
#include "afxdialogex.h"
#include "CMarsMeetingDlg.h"
#include "CUserInfo.h"
#include "CMarsFunctions.h"
#include "CConferenceInfo.h"
#include "CommonUtils.h"
#include "CParticipant.h"
#include "multimedia/MultimediaControl.h"
#include "net/SessionController.h"

#include <opencv2\highgui\highgui.hpp>
#include <opencv2\opencv.hpp>
#include <windows.h>


int getIndexFromDeque(const std::deque<string>& myDeque, string value) {
	int index = 0;
	for (const auto& element : myDeque) {
		if (element.compare(value) == 0) {
			return index;
		}
		index++;
	}
	return -1;  // Value not found
}

void setupBitmapInfo(BITMAPINFO* bitInfo, cv::Mat* mat) {
	bitInfo->bmiHeader.biBitCount = 24;
	bitInfo->bmiHeader.biWidth = mat->cols;
	bitInfo->bmiHeader.biHeight = -mat->rows;
	bitInfo->bmiHeader.biPlanes = 1;
	bitInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitInfo->bmiHeader.biCompression = BI_RGB;
	bitInfo->bmiHeader.biClrImportant = 0;
	bitInfo->bmiHeader.biClrUsed = 0;
	bitInfo->bmiHeader.biSizeImage = 0;
	bitInfo->bmiHeader.biXPelsPerMeter = 0;
	bitInfo->bmiHeader.biYPelsPerMeter = 0;
}


void drawDefaultFrame(CStatic* mCam) {
	cv::Mat mat = cv::imread(".\\res\\Call.bmp", cv::IMREAD_COLOR);

	RECT r;
	mCam->GetClientRect(&r);

	/* align image (opencv constraint) */
	r.right = r.right - r.right % 4;
	r.bottom = r.bottom - r.bottom % 4;
	cv::Size winSize(r.right, r.bottom);

	cv::Mat matResized;
	cv::resize(mat, matResized, winSize);

	CImage cImage;
	cImage.Create(winSize.width, winSize.height, 24);

	BITMAPINFO* bitInfo = (BITMAPINFO*)malloc(sizeof(BITMAPINFOHEADER));
	setupBitmapInfo(bitInfo, &matResized);

	SetDIBitsToDevice(cImage.GetDC(),
		//destination rectangle
		0, 0, winSize.width, winSize.height,
		0, 0, 0, matResized.rows,
		matResized.data, bitInfo, DIB_RGB_COLORS);

	HDC dc = ::GetDC(mCam->m_hWnd);
	cImage.BitBlt(dc, 0, 0);

	::ReleaseDC(mCam->m_hWnd, dc);

	cImage.ReleaseDC();
	cImage.Destroy();
}

void drawFrame(CStatic* mCam, uint8_t* frame) {
	RECT r;
	mCam->GetClientRect(&r);

	/* align image (opencv constraint) */
	r.right = r.right - r.right % 4;
	r.bottom = r.bottom - r.bottom % 4;
	cv::Size winSize(r.right, r.bottom);

	cv::Mat mat(480, 640, CV_8UC3, const_cast<uint8_t*>(frame));
	cv::Mat matResized;
	cv::resize(mat, matResized, winSize);

	CImage cImage;
	cImage.Create(winSize.width, winSize.height, 24);

	BITMAPINFO* bitInfo = (BITMAPINFO*)malloc(sizeof(BITMAPINFOHEADER));
	setupBitmapInfo(bitInfo, &matResized);

	SetDIBitsToDevice(cImage.GetDC(),
		//destination rectangle
		0, 0, winSize.width, winSize.height,
		0, 0, 0, matResized.rows,
		matResized.data, bitInfo, DIB_RGB_COLORS);

	HDC dc = ::GetDC(mCam->m_hWnd);
	cImage.BitBlt(dc, 0, 0);

	::ReleaseDC(mCam->m_hWnd, dc);

	cImage.ReleaseDC();
	cImage.Destroy();
}

// CMarsMeetingDlg dialog

IMPLEMENT_DYNAMIC(CMarsMeetingDlg, CDialogEx)

CMarsMeetingDlg::CMarsMeetingDlg(CUserInfo& userInfo, string conferenceId, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_MEETING, pParent),
	userInfo(userInfo),
	conferenceId(conferenceId)
{
	cout << "####### CMarsMeetingDlg::CMarsMeetingDlg()" << endl;
}

CMarsMeetingDlg::~CMarsMeetingDlg()
{
	cout << "####### CMarsMeetingDlg::~CMarsMeetingDlg()" << endl;
}

void CMarsMeetingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_CAM1, m_cam[0]);
	DDX_Control(pDX, IDC_STATIC_CAM2, m_cam[1]);
	DDX_Control(pDX, IDC_STATIC_CAM3, m_cam[2]);
	DDX_Control(pDX, IDC_STATIC_CAM4, m_cam[3]);
	DDX_Control(pDX, IDC_STATIC_CAM_NAME1, m_camName[0]);
	DDX_Control(pDX, IDC_STATIC_CAM_NAME2, m_camName[1]);
	DDX_Control(pDX, IDC_STATIC_CAM_NAME3, m_camName[2]);
	DDX_Control(pDX, IDC_STATIC_CAM_NAME4, m_camName[3]);
	DDX_Control(pDX, IDC_LIST_ACTIVE, m_active);
	DDX_Control(pDX, IDC_LIST_INACTIVE, m_inactive);
	DDX_Control(pDX, IDC_STATIC_TOPIC, m_topic);
	DDX_Control(pDX, IDC_BUTTON_NEXT, m_next);

	cout << "####### CMarsMeetingDlg::DoDataExchange()" << endl;

}

void CMarsMeetingDlg::initMapParticipant() {
	int i = 0;
	mapParticipant.clear();
	mIdxIdIp.clear();
	mapParticipant[userInfo.getId()] = CParticipant(i++, userInfo.getId(), userInfo.getEmail(), userInfo.getFirstName(), userInfo.getLastName(), userInfo.getIpAddress(), true);

	CMarsFunctions mMars;
	json res = mMars.getConference(conferenceId);

	for (auto x : res["participant"]) {
		CUserInfo mUserInfo = mMars.getUserInfo(x);
		if (userInfo.getId() != mUserInfo.getId()) {
			mapParticipant[mUserInfo.getId()] = CParticipant(i, mUserInfo.getId(), mUserInfo.getEmail(),
				mUserInfo.getFirstName(), mUserInfo.getLastName(), mUserInfo.getIpAddress(), false);
			mIdxIdIp[i++] = make_pair(mUserInfo.getId(), mUserInfo.getIpAddress());
		}
	}

	for (auto x : res["joinUsers"]) {
		mapParticipant[x].setActivate(true);
	}
}

void CMarsMeetingDlg::initUI() {
	m_active.ResetContent();
	m_inactive.ResetContent();

	for (auto x : mapParticipant) {
		if (x.second.getActivateState() == true) {
			m_active.AddString(stringToCStringW(x.second.getFullName()));
		} else {
			m_inactive.AddString(stringToCStringW(x.second.getFullName()));
		}
		int idx = x.second.getIdx();
		if (idx < 4) {
			m_camName[idx].SetWindowTextW(stringToCStringW(x.second.getFullName()));
		}
	}

	for (int i = 0; i < 4; i++) {
		if (i < mapParticipant.size()) {
			m_cam[i].ShowWindow(SW_SHOW);
			m_camName[i].ShowWindow(SW_SHOW);
		}
		else {
			m_cam[i].ShowWindow(SW_HIDE);
			m_camName[i].ShowWindow(SW_HIDE);
		}
	}
//	updateUI();
}

BEGIN_MESSAGE_MAP(CMarsMeetingDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_END, &CMarsMeetingDlg::OnBnClickedButtonEnd)
	ON_BN_CLICKED(IDC_BUTTON_NEXT, &CMarsMeetingDlg::OnBnClickedButtonNext)
	ON_WM_DESTROY(&CMarsMeetingDlg::OnDestroy)
	ON_WM_CREATE()
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CMarsMeetingDlg message handlers
//
void CMarsMeetingDlg::OnDestroy()
{
	cout << "####### CMarsMeetingDlg::OnDestroy()" << endl;

	MARSMultimediaManager.conferenceDeleted(mIdxIdIp);
	MARSMultimediaControl.delVideoEventHandler();
	SessionController::Get()->RemoveHandler(this);

	CMarsFunctions mMars;
	mMars.updateJoinConference(userInfo.getId(), conferenceId, false);

	CDialog::OnDestroy();
}

void CMarsMeetingDlg::OnBnClickedButtonEnd()
{
	// TODO: Add your control notification handler code here
	OnOK();
}


bool CMarsMeetingDlg::onFrameArrive(uint8_t* frame, size_t size, int peerIdx) {
	CStatic* mCam;

	mCam = m_cam + peerIdx;
	drawFrame(mCam, frame);

	return true;
}


void CMarsMeetingDlg::OnCallReservation(const string& call_id, list<string> participant_id_list) {
	cout << "####### CMarsMeetingDlg::OnCallReservation()" << endl;

}

void CMarsMeetingDlg::OnCallRequest(const string& call_id) {
	cout << "####### CMarsMeetingDlg::OnCallRequest()" << endl;

}


void CMarsMeetingDlg::updateUI() {
	for (auto x : mapParticipant) {
		if (x.second.getActivateState() == true) {
			m_active.AddString(stringToCStringW(x.second.getFullName()));
		}
		else {
			m_inactive.AddString(stringToCStringW(x.second.getFullName()));
		}
		int idx = x.second.getIdx();
		if (idx < 4) {
			m_camName[idx].SetWindowTextW(stringToCStringW(x.second.getFullName()));
		}
	}

	int numOfParticipants = mapParticipant.size();

	cout << "#################" << numOfParticipants << endl;

	for (int i = numOfParticipants; i < 4; i++) {
		m_cam[i].ShowWindow(SW_HIDE);
		m_camName[i].ShowWindow(SW_HIDE);
	}
}

void CMarsMeetingDlg::OnCallJoined(const string& call_id, const string& joined_uid) {
	//	userInfo.update();
	//
	cout << "###### CMarsMeetingDlg::OnCallJoined()" << endl;
	mapParticipant[joined_uid].setActivate(true);

	// update UI
	CStringW fullName = stringToCStringW(mapParticipant[joined_uid].getFullName());
	int index = m_inactive.FindStringExact(-1, fullName);
	if (index != LB_ERR) {
		m_inactive.DeleteString(index);
		m_active.AddString(fullName);
	}
}

void CMarsMeetingDlg::OnCallLeft(const string& call_id, const string& left_uid) {
	//	userInfo.update();
	cout << "####### CMarsMeetingDlg::OnCallLeft()" << endl;

	mapParticipant[left_uid].setActivate(false);

	// update UI
	CStringW fullName = stringToCStringW(mapParticipant[left_uid].getFullName());
	int index = m_active.FindStringExact(-1, fullName);
	if (index != LB_ERR) {
		m_active.DeleteString(index);
		m_inactive.AddString(fullName);
	}
}

void CMarsMeetingDlg::OnCallReject(const string& call_id, const string& rejected_uid) {
	//userInfo.update();
	if (conferenceId != call_id) {
		return;
	}

	// update UI
	CStringW fullName = stringToCStringW(mapParticipant[rejected_uid].getFullName());
	int index = m_inactive.FindStringExact(-1, fullName);
	if (index != LB_ERR) {
		m_inactive.DeleteString(index);
	}

	mapParticipant.erase(rejected_uid);

	if (mapParticipant.size() == 1) {
		AfxMessageBox(_T("All participants rejected"));
		OnOK();
	}
}

// detect lost connection
void CMarsMeetingDlg::OnUserUpdate(const string& updated_uid) {
	cout << "####### CMarsMeetingDlg::OnUserUpdate()" << endl;

	//auto it = find(activeUserIdQueue.begin(), activeUserIdQueue.end(), updated_uid);
	//if (it != activeUserIdQueue.end()) {
	//	CStringW fullName = stringToCStringW(mapParticipant.find(updated_uid)->second.getFullName());
	//	
	//	AfxMessageBox(fullName + _T("'s connection was lost due to a bad network."));

	//	updateUserIdQueue("left", updated_uid);
	//	updateUI();
	//	notifyUserListToMM("left", updated_uid);

	//	int index = m_active.FindStringExact(-1, fullName);
	//	if (index != LB_ERR) {
	//		m_active.DeleteString(index);
	//		m_inactive.AddString(fullName);
	//	}
	//}
}

void CMarsMeetingDlg::OnBnClickedButtonNext()
{
	// TODO: Add your control notification handler code here
	if (activeUserIdQueue.size() > 4) {
		rotate(activeUserIdQueue.begin() + 1, activeUserIdQueue.begin() + 4, activeUserIdQueue.end());
	}
}


int CMarsMeetingDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;

	cout << "####### OnCreate()" << endl;

	return 0;
}


void CMarsMeetingDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	__super::OnShowWindow(bShow, nStatus);
	cout << "####### CMarsMeetingDlg::OnShowWindow()" << endl;

	// TODO: Add your message handler code here

	CMarsFunctions mMars;
	json res = mMars.getConference(conferenceId);
	m_topic.SetWindowTextW(stringToCStringW(res["topic"]));

	mMars.updateJoinConference(userInfo.getId(), conferenceId, true);
	userInfo.update();

	initMapParticipant();
	initUI();

	MARSMultimediaControl.addVideoEventHandler(this);
	SessionController::Get()->AddHandler(this);

	MARSMultimediaManager.conferenceCreated(mIdxIdIp);
}

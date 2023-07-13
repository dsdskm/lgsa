// CMarsMainDlg.cpp : implementation file
//

#include "pch.h"
#include "Mars.h"
#include "afxdialogex.h"
#include "CMarsMainDlg.h"
#include "CMarsSettingsDlg.h"
#include "CMarsContactDlg.h"
#include "CMarsCamTestDlg.h"
#include "CMarsFunctions.h"
#include "CommonUtils.h"
#include "CMarsConferenceDlg.h"
#include "CMarsMeetingDlg.h"
#include "CMarsIncomingCallDlg.h"

// CMarsMainDlg dialog

IMPLEMENT_DYNAMIC(CMarsMainDlg, CDialogEx)

CMarsMainDlg::CMarsMainDlg(CUserInfo userInfo, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_MAIN, pParent),
	  userInfo(userInfo)
{
}

CMarsMainDlg::~CMarsMainDlg()
{
	SessionController::Get()->RemoveHandler(this);
}

void CMarsMainDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_WELCOME, m_welcome);
	DDX_Control(pDX, IDC_LIST_CONTACT, m_contactList);
	DDX_Control(pDX, IDC_LIST_CONFERENCE, m_conferenceList);
	DDX_Control(pDX, IDC_BUTTON_JOIN, m_join);
	userInfo.update();
}


BEGIN_MESSAGE_MAP(CMarsMainDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_SETTINGS, &CMarsMainDlg::OnBnClickedButtonSettings)
	ON_BN_CLICKED(IDC_BUTTON_ADD_CONFERENCE, &CMarsMainDlg::OnBnClickedButtonAddConference)
	ON_BN_CLICKED(IDC_BUTTON_JOIN, &CMarsMainDlg::OnBnClickedButtonJoin)
	ON_BN_CLICKED(IDC_BUTTON_ADD_CONTACT, &CMarsMainDlg::OnBnClickedButtonAddContact)
	ON_BN_CLICKED(IDC_BUTTON_CALL, &CMarsMainDlg::OnBnClickedButtonCall)
	ON_WM_ACTIVATE()
	ON_LBN_SELCHANGE(IDC_LIST_CONTACT, &CMarsMainDlg::OnLbnSelchangeListContact)
	ON_LBN_SELCHANGE(IDC_LIST_CONFERENCE, &CMarsMainDlg::OnLbnSelchangeListConference)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CMarsMainDlg message handlers


void CMarsMainDlg::OnBnClickedButtonSettings()
{
	// TODO: Add your control notification handler code here
	CMarsSettingsDlg dlgMarsSettings(userInfo);
	dlgMarsSettings.DoModal();
}

list<string> CMarsMainDlg::getSelectedContactList() {
	int selectedCount = m_contactList.GetSelCount();
	CArray<int, int> selectedIndices;
	selectedIndices.SetSize(selectedCount);

	m_contactList.GetSelItems(selectedCount, selectedIndices.GetData());

	list<string> participantList;

	participantList.push_back(userInfo.getId());

	for (int i = 0; i < selectedCount; i++) {
		int selectedIndex = selectedIndices[i];
		CString selectedItemText;
		m_contactList.GetText(selectedIndex, selectedItemText);

		CString c_fullName, c_id;
		int currentPosition = 0;
		c_fullName = selectedItemText.Tokenize(_T("/"), currentPosition);
		c_id = selectedItemText.Tokenize(_T(" "), currentPosition);
		participantList.push_back(string(CT2CA(c_id)));
	}
	return participantList;
}


void CMarsMainDlg::OnBnClickedButtonAddConference()
{
	// TODO: Add your control notification handler code here
	CMarsConferenceDlg dlgMarsConference(userInfo, this->getSelectedContactList());
	dlgMarsConference.DoModal();
}


void CMarsMainDlg::OnBnClickedButtonJoin()
{
	// TODO: Add your control notification handler code here
	int selectedIndex = m_conferenceList.GetCurSel();

	if (selectedIndex != LB_ERR) {
		m_join.EnableWindow(TRUE);
	}
	else {
		m_join.EnableWindow(FALSE);
	}

	CString selectedItemText;
	m_conferenceList.GetText(selectedIndex, selectedItemText.GetBuffer(m_conferenceList.GetTextLen(selectedIndex) + 1));
	selectedItemText.ReleaseBuffer();

	CString conferenceId;
	CString seperator = _T("|");
	int currentPosition = 0;
	conferenceId = selectedItemText.Tokenize(seperator, currentPosition);
	conferenceId = selectedItemText.Tokenize(seperator, currentPosition);
	conferenceId.Replace(_T(" "), NULL);

	CMarsMeetingDlg dlgMarsMeeting(userInfo, string(CT2CA(conferenceId)));
	dlgMarsMeeting.DoModal();
}


void CMarsMainDlg::OnBnClickedButtonAddContact()
{
	// TODO: Add your control notification handler code here
	CMarsContactDlg dlgMarsContact(userInfo);
	dlgMarsContact.DoModal();
}


void CMarsMainDlg::OnBnClickedButtonCall()
{
	// TODO: Add your control notification handler code here
	string currentTime = string(CT2CA(CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:00")));

	list<string> calleeList = getSelectedContactList();
	list<string> busyList;

	CMarsFunctions mMars;
	string conferenceId = mMars.createConference("Immediate Call", currentTime, calleeList, userInfo.getId());
	userInfo.update();

	for (auto x : calleeList) {
		CUserInfo callee = mMars.getUserInfo(x);
		if (callee.getStatus() == "busy") {
			busyList.splice(busyList.end(), calleeList, find(calleeList.begin(), calleeList.end(), x));
		}
	}

	mMars.requestCall(conferenceId, userInfo.getId());

	if (calleeList.empty()) {
		AfxMessageBox(_T("All participant's lines are busy."));
	} else {
		if (!busyList.empty()) {
			string msg;
			for (auto element : busyList) {
				msg += element + " ";
			}
			AfxMessageBox(stringToCStringW(msg) + _T(" lines are busy."));
		}
		CMarsMeetingDlg dlgMarsMeeting(userInfo, conferenceId);
		dlgMarsMeeting.DoModal();
	}
}

void CMarsMainDlg::updateUI() {
	userInfo.update();
	string currentTime = string(CT2CA(CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:00")));
	m_conferenceList.ResetContent();
	list<CConferenceInfo> tmpConferenceList = userInfo.getConferenceList();
	for (auto it = tmpConferenceList.begin(); it != tmpConferenceList.end(); it++) {
		if (it->getStart() >= currentTime) {
			m_conferenceList.AddString(stringToCStringW(it->getStart() + "  |  " + it->getId() + "  |  " + it->getTopic()));
		}
	}

	m_contactList.ResetContent();
	list<CUserInfo> tmpBuddy = userInfo.getBuddy();
	for (auto it = tmpBuddy.begin(); it != tmpBuddy.end(); it++) {
		m_contactList.AddString(stringToCStringW(it->getFirstName() + " " + it->getLastName() + " / " + it->getId()));
	}
}

void CMarsMainDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CDialogEx::OnActivate(nState, pWndOther, bMinimized);

	cout << "####### CMarsMainDlg::OnActivate()" << endl;


	// TODO: Add your message handler code here
	string tmp = "Hi, " + userInfo.getFirstName() + " " + userInfo.getLastName() + "! (" + userInfo.getIpAddress() + ")";
	m_welcome.SetWindowTextW(stringToCStringW(tmp));
	updateUI();
}


void CMarsMainDlg::OnLbnSelchangeListContact()
{
	// TODO: Add your control notification handler code here
	if (m_contactList.GetSelCount() > 0) {
		GetDlgItem(IDC_BUTTON_ADD_CONFERENCE)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_CALL)->EnableWindow(TRUE);
	}
	else {
		GetDlgItem(IDC_BUTTON_ADD_CONFERENCE)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_CALL)->EnableWindow(FALSE);
	}
}


void CMarsMainDlg::OnLbnSelchangeListConference()
{
	// TODO: Add your control notification handler code here
	int selectedIndex = m_conferenceList.GetCurSel();

	if (selectedIndex != LB_ERR) {
		m_join.EnableWindow(TRUE);
	}
	else {
		m_join.EnableWindow(FALSE);
	}
}


void CMarsMainDlg::OnCallReservation(const string& call_id, list<string> participant_id_list) {
//	updateUI();
	cout << "####### CMarsMainDlg::OnCallReservation()" << endl;

}


void CMarsMainDlg::OnCallRequest(const string& call_id) {
	cout << "####### CMarsMainDlg::OnCallRequest()" << endl;

	CMarsFunctions mMars;
	json res = mMars.getConference(call_id);

	if (res["creator"] == userInfo.getId()) {
		return;
	}

	for (auto x : res["participant"]) {
		if (x == userInfo.getId()) {
			if (userInfo.getStatus() == "busy") {
				AfxMessageBox(_T("missed call from ") + stringToCStringW(res["creator"]));
			} else {
				CMarsIncomingCallDlg dlgMarsIncomingCall(userInfo, call_id);
				dlgMarsIncomingCall.DoModal();
			}
		}
	}
}

void CMarsMainDlg::OnCallJoined(const string& call_id, const string& joined_uid) {
	cout << "####### CMarsMainDlg::OnCallJoined()" << endl;

}

void CMarsMainDlg::OnCallLeft(const string& call_id, const string& left_uid) {
	cout << "####### CMarsMainDlg::OnCallLeft()" << endl;

}

void CMarsMainDlg::OnCallReject(const string& call_id, const string& rejected_uid) {
	cout << "####### CMarsMainDlg::OnCallReject()" << endl;

}



void CMarsMainDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	__super::OnShowWindow(bShow, nStatus);
	cout << "####### CMarsMainDlg::OnShowWindow()" << endl;

	// TODO: Add your message handler code here

	SessionController::Get()->AddHandler(this);
}

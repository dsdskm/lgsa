// CMarsConferenceDlg.cpp : implementation file
//

#include "pch.h"
#include "Mars.h"
#include "afxdialogex.h"
#include "CMarsConferenceDlg.h"
#include "CommonUtils.h"
#include <fmt/format.h>
#include <algorithm>
#include "CMarsFunctions.h"


// CMarsConferenceDlg dialog

IMPLEMENT_DYNAMIC(CMarsConferenceDlg, CDialogEx)

CMarsConferenceDlg::CMarsConferenceDlg(CUserInfo& userInfo, list<string> participantList, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_CONFERENCE, pParent),
	userInfo(userInfo),
	participantList(participantList)
{

}

CMarsConferenceDlg::~CMarsConferenceDlg()
{
}

void CMarsConferenceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_TITLE, m_topic);
	DDX_Control(pDX, IDC_DATETIMEPICKER_DATE, m_date);
	DDX_Control(pDX, IDC_DATETIMEPICKER_TIME, m_time);
	DDX_Control(pDX, IDC_STATIC_PARTICIPANTS, m_participants);

	m_date.SetFormat(_T("yyyy/MM/dd"));
	m_time.SetFormat(_T("HH:mm"));
	
	string str = fmt::format("{}", fmt::join(participantList, ", "));
	m_participants.SetWindowTextW(stringToCStringW(str));
}


BEGIN_MESSAGE_MAP(CMarsConferenceDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CMarsConferenceDlg::OnBnClickedButtonSave)
END_MESSAGE_MAP()


// CMarsConferenceDlg message handlers


void CMarsConferenceDlg::OnBnClickedButtonSave()
{
	// TODO: Add your control notification handler code here
	CString s_topic;
	m_topic.GetWindowTextW(s_topic);
	string topic = string(CT2CA(s_topic));

	CTime selectedDate, selectedTime;
	m_date.GetTime(selectedDate);
	m_time.GetTime(selectedTime);

	CString cs_date = selectedDate.Format("%Y-%m-%d");
	CString cs_time = selectedTime.Format("%H:%M:00");
//	AfxMessageBox(cs_date + L" " + cs_time);

	string start_time = string(CT2CA(cs_date + L" " + cs_time));

	CMarsFunctions mMars;
	mMars.createConference(topic, start_time, participantList, userInfo.getId());
	AfxMessageBox(_T("Successfully scheduled"));

	userInfo.update();
	OnOK();
}

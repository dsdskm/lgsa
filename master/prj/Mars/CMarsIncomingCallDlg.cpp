// CMarsIncomingCallDlg.cpp : implementation file
//

#include "pch.h"
#include "Mars.h"
#include "afxdialogex.h"
#include "CMarsIncomingCallDlg.h"
#include "CMarsFunctions.h"
#include "CommonUtils.h"
#include "CMarsMeetingDlg.h"


// CMarsIncomingCallDlg dialog

IMPLEMENT_DYNAMIC(CMarsIncomingCallDlg, CDialogEx)

CMarsIncomingCallDlg::CMarsIncomingCallDlg(CUserInfo& userInfo, string conferenceId, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_INCOMING_CALL, pParent),
	userInfo(userInfo),
	conferenceId(conferenceId)
{

}

CMarsIncomingCallDlg::~CMarsIncomingCallDlg()
{
}

void CMarsIncomingCallDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PARTICIPANTS, m_listParticipants);
	DDX_Control(pDX, IDC_STATIC_CALLER_NAME, m_callerName);

	CMarsFunctions mMars;
	json res = mMars.getConference(conferenceId);
	m_callerName.SetWindowTextW(stringToCStringW(res["creator"]));

	m_listParticipants.ResetContent();

	for (auto x : res["participant"]) {
		m_listParticipants.AddString(stringToCStringW(x));
	}
}


BEGIN_MESSAGE_MAP(CMarsIncomingCallDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_ACCEPT, &CMarsIncomingCallDlg::OnBnClickedButtonAccept)
	ON_BN_CLICKED(IDC_BUTTON_REJECT, &CMarsIncomingCallDlg::OnBnClickedButtonReject)
END_MESSAGE_MAP()


// CMarsIncomingCallDlg message handlers


void CMarsIncomingCallDlg::OnBnClickedButtonAccept()
{
	// TODO: Add your control notification handler code here
	OnOK();
	CMarsMeetingDlg dlgMarsMeeting(userInfo, conferenceId);
	dlgMarsMeeting.DoModal();
}


void CMarsIncomingCallDlg::OnBnClickedButtonReject()
{
	// TODO: Add your control notification handler code here
	CMarsFunctions mMars;
	json res = mMars.getConference(conferenceId);
	list<string> participant = res["participant"];
	participant.remove(userInfo.getId());
	mMars.updateConference(conferenceId, res["topic"], res["start"], participant);
	AfxMessageBox(_T("Successfully Rejected!!!"));
	OnOK();
}

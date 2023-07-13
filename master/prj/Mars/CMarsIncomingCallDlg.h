#pragma once
#include "afxdialogex.h"
#include "CUserInfo.h"


// CMarsIncomingCallDlg dialog

class CMarsIncomingCallDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMarsIncomingCallDlg)

public:
	CMarsIncomingCallDlg(CUserInfo& userInfo, string conferenceId, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CMarsIncomingCallDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_INCOMING_CALL };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonAccept();
	afx_msg void OnBnClickedButtonReject();
	CListBox m_listParticipants;
	CStatic m_callerName;

	CUserInfo& userInfo;
	string conferenceId;
};

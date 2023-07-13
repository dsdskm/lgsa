#pragma once
#include "afxdialogex.h"
#include "CUserInfo.h"


// CMarsConferenceDlg dialog

class CMarsConferenceDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMarsConferenceDlg)

public:
	CMarsConferenceDlg(CUserInfo& userInfo, list<string> participantList, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CMarsConferenceDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CONFERENCE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_topic;
	CDateTimeCtrl m_date;
	CDateTimeCtrl m_time;
	afx_msg void OnBnClickedButtonSave();
	CUserInfo& userInfo;
	list<string> participantList;
	CStatic m_participants;
};

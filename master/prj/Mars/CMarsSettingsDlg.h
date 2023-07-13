#pragma once
#include "afxdialogex.h"
#include "CUserInfo.h"


// CMarsSettingsDlg dialog

class CMarsSettingsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMarsSettingsDlg)

public:
	CMarsSettingsDlg(CUserInfo& userInfo, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CMarsSettingsDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SETTINGS };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_email;
	CEdit m_password;
	afx_msg void OnBnClickedButtonChangeEmail();
	afx_msg void OnBnClickedButtonChangePassword();
	CUserInfo& userInfo;
};

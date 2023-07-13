#pragma once
#include "afxdialogex.h"


// CMarsRegisterDlg dialog

class CMarsRegisterDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMarsRegisterDlg)

public:
	CMarsRegisterDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CMarsRegisterDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_REGISTER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonRegister();
	CEdit m_id;
	CEdit m_password;
	CEdit m_firstName;
	CEdit m_lastName;
	CEdit m_email;
};

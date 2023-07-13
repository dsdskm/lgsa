#pragma once
#include "afxdialogex.h"


// CMarsResetPasswordDlg dialog

class CMarsResetPasswordDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMarsResetPasswordDlg)

public:
	CMarsResetPasswordDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CMarsResetPasswordDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_RESET_PASSWORD };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_email;
	afx_msg void OnBnClickedButtonReset();
	CEdit m_id;
};

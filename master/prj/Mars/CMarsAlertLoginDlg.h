#pragma once
#include "afxdialogex.h"


// CMarsAlertLoginDlg dialog

class CMarsAlertLoginDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMarsAlertLoginDlg)

public:
	CMarsAlertLoginDlg(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CMarsAlertLoginDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_ALERT_LOGIN };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};

#pragma once
#include "afxdialogex.h"


// CMarsAdminDialog dialog

class CMarsAdminDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CMarsAdminDialog)

public:
	CMarsAdminDialog(CWnd* pParent = nullptr);   // standard constructor
	virtual ~CMarsAdminDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_ADMIN };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCheckBtn();
	CEdit mAdminPasswordControl;
	CListBox mUserListView;
	afx_msg void OnBnClickedEnableBtn();
	afx_msg void OnLbnSelchangeUserList();
	void doEnable(bool enable);
	void refreshList(bool isInit);
	afx_msg void OnBnClickedDisableBtn();
};

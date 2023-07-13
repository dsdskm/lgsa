#pragma once
#include "afxdialogex.h"
#include "CUserInfo.h"


// CMarsContactDlg dialog

class CMarsContactDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CMarsContactDlg)

public:
	CMarsContactDlg(CUserInfo& userInfo, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CMarsContactDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CONTACT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_editSearch;
	CListBox m_searchList;
	afx_msg void OnBnClickedButtonSearch();
	CUserInfo& userInfo;
	afx_msg void OnBnClickedButtonAdd();
};

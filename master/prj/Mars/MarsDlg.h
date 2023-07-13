
// MarsDlg.h : header file
//

#pragma once


// CMarsDlg dialog
class CMarsDlg : public CDialogEx
{
// Construction
public:
	CMarsDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MARS_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonRegister();
	afx_msg void OnBnClickedButtonResetPassword();
	afx_msg void OnBnClickedButtonLogin();
	CEdit m_id;
	CEdit m_password;
	CButton m_test;
	afx_msg void OnBnClickedButtonTest();
	afx_msg void OnBnClickedAdminBtn();
};

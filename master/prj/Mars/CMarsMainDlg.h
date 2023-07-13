#pragma once
#include "afxdialogex.h"
#include "CUserInfo.h"
#include "net/SessionController.h"

// CMarsMainDlg dialog

class CMarsMainDlg : public CDialogEx, public net::SessionEventHandler
{
	DECLARE_DYNAMIC(CMarsMainDlg)

public:
	CMarsMainDlg(CUserInfo userInfo, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CMarsMainDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_MAIN };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonSettings();
	afx_msg void OnBnClickedButtonAddConference();
	afx_msg void OnBnClickedButtonJoin();
	afx_msg void OnBnClickedButtonAddContact();
	afx_msg void OnBnClickedButtonCall();
	CUserInfo userInfo;
	CStatic m_welcome;
	CListBox m_contactList;
	CListBox m_conferenceList;
	CButton m_join;
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnLbnSelchangeListContact();
	afx_msg void OnLbnSelchangeListConference();
	list<string> getSelectedContactList();
	void updateUI();

	void OnCallReservation(const string& call_id, list<string> participant_id_list) override;
	void OnCallRequest(const string& call_id) override;
	void OnCallJoined(const string& call_id, const string& joined_uid) override;
	void OnCallLeft(const string& call_id, const string& left_uid) override;
	void OnCallReject(const string& call_id, const string& rejected_uid) override;
	void OnUserUpdate(const string& updated_uid) override {}
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};

// CMarsSettingsDlg.cpp : implementation file
//

#include "pch.h"
#include "Mars.h"
#include "afxdialogex.h"
#include "CMarsSettingsDlg.h"
#include "CUserInfo.h"
#include "CMarsFunctions.h"
#include "CommonUtils.h"

// CMarsSettingsDlg dialog

IMPLEMENT_DYNAMIC(CMarsSettingsDlg, CDialogEx)

CMarsSettingsDlg::CMarsSettingsDlg(CUserInfo& userInfo, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_SETTINGS, pParent),
	  userInfo(userInfo)
{
}

CMarsSettingsDlg::~CMarsSettingsDlg()
{
}

void CMarsSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_EMAIL, m_email);
	DDX_Control(pDX, IDC_EDIT_PASSWORD, m_password);
}


BEGIN_MESSAGE_MAP(CMarsSettingsDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_CHANGE_EMAIL, &CMarsSettingsDlg::OnBnClickedButtonChangeEmail)
	ON_BN_CLICKED(IDC_BUTTON_CHANGE_PASSWORD, &CMarsSettingsDlg::OnBnClickedButtonChangePassword)
END_MESSAGE_MAP()


// CMarsSettingsDlg message handlers


void CMarsSettingsDlg::OnBnClickedButtonChangeEmail()
{
	// TODO: Add your control notification handler code here
	CString s_email;
	m_email.GetWindowTextW(s_email);
	string email = string(CT2CA(s_email));

	CMarsFunctions mMars;
	bool res = mMars.changeEmail(userInfo.getId(), email);
	AfxMessageBox(_T("Successfully changed!"));
}


void CMarsSettingsDlg::OnBnClickedButtonChangePassword()
{
	// TODO: Add your control notification handler code here
	CString s_password;
	m_password.GetWindowTextW(s_password);

	string password = string(CT2CA(s_password));
	CMarsFunctions mMars;
	bool res = mMars.changePassword(userInfo.getId(), sha256(password));

	AfxMessageBox(_T("Successfully changed!"));

	//OnOK();
}

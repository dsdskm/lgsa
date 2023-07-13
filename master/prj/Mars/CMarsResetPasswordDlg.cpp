// CMarsResetPasswordDlg.cpp : implementation file
//

#include "pch.h"
#include "Mars.h"
#include "afxdialogex.h"
#include "CMarsResetPasswordDlg.h"
#include "CUserInfo.h"
#include "CMarsFunctions.h"

// CMarsResetPasswordDlg dialog

IMPLEMENT_DYNAMIC(CMarsResetPasswordDlg, CDialogEx)

CMarsResetPasswordDlg::CMarsResetPasswordDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_RESET_PASSWORD, pParent)
{

}

CMarsResetPasswordDlg::~CMarsResetPasswordDlg()
{
}

void CMarsResetPasswordDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_EMAIL, m_email);
	DDX_Control(pDX, IDC_EDIT_ID, m_id);
}


BEGIN_MESSAGE_MAP(CMarsResetPasswordDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_RESET, &CMarsResetPasswordDlg::OnBnClickedButtonReset)
END_MESSAGE_MAP()


// CMarsResetPasswordDlg message handlers


void CMarsResetPasswordDlg::OnBnClickedButtonReset()
{
	// TODO: Add your control notification handler code here
	CString s_id, s_email;
	m_id.GetWindowTextW(s_id);
	m_email.GetWindowTextW(s_email);

	string id = string(CT2CA(s_id));
	string email = string(CT2CA(s_email));

	CMarsFunctions mMars;
	bool res = mMars.resetPassword(id, email);
//	CRest rest;
//	json res = rest.send("DELETE", "email", body);
}

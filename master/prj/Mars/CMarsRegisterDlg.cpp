// CMarsRegisterDlg.cpp : implementation file
//

#include "pch.h"
#include "Mars.h"
#include "afxdialogex.h"
#include "CMarsRegisterDlg.h"
#include "CMarsFunctions.h"
#include "CommonUtils.h"


// CMarsRegisterDlg dialog

IMPLEMENT_DYNAMIC(CMarsRegisterDlg, CDialogEx)

CMarsRegisterDlg::CMarsRegisterDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_REGISTER, pParent)
{

}

CMarsRegisterDlg::~CMarsRegisterDlg()
{
}

void CMarsRegisterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_ID, m_id);
	DDX_Control(pDX, IDC_EDIT_PASSWORD, m_password);
	DDX_Control(pDX, IDC_EDIT_FIRST_NAME, m_firstName);
	DDX_Control(pDX, IDC_EDIT_LAST_NAME, m_lastName);
	DDX_Control(pDX, IDC_EDIT_EMAIL, m_email);
}


BEGIN_MESSAGE_MAP(CMarsRegisterDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_REGISTER, &CMarsRegisterDlg::OnBnClickedButtonRegister)
END_MESSAGE_MAP()


// CMarsRegisterDlg message handlers


void CMarsRegisterDlg::OnBnClickedButtonRegister()
{
	// TODO: Add your control notification handler code here
	CString s_id, s_email, s_password, s_firstName, s_lastName;
	m_id.GetWindowTextW(s_id);
	m_email.GetWindowTextW(s_email);
	m_password.GetWindowTextW(s_password);
	m_firstName.GetWindowTextW(s_firstName);
	m_lastName.GetWindowTextW(s_lastName);

	string id = string(CT2CA(s_id));
	string email = string(CT2CA(s_email));
	string password = sha256(string(CT2CA(s_password)));
	string firstName = string(CT2CA(s_firstName));
	string lastName = string(CT2CA(s_lastName));

	cout << password << endl;

	CMarsFunctions mMars;
	if (mMars.registerUser(id, email, password, firstName, lastName)) {
		AfxMessageBox(_T("Successfully registered!"));
		OnOK();
	}
	else {
		AfxMessageBox(_T("Fail to register!"));
	}
}

// MarsDlg.cpp : implementation file
//

#include "pch.h"
#include "framework.h"
#include "Mars.h"
#include "MarsDlg.h"
#include "afxdialogex.h"
#include "CMarsRegisterDlg.h"
#include "CMarsResetPasswordDlg.h"
#include "CMarsMainDlg.h"
#include "CMarsAlertLoginDlg.h"
#include "CMarsFunctions.h"
#include "CMarsCamTestDlg.h"
#include "CMarsAdminDialog.h"
#include "CommonUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMarsDlg dialog



CMarsDlg::CMarsDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MARS_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMarsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_ID, m_id);
	DDX_Control(pDX, IDC_EDIT_PASSWORD, m_password);
	DDX_Control(pDX, IDC_BUTTON_TEST, m_test);
}

BEGIN_MESSAGE_MAP(CMarsDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_REGISTER, &CMarsDlg::OnBnClickedButtonRegister)
	ON_BN_CLICKED(IDC_BUTTON_RESET_PASSWORD, &CMarsDlg::OnBnClickedButtonResetPassword)
	ON_BN_CLICKED(IDC_BUTTON_LOGIN, &CMarsDlg::OnBnClickedButtonLogin)
	ON_BN_CLICKED(IDC_BUTTON_TEST, &CMarsDlg::OnBnClickedButtonTest)
	ON_BN_CLICKED(IDC_BUTTON2, &CMarsDlg::OnBnClickedAdminBtn)
END_MESSAGE_MAP()


// CMarsDlg message handlers

BOOL CMarsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMarsDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMarsDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMarsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CMarsDlg::OnBnClickedButtonRegister()
{
	// TODO: Add your control notification handler code here
	CMarsRegisterDlg dlgRegister;
	dlgRegister.DoModal();
}


void CMarsDlg::OnBnClickedButtonResetPassword()
{
	// TODO: Add your control notification handler code here
	CMarsResetPasswordDlg dlgResetPassword;
	dlgResetPassword.DoModal();
}


void CMarsDlg::OnBnClickedButtonLogin()
{
	// TODO: Add your control notification handler code here
	CString s_id, s_password;
	m_id.GetWindowTextW(s_id);
	m_password.GetWindowTextW(s_password);

	string id = string(CT2CA(s_id));
	string password = sha256(string(CT2CA(s_password)));

	CMarsFunctions mMars;
	bool res = mMars.authenticateUser(id, password);

	if (res == true) { // login info correct
		CUserInfo mUserInfo(id);
		CMarsMainDlg dlgMain(mUserInfo);
		OnOK();
		dlgMain.DoModal();
	}
	else {
		CMarsAlertLoginDlg dlgAlert;
		dlgAlert.DoModal();
	}
}


void CMarsDlg::OnBnClickedButtonTest()
{
	// TODO: Add your control notification handler code here
	CMarsCamTestDlg dlgMarsCamTest;
	dlgMarsCamTest.DoModal();
}


void CMarsDlg::OnBnClickedAdminBtn()
{
	CMarsAdminDialog d;
	d.DoModal();
}

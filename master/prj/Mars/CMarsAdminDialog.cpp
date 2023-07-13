// CMarsAdminDialog.cpp : implementation file
//

#include "pch.h"
#include "Mars.h"
#include "afxdialogex.h"
#include "CMarsAdminDialog.h"
#include "CMarsFunctions.h"
#include "CommonUtils.h"


// CMarsAdminDialog dialog

IMPLEMENT_DYNAMIC(CMarsAdminDialog, CDialogEx)

CMarsAdminDialog::CMarsAdminDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_ADMIN, pParent)
{

}

CMarsAdminDialog::~CMarsAdminDialog()
{
}

void CMarsAdminDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, mAdminPasswordControl);
	DDX_Control(pDX, IDC_LIST1, mUserListView);
}


BEGIN_MESSAGE_MAP(CMarsAdminDialog, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CMarsAdminDialog::OnBnClickedCheckBtn)
	ON_BN_CLICKED(IDC_BUTTON2, &CMarsAdminDialog::OnBnClickedEnableBtn)
	ON_BN_CLICKED(IDC_BUTTON3, &CMarsAdminDialog::OnBnClickedDisableBtn)
END_MESSAGE_MAP()


// CMarsAdminDialog message handlers


void CMarsAdminDialog::OnBnClickedCheckBtn()
{
	refreshList(true);
}


void CMarsAdminDialog::OnBnClickedEnableBtn()
{
	doEnable(true);
}

void CMarsAdminDialog::refreshList(bool isInit) {
	CString password;
	mAdminPasswordControl.GetWindowTextW(password);
	CMarsFunctions mMars;
	if (password == "mars" || !isInit) {
		mUserListView.ResetContent();
		for (auto p : mMars.getAllUserList()) {
			string id = p["id"];
			string enable = "";
			if (p["isEnable"] == "1") {
				enable = "enable";
			}
			else {
				enable = "disable";
			}
			string email = "null";
			if (!p["email"].is_null()) {
				email = p["email"];
			}
			
			string status = "";
			if (!p["status"].is_null()) {
				status = p["status"];
			}
			mUserListView.AddString(stringToCStringW(id) + "  |  " + stringToCStringW (email) +"  |  " + stringToCStringW(status) + "  |  " + stringToCStringW(enable));
		}

		mAdminPasswordControl.SetWindowTextW(stringToCStringW(""));
	}
}
void CMarsAdminDialog::doEnable(bool enable) {
	int selectedCount = mUserListView.GetSelCount();
	CArray<int, int> selectedIndices;
	selectedIndices.SetSize(selectedCount);
	mUserListView.GetSelItems(selectedCount, selectedIndices.GetData());
	list<string> userIdList;
	CMarsFunctions mMars;
	for (int i = 0; i < selectedCount; i++) {
		int selectedIndex = selectedIndices[i];

		CString selectedItemText;
		mUserListView.GetText(selectedIndex, selectedItemText);

		CString uid;
		int currentPosition = 0;
		uid = selectedItemText.Tokenize(_T("  |  "), currentPosition);
		mMars.updateUserEnable(string(CT2CA(uid)), enable);
	}
	refreshList(false);
}

void CMarsAdminDialog::OnBnClickedDisableBtn()
{
	doEnable(false);
}

// CMarsContactDlg.cpp : implementation file
//

#include "pch.h"
#include "Mars.h"
#include "afxdialogex.h"
#include "CMarsContactDlg.h"
#include "CMarsFunctions.h"
#include <nlohmann/json.hpp>
#include "CommonUtils.h"

// CMarsContactDlg dialog

IMPLEMENT_DYNAMIC(CMarsContactDlg, CDialogEx)

CMarsContactDlg::CMarsContactDlg(CUserInfo& userInfo, CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_CONTACT, pParent),
	  userInfo(userInfo)
{

}

CMarsContactDlg::~CMarsContactDlg()
{
}

void CMarsContactDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_SEARCH, m_editSearch);
	DDX_Control(pDX, IDC_LIST_SEARCH, m_searchList);
}
  

BEGIN_MESSAGE_MAP(CMarsContactDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_SEARCH, &CMarsContactDlg::OnBnClickedButtonSearch)
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CMarsContactDlg::OnBnClickedButtonAdd)
END_MESSAGE_MAP()


// CMarsContactDlg message handlers


void CMarsContactDlg::OnBnClickedButtonSearch()
{
	// TODO: Add your control notification handler code here
	CString s_search;
	m_editSearch.GetWindowTextW(s_search);

	string searchKeyword = string(CT2CA(s_search));

	CMarsFunctions mMars;
//	bool res = mMars.searchUser(userInfo.getId(), searchKeyword);

	list<CUserInfo> tmpUserInfoList;
	for (auto p : mMars.searchUser(userInfo.getId(), searchKeyword)) {
		tmpUserInfoList.push_back(CUserInfo(p["id"], p["email"], p["firstname"], p["lastname"], p["ipAddress"], p["status"]));
	}


	m_searchList.ResetContent();
	for (auto it = tmpUserInfoList.begin(); it != tmpUserInfoList.end(); it++) {
		m_searchList.AddString(stringToCStringW(it->getFirstName() + " " + it->getLastName() + " / " + it->getId() + " (" + it->getEmail() + ")"));
	}

	//	json res = rest.send("POST", "user", body);
	//	std::cout << "jsonString" << res.dump() << std::endl;
	//	std::cout << res["lastname"] << std::endl;
}


void CMarsContactDlg::OnBnClickedButtonAdd()
{
	// TODO: Add your control notification handler code here
	CListBox* pList1 = (CListBox*)GetDlgItem(IDC_LIST_SEARCH);

	int nSel = pList1->GetCurSel();
	if (nSel != LB_ERR) {
		CString ItemSelected;
		pList1->GetText(nSel, ItemSelected);
		CString szSub;

		AfxExtractSubString(szSub, ItemSelected, 1, '/');
		AfxExtractSubString(ItemSelected, szSub, 1, ' ');

		list<CUserInfo> tmp = userInfo.getBuddy();
		list<string> new_buddy;

		for (auto it = tmp.begin(); it != tmp.end(); it++) {
			new_buddy.push_back(it->getId());
		}
		
		if (find(new_buddy.begin(), new_buddy.end(), string(CT2CA(ItemSelected))) == new_buddy.end()) {
			new_buddy.push_back(string(CT2CA(ItemSelected)));

			CMarsFunctions mMars;
			mMars.updateContact(userInfo.getId(), new_buddy);
			userInfo.update();
			AfxMessageBox(_T("Successfully added"));
		}
		else {
			AfxMessageBox(_T("Already your buddy!!"));
		}
	}
}
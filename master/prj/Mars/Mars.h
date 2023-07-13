
// Mars.h : main header file for the PROJECT_NAME application
//

#pragma once
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CMarsApp:
// See Mars.cpp for the implementation of this class
//

class CMarsApp : public CWinApp
{
public:
	CMarsApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CMarsApp theApp;

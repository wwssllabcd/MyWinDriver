
// MFCApplication1.h : PROJECT_NAME ���ε{�����D�n���Y��
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�� PCH �]�t���ɮ׫e���]�t 'stdafx.h'"
#endif

#include "resource.h"		// �D�n�Ÿ�


// CMfcWinDriver: 
// �аѾ\��@�����O�� MFCApplication1.cpp
//

class CMfcWinDriver : public CWinApp
{
public:
	CMfcWinDriver();

// �мg
public:
	virtual BOOL InitInstance();

// �{���X��@

	DECLARE_MESSAGE_MAP()
};

extern CMfcWinDriver theApp;
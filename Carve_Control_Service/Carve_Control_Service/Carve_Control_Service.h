
// Carve_Control_Service.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CCarve_Control_ServiceApp:
// �йش����ʵ�֣������ Carve_Control_Service.cpp
//

class CCarve_Control_ServiceApp : public CWinApp
{
public:
	CCarve_Control_ServiceApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CCarve_Control_ServiceApp theApp;
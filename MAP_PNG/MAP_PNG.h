
// MAP_PNG.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CMAP_PNGApp:
// �йش����ʵ�֣������ MAP_PNG.cpp
//

class CMAP_PNGApp : public CWinAppEx
{
public:
	CMAP_PNGApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CMAP_PNGApp theApp;
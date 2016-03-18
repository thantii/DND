// MAPRAN2TXT.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "windows.h"
#include "string"
#include "comutil.h"

void Step(CHAR*& p)
{
	for (;;)
	{
		if (*p=='\t' || *p=='\n' || *p==0)
			return;
		p++;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 4)
		return -1;

	TCHAR* szRuleFile = argv[1];
	TCHAR* szSrc = argv[2];
	TCHAR* szTar = argv[3];

	HANDLE hSrc = NULL;
	HANDLE hTar = NULL;
	DWORD dwSize = 0;
	DWORD dw = 0;
	CHAR* pSrc = NULL;
	WCHAR szBuf[MAX_PATH] = {0};

	hSrc = CreateFile(szSrc, 
		GENERIC_READ, 
		FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
		NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	hTar = CreateFile(szTar, 
		GENERIC_WRITE, 
		0,
		NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (INVALID_HANDLE_VALUE==hSrc || INVALID_HANDLE_VALUE==hTar)
		goto exit1;

	dwSize = GetFileSize(hSrc, NULL);
	pSrc = new CHAR[dwSize + 1];

	ReadFile(hSrc, 
		pSrc, 
		dwSize,
		&dw,
		NULL);
	pSrc[dwSize] = 0;

	CHAR* pBegin = pSrc;
	CHAR* pEnd = pSrc;

	for (;;)
	{
		Step(pEnd);

		if (pEnd==pBegin)
		{
			DWORD dw = GetPrivateProfileStringW(L"app", L"TAB", L"", szBuf, MAX_PATH, szRuleFile);
			if (0==dw)
			{
				int a = 1;
			}
			_bstr_t bstr(szBuf);
			LPCSTR pAnsi = (LPCSTR)bstr;
			WriteFile(hTar, pAnsi, strlen(pAnsi), &dw, NULL);
		}
		else
		{
			std::string str(pBegin, pEnd - pBegin);
			if (0!=strcmp(str.c_str(), "\n"))
			{
				_bstr_t bstr1(str.c_str());
				DWORD dw = GetPrivateProfileStringW(L"app", bstr1, bstr1, szBuf, MAX_PATH, szRuleFile);
				_bstr_t bstr2(szBuf);
				LPCSTR pAnsi = (LPCSTR)bstr2;
				WriteFile(hTar, pAnsi, strlen(pAnsi), &dw, NULL);
			}
		}

		if ('\n'==*pEnd)
		{
			WriteFile(hTar, "\r\n", 2, &dw, NULL);
		}
		else if (0==*pEnd)
		{
			break;
		}

		pEnd++;
		pBegin = pEnd;
	}

exit1:
	return 0;
}


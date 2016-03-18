
// MAP_PNG.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "MAP_PNG.h"
#include "MAP_PNGDlg.h"
#include "vector"
#include "locale.h"

#include <gdiplus.h>
using namespace Gdiplus;
#pragma  comment(lib, "gdiplus.lib")

#include "strsafe.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMAP_PNGApp

BEGIN_MESSAGE_MAP(CMAP_PNGApp, CWinAppEx)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CMAP_PNGApp 构造

CMAP_PNGApp::CMAP_PNGApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CMAP_PNGApp 对象

CMAP_PNGApp theApp;


// CMAP_PNGApp 初始化

struct AUTOTILE
{
	int a;
	int b;
	int c;
	int d;
};

AUTOTILE Autotiles[6][8] = {
	{{27, 28, 33, 34}, { 5, 28, 33, 34}, {27,  6, 33, 34}, { 5,  6, 33, 34}, {27, 28, 33, 12}, { 5, 28, 33, 12}, {27,  6, 33, 12}, { 5,  6, 33, 12}},
	{{27, 28, 11, 34}, { 5, 28, 11, 34}, {27,  6, 11, 34}, { 5,  6, 11, 34}, {27, 28, 11, 12}, { 5, 28, 11, 12}, {27,  6, 11, 12}, { 5,  6, 11, 12}},
	{{25, 26, 31, 32}, {25,  6, 31, 32}, {25, 26, 31, 12}, {25,  6, 31, 12}, {15, 16, 21, 22}, {15, 16, 21, 12}, {15, 16, 11, 22}, {15, 16, 11, 12}},
	{{29, 30, 35, 36}, {29, 30, 11, 36}, { 5, 30, 35, 36}, { 5, 30, 11, 36}, {39, 40, 45, 46}, { 5, 40, 45, 46}, {39,  6, 45, 46}, { 5,  6, 45, 46}},
	{{25, 30, 31, 36}, {15, 16, 45, 46}, {13, 14, 19, 20}, {13, 14, 19, 12}, {17, 18, 23, 24}, {17, 18, 11, 24}, {41, 42, 47, 48}, { 5, 42, 47, 48}},
	{{37, 38, 43, 44}, {37,  6, 43, 44}, {13, 18, 19, 24}, {13, 14, 43, 44}, {37, 42, 43, 48}, {17, 18, 47, 48}, {13, 18, 43, 48}, { 1,  2,  7,  8}}
};

BOOL unicodeToUtf8(const WCHAR *zWideFilename , char * szDes, size_t len)
{
	int nByte;
	char *zFilename;

	nByte = WideCharToMultiByte(CP_UTF8, 0, zWideFilename, -1, 0, 0, 0, 0);
	zFilename = new char[nByte];
	if( zFilename==0 ){
		return FALSE;
	}
	nByte = WideCharToMultiByte(CP_UTF8, 0, zWideFilename, -1, zFilename, nByte,
		0, 0);
	if( nByte == 0 ){
		delete [] zFilename;
		zFilename = 0;
		return FALSE;
	}

	StringCchCopyA(szDes,len,zFilename);
	delete [] zFilename;
	return TRUE;
}

BOOL utf8ToUnicode(const char *zFilename ,WCHAR * szDes, size_t nLen)
{
	int nChar;
	WCHAR *zWideFilename;

	nChar = MultiByteToWideChar(CP_UTF8, 0, zFilename, -1, NULL, 0);
	zWideFilename = new WCHAR[nChar];
	if( zWideFilename==0 ){
		return FALSE;
	}
	nChar = MultiByteToWideChar(CP_UTF8, 0, zFilename, -1, zWideFilename, nChar);
	if( nChar==0 ){
		delete [] zWideFilename;
		zWideFilename = 0;
		return FALSE;
	}

	StringCchCopyW(szDes,nLen,zWideFilename);
	delete [] zWideFilename;
	return TRUE;
}

void OutputTile(HDC m_hSrc, HDC m_hTar, int n, std::vector<HBITMAP>& vec, int nTarX, int nTarY, BLENDFUNCTION blend)
{
	// 空白
	if (n == 0)
	{
	}
	else
	{
		int nTileID = n / 48;
		if (nTileID >= vec.size())
		{
			nTileID = 0;
		}

		// 正常色块
		if (nTileID == 0)
		{
			n = n - 384;

			BITMAP bmpInfo = {0};
			int n1 = ::GetObject(vec[0], sizeof(BITMAP), &bmpInfo);

			int nWidth = bmpInfo.bmWidth / 32;

			int nX = n % nWidth;
			int nY = n / nWidth;

			::SelectObject(m_hSrc, vec[0]);
			BOOL b = ::AlphaBlend(m_hTar, nTarX * 32, nTarY * 32, 32, 32, m_hSrc, nX * 32, nY * 32, 32, 32, blend);

			if (FALSE==b)
			{
				BitBlt(m_hTar, nTarX * 32, nTarY * 32, 32, 32, m_hSrc, nX * 32, nY * 32, SRCCOPY);
			}

// 			::AlphaBlend(m_hTar, nTarX * 32, nTarY * 32, 32, 32, m_hSrc, nX * 32, nY * 32, 32, 32, blend); 
// 			//BitBlt(m_hTar, nTarX * 32, nTarY * 32, 32, 32, m_hSrc, nX * 32, nY * 32, SRCCOPY);

			int a = 1;
		}
		else
		{
			// 自动图元
			int nTileID = n / 48;
			::SelectObject(m_hSrc, vec[nTileID]);
			n %= 48;

			AUTOTILE tiles = Autotiles[n / 8][n % 8];
			int* real_tiles = (int*)&tiles;

			for (int i = 0; i < 4; i++)
			{
				int tile_position = real_tiles[i] - 1;
				CRect rc(CPoint(tile_position % 6 * 16, tile_position / 6 * 16), CSize(16, 16));

				{
					int xSrc = tile_position % 6 * 16;
					int ySrc = tile_position / 6 * 16;
					int xTar = nTarX * 32 + i % 2 * 16;
					int yTar = nTarY * 32 + i / 2 * 16;

					//BitBlt(m_hTar, xTar, yTar, 16, 16, m_hSrc, xSrc, ySrc, SRCCOPY);
					BOOL b = ::AlphaBlend(m_hTar, xTar, yTar, 16, 16, m_hSrc, xSrc, ySrc, 16, 16, blend);
					if (FALSE==b)
					{
						BitBlt(m_hTar, xTar, yTar, 16, 16, m_hSrc, xSrc, ySrc, SRCCOPY);
					}
				}
			}
		}

	}
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT num= 0;
	UINT size= 0;

	ImageCodecInfo* pImageCodecInfo= NULL;

	GetImageEncodersSize(&num, &size);
	if(size== 0)
	{
		return -1;
	}
	pImageCodecInfo= (ImageCodecInfo*)(malloc(size));
	if(pImageCodecInfo== NULL)
	{
		return -1;
	}

	GetImageEncoders(num, size, pImageCodecInfo);

	for(UINT j=0; j< num; ++j)
	{
		if(wcscmp(pImageCodecInfo[j].MimeType, format)== 0)
		{
			*pClsid= pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;
		}
	}

	free(pImageCodecInfo);
	return -1;
}

void OutputBitmap(LPCTSTR szSrc, LPCTSTR szTar, LPCTSTR szDir, I_LBCLoadImage* pImg)
{
	_bstr_t bstrTar = szTar;

	CStdioFile file;
	file.Open(szSrc, CFile::modeRead|CFile::typeText);

	CString str;

	{
		// 第一行是id
		file.ReadString(str);

		// 第二行有文件名
		file.ReadString(str);

		WCHAR szPath[MAX_PATH] = {0};
		utf8ToUnicode(str, szPath, MAX_PATH);
		_bstr_t bstr = szPath;
		LPCTSTR sz = bstr;

		// 从文件名把所有位图拿到
		BOOL bFirst = TRUE;
		std::vector<HBITMAP> vec;

		TCHAR* p = _tcstok((TCHAR*)(LPCTSTR)sz, _T(","));
		while (p)
		{
			CString strFile;
			if (bFirst)
			{
				bFirst = FALSE;
				strFile.Format(_T("%sGraphics\\Tilesets\\%s.png"), szDir, p);
			}
			else
			{
				strFile.Format(_T("%sGraphics\\Autotiles\\%s.png"), szDir, p);
			}

			LBCLOADIMAGE_FILE lf = {0};
			lf.lpszPath = _bstr_t(strFile);
			lf.nOption = LBCLOADIMAGE_OPTION_DIB;
			HBITMAP hRet = pImg->LoadImageFromFile(&lf);

//			if (bFirst)
			{
				LBCLoadImage_PreMul32Bit(hRet);
			}

			vec.push_back(hRet);
			p = _tcstok(NULL, _T(","));
		}

		// 第三行是地图大小
		file.ReadString(str);

		// 创建大小合适的位图
		int nX = 0; int nY = 0;
		p = _tcstok((TCHAR*)(LPCTSTR)str, _T(","));
		if (p)
		{
			nX = _ttoi64(p);
			p = _tcstok(NULL, _T(","));
			nY = _ttoi64(p);
		}
		int nTarWidth = nX * 32;
		int nTarHeight = nY * 32;

		BYTE *pvBits = NULL;          // pointer to DIB section
		BITMAPINFO bmi;        // bitmap header

		ZeroMemory(&bmi, sizeof(BITMAPINFO));
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = nTarWidth;
		bmi.bmiHeader.biHeight = nTarHeight;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;         // four 8-bit components
		bmi.bmiHeader.biCompression = BI_RGB;
		bmi.bmiHeader.biSizeImage = nTarWidth * nTarHeight * 4;

		HBITMAP hbitmap = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void **)&pvBits, NULL, 0);

		// 第四行到最后，是地图数据
		int nTarX = 0;
		int nTarY = 0;

		HDC m_hDC = ::GetDC(NULL);
		HDC m_hSrc = ::CreateCompatibleDC(m_hDC);
		HDC m_hTar = ::CreateCompatibleDC(m_hDC);
		::SelectObject(m_hTar, hbitmap);

		::FillRect(m_hTar, CRect(0, 0, nTarWidth, nTarHeight), CBrush(RGB(255, 255, 255)));

		BLENDFUNCTION blend;
		blend.BlendOp = AC_SRC_OVER;  
		blend.BlendFlags = 0;
		blend.AlphaFormat = AC_SRC_ALPHA;
		blend.SourceConstantAlpha = 255;

		for (;;)
		{
			nTarX = 0;

			BOOL b = file.ReadString(str);
			if (FALSE==b)
				break;

			p = _tcstok((TCHAR*)(LPCTSTR)str, _T(","));
			while (p)
			{
				int n = _ttoi64(p);

				OutputTile(m_hSrc, m_hTar, n, vec, nTarX, nTarY, blend);
				p = _tcstok(NULL, _T(","));
				nTarX++;
			}
			nTarY++;

			if (nTarY >= nY)
			{
				nTarY = 0;
			}
		}

		ULONG_PTR m_gdiplusToken;
		Gdiplus::GdiplusStartupInput StartupInput;
		GdiplusStartup(&m_gdiplusToken,&StartupInput,NULL); // GDI+初始化

		//保存到磁盘上的bmp文件
		CLSID pngid;
		GetEncoderClsid(L"image/png", &pngid);
		Gdiplus::Bitmap bitmap(hbitmap, NULL);
		bitmap.Save(bstrTar, &pngid, NULL);

		/*
		DWORD dwSize = bmi.bmiHeader.biSizeImage;

		BITMAPFILEHEADER bf = {0};  
		HANDLE hDib = GlobalAlloc(GHND, dwSize + sizeof(BITMAPINFOHEADER));  
		LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);  
		*lpbi = bmi.bmiHeader;  

		::GetDIBits(m_hTar, hbitmap, 0, bmi.bmiHeader.biHeight, (BYTE*)lpbi + sizeof(BITMAPINFOHEADER), (BITMAPINFO*)lpbi, DIB_RGB_COLORS);  

		HANDLE hFile = CreateFile(szTar, 
			GENERIC_WRITE, 
			FILE_SHARE_READ, 
			NULL,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL, 
			NULL);

		if (hFile == INVALID_HANDLE_VALUE)
			return ;

		bf.bfType = 0x4d42;  
		dwSize += sizeof(BITMAPINFOHEADER);  
		bf.bfSize = dwSize;  
		bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);  
		DWORD dwBytesWritten = 0;
		WriteFile(hFile, (BYTE*)&bf, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
		WriteFile(hFile, (BYTE*)lpbi, dwSize, &dwBytesWritten, NULL);
		CloseHandle(hFile);

		GlobalUnlock(hDib);  
		GlobalFree(hDib);  
		*/

		DeleteObject(hbitmap);

		::ReleaseDC(NULL, m_hDC);
		::DeleteDC(m_hSrc);
		::DeleteDC(m_hTar);

		{
			for (size_t i = 0; i < vec.size(); i++)
			{
				DeleteObject(vec[i]);
			}
		}
	}
}

BOOL CMAP_PNGApp::InitInstance()
{
	setlocale(0, "chs");

	I_LBCLoadImage* pImg = NULL;
	BOOL b =  LBCLoadImage_CreateInstance_Libpng(&pImg);

	TCHAR szDir[MAX_PATH] = {0};
	GetModuleFileName(NULL, szDir, MAX_PATH);
	PathAppend(szDir, _T(".."));
	PathAddBackslash(szDir);

	CString strDir = szDir;

	CFileFind ff;
	BOOL bFind = ff.FindFile(strDir + "Maps\\*.txt");
	while(bFind)
	{
		bFind = ff.FindNextFile() ;

		if (!ff.IsDots())
		{
			CString str = ff.GetFileName();
			CString strOut = str;
			strOut.Replace(_T(".txt"), _T(".png"));
			
			OutputBitmap(strDir + _T("Maps\\") + str, strDir + _T("MAP_PNG\\") + strOut, strDir, pImg);
		}
	}

	return FALSE;
}

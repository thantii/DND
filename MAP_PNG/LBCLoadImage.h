#pragma once

#include "I_LBCLoadImage.h"
#include "list"
#include "shlwapi.h"
#pragma comment(lib, "shlwapi.lib")

// 支持的控制
// #define LBCLOADIMAGE_WIN2000_PNG_SUPPORT
// #define LBCLOADIMAGE_WIN2000_JPG_SUPPORT
// #define LBCLOADIMAGE_WIN2000_GIF_SUPPORT

#define LBCLOADIMAGE_TYPE_UNKNOWN					0x00
#define LBCLOADIMAGE_TYPE_BMP						0x01
#define LBCLOADIMAGE_TYPE_PNG						0x02
#define LBCLOADIMAGE_TYPE_JPG						0x03
#define LBCLOADIMAGE_TYPE_GIF						0x04
#define LBCLOADIMAGE_TYPE_COUNT						0x05

extern HBITMAP LBCLoadImage_FromFile(LPCWSTR lpszPath, UINT nOption = LBCLOADIMAGE_OPTION_DIB, HDC hDC = NULL, UINT nType = LBCLOADIMAGE_TYPE_UNKNOWN);
extern HBITMAP LBCLoadImage_FromBuffer(void* pBuf, UINT nBufLen, UINT nOption = LBCLOADIMAGE_OPTION_DIB, HDC hDC = NULL, UINT nType = LBCLOADIMAGE_TYPE_UNKNOWN);
extern BOOL LBCLoadImage_CreateBITMAPINFO(const BITMAP* pBmp, PBITMAPINFO pBmpInfo);
extern void LBCLoadImage_PreMul32Bit(void* pSource, UINT nLen);
extern BOOL LBCLoadImage_PreMul32Bit(HBITMAP hSource, HDC hDC = NULL);

#ifdef _MT
	#ifdef _DLL
		#define LOADIMAGE_LIB_SUFFIX "MD"
	#else
		#define LOADIMAGE_LIB_SUFFIX "MT"
	#endif
#else
	#define LOADIMAGE_LIB_SUFFIX "ST"
#endif

extern BOOL LBCLoadImage_CreateInstance_Basic(UINT nInsID, I_LBCLoadImage** ppIns);
#pragma comment(lib, "LBCLoadImage_Basic" LOADIMAGE_LIB_SUFFIX ".lib")

#ifdef LBCLOADIMAGE_WIN2000_PNG_SUPPORT
	extern BOOL LBCLoadImage_CreateInstance_Libpng(I_LBCLoadImage** ppIns);
	#pragma comment(lib, "LBCLoadImage_png" LOADIMAGE_LIB_SUFFIX ".lib")
	#pragma comment(lib, "unzip" LOADIMAGE_LIB_SUFFIX ".lib")
#endif

#ifdef LBCLOADIMAGE_WIN2000_JPG_SUPPORT
	extern BOOL LBCLoadImage_CreateInstance_IJG(I_LBCLoadImage** ppIns);
	#pragma comment(lib, "LBCLoadImage_jpg" LOADIMAGE_LIB_SUFFIX ".lib")
#endif

#ifdef LBCLOADIMAGE_WIN2000_GIF_SUPPORT
	extern BOOL LBCLoadImage_CreateInstance_Libgif(I_LBCLoadImage** ppIns);
	#pragma comment(lib, "LBCLoadImage_gif" LOADIMAGE_LIB_SUFFIX ".lib")
#endif

class LBCLoadImageWrap
{
	typedef BOOL (*PFNLBCLoadImage_CreateInstance) (UINT nInsID, I_LBCLoadImage** ppIns);
	typedef BOOL (*PFNLBCLoadImage_CreateInstanceOne) (I_LBCLoadImage** ppIns);

	LBCLoadImageWrap()
	{
		ZeroMemory(m_arrLoader, sizeof(m_arrLoader));
		_InitTypeLoader();

		_Init(LBCLoadImage_CreateInstance_Basic);

#ifdef LBCLOADIMAGE_WIN2000_PNG_SUPPORT
		_Init(LBCLOADIMAGE_INSTANCE_ID_LIBPNG, LBCLoadImage_CreateInstance_Libpng);
#endif
#ifdef LBCLOADIMAGE_WIN2000_JPG_SUPPORT
		_Init(LBCLOADIMAGE_INSTANCE_ID_LIBJPG, LBCLoadImage_CreateInstance_IJG);
#endif
#ifdef LBCLOADIMAGE_WIN2000_GIF_SUPPORT
		_Init(LBCLOADIMAGE_INSTANCE_ID_LIBGIF, LBCLoadImage_CreateInstance_Libgif);
#endif
	}

public:
	~LBCLoadImageWrap()
	{
		Uninit();
	}

	BOOL Init(LPCWSTR lpszDllPath)
	{
		if (NULL==lpszDllPath)
			return FALSE;

		HMODULE hDll = LoadLibraryW(lpszDllPath);
		BOOL bRet = Init(hDll);
		if (FALSE==bRet && hDll)
			FreeLibrary(hDll);

		return bRet;
	}

	BOOL Init(LPCSTR lpszDllPath)
	{
		if (NULL==lpszDllPath)
			return FALSE;
		
		HMODULE hDll = LoadLibraryA(lpszDllPath);
		BOOL bRet = Init(hDll);
		if (FALSE==bRet && hDll)
			FreeLibrary(hDll);
		
		return bRet;
	}

	BOOL Init(HMODULE hDll)
	{
		if (NULL==hDll)
			return FALSE;

		BOOL bRet = FALSE;
		PFNLBCLoadImage_CreateInstance pfnCreateInstance	= 
			(PFNLBCLoadImage_CreateInstance)GetProcAddress(hDll, "LBCLoadImage_CreateInstance");
		
		if (pfnCreateInstance)
			bRet = _Init(pfnCreateInstance);

		if (bRet)
			m_lsDll.push_back(hDll);

		return bRet;
	}

	void Uninit()
	{
		for (UINT i = 0; i < LBCLOADIMAGE_INSTANCE_COUNT; i++)
		{
			if (m_arrLoader[i])
			{
				I_LBCLoadImage* p = m_arrLoader[i];
				m_arrLoader[i] = NULL;

				p->Free();
			}
		}

		LIST_DLL::iterator it = m_lsDll.begin();
		for (; it != m_lsDll.end(); it++)
			FreeLibrary(*it);

		m_lsDll.clear();
	}

	static LBCLoadImageWrap& Instance()
	{
		static LBCLoadImageWrap s_ins;
		return s_ins;
	}

	HBITMAP LoadImageFromFile(LPCWSTR lpszPath, UINT nOption, HDC hDC, UINT nType)
	{
		if (NULL==lpszPath)
			return NULL;
		
		HDC hTmpDC = NULL;
		HBITMAP hRet = NULL;
		LBCLOADIMAGE_FILE li_file = {hDC, lpszPath, nOption};

		if (NULL==hDC)
			hTmpDC = hDC = ::GetDC(NULL);
		
		// 分析图片类型
		if (LBCLOADIMAGE_TYPE_UNKNOWN==nType)
			nType = _GetType(lpszPath);

		UINT& nLoader0 = m_arrTypeLoader[nType][0];
		UINT& nLoader1 = m_arrTypeLoader[nType][1];

		if (m_arrLoader[nLoader0])
			hRet = m_arrLoader[nLoader0]->LoadImageFromFile(&li_file);

		if (NULL==hRet && m_arrLoader[nLoader1])
			hRet = m_arrLoader[nLoader1]->LoadImageFromFile(&li_file);
		
		if (hTmpDC)
			::ReleaseDC(NULL, hTmpDC);

		return hRet;
	}

	HBITMAP LoadImageFromBuffer(void* pBuf, UINT nBufLen, UINT nOption, HDC hDC, UINT nType)
	{
		return NULL;
	}

private:
	BOOL _Init(PFNLBCLoadImage_CreateInstance pfnCreateInstance)
	{
		BOOL bRet = FALSE;

		for (UINT i = 0; i < LBCLOADIMAGE_INSTANCE_COUNT; i++)
		{
			if (NULL==m_arrLoader[i])
			{
				I_LBCLoadImage* pTmp = NULL;
				if (pfnCreateInstance(i, &pTmp))
				{
					if (pTmp->Init())
					{
						m_arrLoader[i] = pTmp;
						bRet = TRUE;
					}
					else
						pTmp->Free();
				}
			}
		}

		return bRet;
	}

	BOOL _Init(UINT nInsID, PFNLBCLoadImage_CreateInstanceOne pfnCreateInstance)
	{
		BOOL bRet = FALSE;

		if (NULL==m_arrLoader[nInsID])
		{
			I_LBCLoadImage* pTmp = NULL;
			if (pfnCreateInstance(&pTmp))
			{
				if (pTmp->Init())
				{
					m_arrLoader[nInsID] = pTmp;
					bRet = TRUE;
				}
				else
					pTmp->Free();
			}
		}
		
		return bRet;
	}

	UINT _GetType(LPCWSTR lpszPath)
	{
		UINT nRet = LBCLOADIMAGE_TYPE_UNKNOWN;

		LPWSTR pExt = PathFindExtensionW(lpszPath);
		if (NULL==pExt)
			nRet = LBCLOADIMAGE_TYPE_UNKNOWN;
		else if (0==StrCmpIW(pExt, L".bmp"))
			nRet = LBCLOADIMAGE_TYPE_BMP;
		else if (0==StrCmpIW(pExt, L".png"))
			nRet = LBCLOADIMAGE_TYPE_PNG;
		else if (0==StrCmpIW(pExt, L".jpg"))
			nRet = LBCLOADIMAGE_TYPE_JPG;
		else if (0==StrCmpIW(pExt, L".gif"))
			nRet = LBCLOADIMAGE_TYPE_GIF;
		
		return nRet;
	}

	void _InitTypeLoader()
	{
		m_arrTypeLoader[LBCLOADIMAGE_TYPE_BMP][0] = LBCLOADIMAGE_INSTANCE_ID_LOADIMAGEW;
		m_arrTypeLoader[LBCLOADIMAGE_TYPE_BMP][1] = LBCLOADIMAGE_INSTANCE_ID_GDIP;

		m_arrTypeLoader[LBCLOADIMAGE_TYPE_PNG][0] = LBCLOADIMAGE_INSTANCE_ID_LIBPNG;
		m_arrTypeLoader[LBCLOADIMAGE_TYPE_PNG][1] = LBCLOADIMAGE_INSTANCE_ID_GDIP;

		m_arrTypeLoader[LBCLOADIMAGE_TYPE_JPG][0] = LBCLOADIMAGE_INSTANCE_ID_LIBJPG;
		m_arrTypeLoader[LBCLOADIMAGE_TYPE_JPG][1] = LBCLOADIMAGE_INSTANCE_ID_GDIP;

		m_arrTypeLoader[LBCLOADIMAGE_TYPE_GIF][0] = LBCLOADIMAGE_INSTANCE_ID_LIBGIF;
		m_arrTypeLoader[LBCLOADIMAGE_TYPE_GIF][1] = LBCLOADIMAGE_INSTANCE_ID_GDIP;

		m_arrTypeLoader[LBCLOADIMAGE_TYPE_UNKNOWN][0] = LBCLOADIMAGE_INSTANCE_ID_GDIP;
		m_arrTypeLoader[LBCLOADIMAGE_TYPE_UNKNOWN][1] = LBCLOADIMAGE_INSTANCE_ID_NONE;
	}

private:
	I_LBCLoadImage* m_arrLoader[LBCLOADIMAGE_INSTANCE_COUNT];
	UINT m_arrTypeLoader[LBCLOADIMAGE_TYPE_COUNT][2];

	typedef std::list<HMODULE> LIST_DLL;
	LIST_DLL m_lsDll;
};

inline HBITMAP LBCLoadImage_FromFile(LPCWSTR lpszPath, UINT nOption, HDC hDC, UINT nType)
{
	return LBCLoadImageWrap::Instance().LoadImageFromFile(lpszPath, nOption, hDC, nType);
}

inline HBITMAP LBCLoadImage_FromBuffer(void* pBuf, UINT nBufLen, UINT nOption, HDC hDC, UINT nType)
{
	return LBCLoadImageWrap::Instance().LoadImageFromBuffer(pBuf, nBufLen, nOption, hDC, nType);
}

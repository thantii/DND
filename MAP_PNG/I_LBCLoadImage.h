#pragma once

// 兼容位图
#define LBCLOADIMAGE_OPTION_DDB					0x00
// 设备无关位图
#define LBCLOADIMAGE_OPTION_DIB					0x01

#define LBCLOADIMAGE_INSTANCE_ID_NONE			0xFF
#define LBCLOADIMAGE_INSTANCE_ID_LOADIMAGEW		0x00
#define LBCLOADIMAGE_INSTANCE_ID_GDIP			0x01
#define LBCLOADIMAGE_INSTANCE_ID_OLE			0x02
#define LBCLOADIMAGE_INSTANCE_ID_LIBPNG			0x03
#define LBCLOADIMAGE_INSTANCE_ID_LIBJPG			0x04
#define LBCLOADIMAGE_INSTANCE_ID_LIBGIF			0x05
#define LBCLOADIMAGE_INSTANCE_COUNT				0x06

struct I_LBCLoadImage;
extern BOOL LBCLoadImage_CreateInstance(UINT nInsID, I_LBCLoadImage** ppIns);
//===============================================================
typedef struct  
{
	HDC hDC;
	LPCWSTR lpszPath;
	UINT nOption;
} LBCLOADIMAGE_FILE, *PLBCLOADIMAGE_FILE;

typedef struct  
{
	HDC hDC;
	void* pBuf;
	UINT nBufLen;
	UINT nOption;
} LBCLOADIMAGE_BUF, *PLBCLOADIMAGE_BUF;

struct I_LBCLoadImage
{
	virtual ~I_LBCLoadImage() {}

	virtual void Free() = 0;

	virtual BOOL Init() = 0;
	virtual HBITMAP LoadImageFromFile(PLBCLOADIMAGE_FILE p) = 0;
	virtual HBITMAP LoadImageFromBuffer(PLBCLOADIMAGE_BUF p) = 0;
};


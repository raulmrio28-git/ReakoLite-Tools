/*
** ===========================================================================
** File: common.c
** Description: ReakoLite library common code
** Copyright (c) 2024 raulmrio28-git.
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 08/26/2024	raulmrio28-git	Add header creation
** 08/23/2024	raulmrio28-git	Initial version
** ===========================================================================
*/

/*
**----------------------------------------------------------------------------
**  Includes
**----------------------------------------------------------------------------
*/

#include "common.h"
#include <stdio.h>

/*
**----------------------------------------------------------------------------
**  Definitions
**----------------------------------------------------------------------------
*/

/*
**----------------------------------------------------------------------------
**  Type Definitions
**----------------------------------------------------------------------------
*/

/*
**----------------------------------------------------------------------------
**  Global variables
**----------------------------------------------------------------------------
*/

uint16_t RLS_Common_StdPal[RLS_SPAL_SIZE];
uint16_t RLS_Common_ExtPal[RLS_EPAL_SIZE];

uint16_t RLS_Common_Block[2*2];
uint32_t RLS_Common_ExtPal_CIdx;

/* 
   Blocks legend: P - pal idx, I - reused pixel idx in blk
       ---------   ---------   ---------   ---------
	  | P0 | I0 | | P0 | P1 | | P0 | P1 | | P0 | I0 |
	   ---------   ---------   ---------   ---------
	  | I0 | I0 | | I1 | I1 | | I0 | I0 | | P1 | I0 |
       ---------   ---------   ---------   --------- 
      | P0 | I0 | | P0 | P1 | | P0 | I0 | | P0 | P1 |
       ---------   ---------   ---------   ---------
      | I0 | P1 | | I0 | I1 | | P1 | I2 | | I1 | I0 |
       ---------   ---------   ---------   --------- 
      | P0 | P1 | | P0 | I0 | | P0 | P1 | | P0 | P1 |
       ---------   ---------   ---------   ---------
      | I0 | P2 | | P1 | P2 | | P2 | I0 | | P2 | I1 |
       ---------   ---------   ---------   --------- 
      | P0 | P1 | | P0 | P1 | | P0 | P1 | | TR\|/AN |
       ---------   ---------   ---------   -UNUSED!-
      | P2 | I2 | | I1 | P2 | | P2 | P3 | | SP/|\BK |
       ---------   ---------   ---------   ---------     
*/

uint8_t RLS_Common_BkIdx[16] =
{
	0b00000000, 0b00010101, 0b00010000, 0b00000100,
	0b00000001, 0b00010001, 0b00000110, 0b00010100,
	0b00010010, 0b00000110, 0b00011000, 0b00011001,
	0b00011010, 0b00010110, 0b00011011, 0b11111111
};

uint8_t RLS_Common_PalBits[16] =
{
	0b1000, 0b1100, 0b1100, 0b1010,
	0b1001, 0b1100, 0b1010, 0b1100,
	0b1101, 0b1011, 0b1110, 0b1110,
	0b1110, 0b1101, 0b1111, 0b1111
};

/*
**----------------------------------------------------------------------------
**  Internal variables
**----------------------------------------------------------------------------
*/

/*
**----------------------------------------------------------------------------
**  Function(internal use only) Declarations
**----------------------------------------------------------------------------
*/

/*
**----------------------------------------------------------------------------
**  Function(external use only) Declarations
**----------------------------------------------------------------------------
*/

/*
** ---------------------------------------------------------------------------
**
** Function:
**     RLS_Common_GetInfo
**
** Description:
**     Gets info of image container
**
** Input:
**     pData - Source data
**     pnFrames - Frame count pointer
**     pnWidth - Width pointer
**     pnHeight - Height pointer
**     pnPixBytes - Pixel bytes pointer
**
** Output:
**     Set values
**
** Return value:
**     12 (if data is valid)/0
**
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 08/23/2024	raulmrio28-git	Initial version
** ---------------------------------------------------------------------------
*/

int RLS_Common_GetInfo(uint8_t* pData, int* pnFrames, int* pnWidth,
					   int* pnHeight, int* pnPixBytes)
{
	RLSBaseHeader_T* ptHeader = (RLSBaseHeader_T*)pData;
	if (ptHeader->wMagic == RLS_MAGIC)
	{
		*pnFrames = ptHeader->nFrames;
		if (ptHeader->wVersion = 0x1210)
		{
			RLSInfoHeader12_T* ptInfo =	(RLSInfoHeader12_T*)
										(pData+sizeof(RLSBaseHeader_T));
			if (pnWidth)
				*pnWidth = ptInfo->nWidth;
			if (pnHeight)
				*pnHeight = ptInfo->nHeight;
			if (pnPixBytes)
				*pnPixBytes = ptInfo->nPixBytes;
		}
		else if (ptHeader->wVersion = 0x1013)
		{
			RLSInfoHeader13_T* ptInfo = (RLSInfoHeader13_T*)
										(pData+sizeof(RLSBaseHeader_T));
			if (pnWidth)
				*pnWidth = ptInfo->nWidth;
			if (pnHeight)
				*pnHeight = ptInfo->nHeight;
			if (pnPixBytes)
				*pnPixBytes = ptInfo->nPixBytes;
		}
		if (pnPixBytes && *pnPixBytes != RLS_PAL_BYTES)
			return 0;
		return 12;
	}
	return 0;
}

/*
** ---------------------------------------------------------------------------
**
** Function:
**     RLS_Common_MakeInfo
**
** Description:
**     Make info of image container
**
** Input:
**     pData - Destination data
**     nFrames - Frame count
**     nWidth - Width
**     nHeight - Height
**
** Output:
**     Written values
**
** Return value:
**     true/false
**
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 08/26/2024	raulmrio28-git	Initial version
** ---------------------------------------------------------------------------
*/

bool RLS_Common_MakeInfo(uint8_t* pData, int nFrames, int nWidth, int nHeight,
						 int nSavings, bool bReserved)
{
	RLSBaseHeader_T tHeader;
	if (!pData || nFrames <= 0 || nWidth <= 0 || nHeight <= 0)
		return false;
	tHeader.wMagic = RLS_MAGIC;
	if (nWidth > UINT8_MAX || nHeight > UINT8_MAX)
		tHeader.wVersion = 0x1013;
	else
		tHeader.wVersion = 0x1210;
	tHeader.nFrames = nFrames;
	memcpy(pData, &tHeader, sizeof(RLSBaseHeader_T));
	if (tHeader.wVersion = 0x1210)
	{
		RLSInfoHeader12_T tInfo;
		tInfo.nWidth = nWidth;
		tInfo.nHeight = nHeight;
		tInfo.nPixBytes = RLS_PAL_BYTES;
		tInfo.tAttributes.nSavings = nSavings;
		tInfo.tAttributes.bWOdd = nWidth % 1;
		tInfo.tAttributes.bHOdd = nHeight % 1;
		tInfo.tAttributes.bReserved = bReserved;
		memcpy(pData+sizeof(RLSBaseHeader_T),&tInfo,sizeof(RLSInfoHeader12_T));
	}
	else if (tHeader.wVersion = 0x1013)
	{
		RLSInfoHeader13_T tInfo;
		tInfo.nWidth = nWidth;
		tInfo.nHeight = nHeight;
		tInfo.nPixBytes = RLS_PAL_BYTES;
		tInfo.tAttributes.nSavings = nSavings;
		tInfo.tAttributes.bWOdd = nWidth % 1;
		tInfo.tAttributes.bHOdd = nHeight % 1;
		memcpy(pData+sizeof(RLSBaseHeader_T),&tInfo,sizeof(RLSInfoHeader13_T));
	}
	return true;
}

/*
** ---------------------------------------------------------------------------
**
** Function:
**     RLS_Common_ExtractBlock
**
** Description:
**     Extracts a 2x2 block from an image
**
** Input:
**     pImg - Source image
**     nWidth - Image width
**     nHeight - Image height
**     nX - X position
**     nY - Y position
**
** Output:
**     Extracted block
**
** Return value:
**     true/false
**
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 08/23/2024	raulmrio28-git	Initial version
** ---------------------------------------------------------------------------
*/

bool RLS_Common_ExtractBlock(uint16_t* pImg, int nWidth, int nHeight, int nX,
							 int nY)
{
	if (!pImg || nX >= (nWidth >> 1) || nY >= (nHeight >> 1))
		return false;
	RLS_Common_Block[0] = pImg[nWidth * (nY << 1) + (nX << 1)];
	if ((nWidth&1 && (nWidth+1) == (nX<<1))
	 && (nHeight&1 && (nHeight+1) == (nY<<1)))
	{
		RLS_Common_Block[1] = RLS_Common_Block[2]
	  = RLS_Common_Block[3] = RLS_Common_Block[0];
	}
	else if (nWidth&1 && (nWidth+1) == (nX<<1))
	{
		RLS_Common_Block[1] = RLS_Common_Block[0];
		RLS_Common_Block[2] = pImg[nWidth * ((nY<<1) + 1) + (nX<<1)];
		RLS_Common_Block[3] = RLS_Common_Block[2];
	}
	else
	{
		RLS_Common_Block[1] = pImg[nWidth * (nY<<1) + ((nX<<1) + 1)];
		RLS_Common_Block[2] = pImg[nWidth * ((nY<<1) + 1) + (nX<<1)];
		RLS_Common_Block[3] = pImg[nWidth * ((nY<<1) + 1) + ((nX<<1) + 1)];
	}

	return true;
}

/*
** ---------------------------------------------------------------------------
**
** Function:
**     RLS_Common_WriteBlock
**
** Description:
**     Writess a 2x2 block to an image
**
** Input:
**     pImg - Dest image
**     nWidth - Image width
**     nHeight - Image height
**     nX - X position
**     nY - Y position
**
** Output:
**     Written block
**
** Return value:
**     true/false
**
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 08/23/2024	raulmrio28-git	Initial version
** ---------------------------------------------------------------------------
*/
bool RLS_Common_WriteBlock(uint16_t* pImg, int nWidth, int nHeight, int nX,
						   int nY)
{
	if (!pImg || nX >= (nWidth >> 1) || nY >= (nHeight >> 1))
		return false;
	pImg[nWidth * (nY<<1) + (nX<<1)] = RLS_Common_Block[0];
	if (nWidth&1 && (nWidth+1) == (nX<<1))
	{
		pImg[nWidth * ((nY<<1)+1) + (nX<<1)] = RLS_Common_Block[2];
	}
	else
	{
		pImg[nWidth * (nY<<1) + (nX<<1)+1] = RLS_Common_Block[1];
		pImg[nWidth * ((nY<<1)+1) + (nX<<1)] = RLS_Common_Block[2];
		pImg[nWidth * ((nY<<1)+1) + (nX<<1)+1] = RLS_Common_Block[3];
	}

	return true;
}
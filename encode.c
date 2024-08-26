/*
** ===========================================================================
** File: encode.c
** Description: ReakoLite library encoder code
** Copyright (c) 2024 raulmrio28-git.
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 08/25/2024	raulmrio28-git	Initial version
** ===========================================================================
*/

/*
**----------------------------------------------------------------------------
**  Includes
**----------------------------------------------------------------------------
*/

#define RLS_EXTERN_VAR
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

/*
**----------------------------------------------------------------------------
**  Definitions
**----------------------------------------------------------------------------
*/

#define RLS_BKI_PU_WB(v, i, n) (v |= ((i&1)<<(3-n)))
#define RLS_BKI_BI_WB(v, i, n) (v |= ((i&3)<<((3-n)<<1)))

#define RLS_COPY(d, s, sz) memcpy(d, s, sz), d+=sz
#define RLS_WRITESZ(d, n) \
		{ \
			uint32_t sz = n; \
			RLS_COPY(d, &sz, sizeof(sz)); \
		}

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
** ---------------------------------------------------------------------------
**
** Function:
**     RLS_Encode_MoveMem
**
** Description:
**     Move memory data to a specific offset in backwards order
**
** Input:
**     pData - input data
**     nSize - size of data
**     nOffset - destination offset
**
** Output:
**     Moved data, originally occupied portion filled with 0
**
** Return value:
**     none
**
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 08/26/2024	raulmrio28-git	Initial version
** ---------------------------------------------------------------------------
*/

void RLS_Encode_MoveMem(uint8_t* pData, int nSize, int nOffset)
{
	uint8_t* pCurrData = pData + nSize - 1;

	while (nSize--)
	{
		pCurrData[nOffset] = *pCurrData;
		*pCurrData = 0x00;
		pCurrData--;
	}
}

/*
** ---------------------------------------------------------------------------
**
** Function:
**     RLS_Encode_ColInSPal
**
** Description:
**     Checks for existence of a color in standard (256 color) palette
**
** Input:
**     wColor - RGB565 color
**
** Output:
**     Offset in standard palette
**
** Return value:
**     nOffset/RLS_SPAL_SIZE
**
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 08/25/2024	raulmrio28-git	Initial version
** ---------------------------------------------------------------------------
*/
uint16_t RLS_Encode_ColInSPal(uint16_t wColor)
{
	int nOffset;
	for (nOffset = 0; nOffset < RLS_SPAL_SIZE; nOffset++)
		if (wColor == RLS_Common_StdPal[nOffset])
			return nOffset;
	return RLS_SPAL_SIZE;
}

/*
** ---------------------------------------------------------------------------
**
** Function:
**     RLS_Encode_ColInBlk
**
** Description:
**     Checks for existence of a color in blk (e.g. alpha color)
**
** Input:
**     pIn - input data
**     wColor - RGB565 color
**
** Output:
**     true/false
**
** Return value:
**     true/false
**
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 08/25/2024	raulmrio28-git	Initial version
** ---------------------------------------------------------------------------
*/

bool RLS_Encode_ColInBlk(uint16_t* pIn, uint16_t wColor)
{
	int nBkPix;
	for (nBkPix = 0; nBkPix < 2*2; nBkPix++)
		if (wColor == pIn[nBkPix])
			return true;
	return false;
}

/*
** ---------------------------------------------------------------------------
**
** Function:
**     RLS_Encode_ColInRow
**
** Description:
**     Checks for existence of a color in row
**
** Input:
**     pIn - input data
**     wColor - RGB565 color
**     nSize - size of row
**
** Output:
**     true/false
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

bool RLS_Encode_ColInRow(uint16_t* pIn, uint16_t wColor, uint32_t nSize)
{
	int nRowPix;
	for (nRowPix = 0; nRowPix < nSize; nRowPix++)
		if (wColor == pIn[nRowPix])
			return true;
	return false;
}

/*
** ---------------------------------------------------------------------------
**
** Function:
**     RLS_Encode_MakeSPal_Sort
**
** Description:
**     Sort row items in ascending order (bubble sort)
**     src: https://www.geeksforgeeks.org/c-program-to-sort-an-array-in-
**          ascending-order
**
** Input:
**     pRow - row
**     nSize - row size
**
** Output:
**     Roted items
**
** Return value:
**     none
**
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 08/26/2024	raulmrio28-git	Initial version
** ---------------------------------------------------------------------------
*/

void RLS_Encode_MakeSPal_Sort(uint16_t* pRow, int nSize) {
	int i, j;
	for (i = 0; i < nSize - 1; i++) {
		for (j = 0; j < nSize - i - 1; j++) {

			if (pRow[j] > pRow[j + 1]) {
				uint16_t wTemp = pRow[j];
				pRow[j] = pRow[j + 1];
				pRow[j + 1] = wTemp;
			}
		}
	}
}

/*
** ---------------------------------------------------------------------------
**
** Function:
**     RLS_Encode_MakeSPal_Undup
**
** Description:
**     Remove duplicate row items (sort of like an LZ compressor without
**     actually compressing, just removing duplicate items)
**     src: https://www.geeksforgeeks.org/c-program-to-remove-duplicates-
**          from-sorted-array
**
** Input:
**     pRow - row
**     pnSize - row size
**
** Output:
**     Duplicate items removed
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
void RLS_Encode_MakeSPal_Undup(uint16_t* pRow, int* pnSize) {
	uint16_t* pTmpRow;
	int i,j = 0;

	if (*pnSize == 0 || *pnSize == 1)
		return;

	pTmpRow = (uint16_t*)malloc(*pnSize << 1);
	if (!pTmpRow)
		return false;

	for (i = 0; i < *pnSize - 1; i++)
		if (pRow[i] != pRow[i + 1] && !RLS_Encode_ColInBlk(pRow, pRow[i]))
			pTmpRow[j++] = pRow[i];

	pTmpRow[j++] = pRow[*pnSize - 1];

	memcpy(pRow, pTmpRow, j << 1);
	free(pTmpRow);
	*pnSize = j;
}

/*
** ---------------------------------------------------------------------------
**
** Function:
**     RLS_Encode_MakeSPal
**
** Description:
**     Make standard palette from image
**
** Input:
**     pIn - input data
**     nWidth - image width
**     nHeight - image height
**
** Output:
**     Built palette
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

bool RLS_Encode_MakeSPal(uint16_t* pIn, int nWidth, int nHeight)
{
	uint16_t* pTmpRow;
	int nCols = RLS_CEIL(nWidth, 2);
	int nRows = RLS_CEIL(nHeight, 2);
	int nCurrCol, nCurrRow;
	int nSize;
	int nPalOffset = 0;
	if (!pIn)
		return 0;

	pTmpRow = (uint16_t*)malloc(nCols << 1);
	if (!pTmpRow)
		return false;

	for (nCurrRow = 0; nCurrRow < nRows; nCurrRow++)
	{
		nSize = 0;
		for (nCurrCol = 0; nCurrCol < nCols; nCurrCol++)
		{
			if (RLS_Common_ExtractBlock(pIn, nWidth,
				nHeight, nCurrCol, nCurrRow) == false)
				return 0;
			pTmpRow[nSize++] = RLS_Common_Block[0];
		}
		RLS_Encode_MakeSPal_Sort(pTmpRow, nSize);
		RLS_Encode_MakeSPal_Undup(pTmpRow, &nSize);
		memcpy(&RLS_Common_StdPal[nPalOffset], pTmpRow, ((nSize < RLS_SPAL_SIZE)
			? nSize : (RLS_SPAL_SIZE-nPalOffset)) << 1);
		nPalOffset += ((nSize < RLS_SPAL_SIZE)
					? nSize : (RLS_SPAL_SIZE - nPalOffset));
		if (nPalOffset >= RLS_SPAL_SIZE)
			break;
	}

	free(pTmpRow);
	return true;
}

/*
** ---------------------------------------------------------------------------
**
** Function:
**     RLS_Encode_EncodeBlk
**
** Description:
**     Encode a block from input data
**
** Input:
**     pIn - input data
**     bAlpha - alpha flag
**     wAlpha - alpha color
**     pOut - output data
**
** Output:
**     Encoded block to pOut
**
** Return value:
**     sizeof(RLSBkInfo_T) + nSPalItems
**
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 08/25/2024	raulmrio28-git	Initial version
** ---------------------------------------------------------------------------
*/

uint32_t RLS_Encode_EncodeBlk(uint16_t* pIn, bool bAlpha, uint16_t wAlpha,
							  uint8_t* pOut)
{
	RLSBkInfo_T tBkInfo;
	uint8_t baBkIdx = 0b00000000;
	uint8_t baPalBits = 0b0000;
	uint8_t bMpalIdxs[2*2] = { 0, 0, 0, 0 };
	uint8_t nBkPix;
	uint8_t nPalCols = 0;
	uint8_t nSPalItems = 0;
	tBkInfo.baPalBits = 0b0000;
	if (bAlpha == true && RLS_Encode_ColInBlk(pIn, wAlpha) == true)
	{
		for (nBkPix = 0; nBkPix < 2 * 2; nBkPix++)
		{
			if (pIn[nBkPix] == wAlpha)
			{
				RLS_BKI_PU_WB(tBkInfo.baPalBits, RLS_BKI_PAL_SP, nBkPix);
			}
			else
			{
				RLS_BKI_PU_WB(tBkInfo.baPalBits, RLS_BKI_PAL_EP, nBkPix);
				RLS_Common_ExtPal[RLS_Common_ExtPal_CIdx++] = pIn[nBkPix];
			}
		}
		tBkInfo.nPbIdx = 0xf;
	}
	else
	{
		for (nBkPix = 0; nBkPix < 2 * 2; nBkPix++)
		{
			uint8_t nBkIdx;
			bool bBkReuse = false;
			for (nBkIdx = 0; nBkPix > 0 && nBkIdx < nBkPix; nBkIdx++)
			{
				if (pIn[nBkPix] == pIn[nBkIdx])
				{
					bBkReuse = true;
					RLS_BKI_BI_WB(baBkIdx, nBkIdx, nBkPix);
					RLS_BKI_PU_WB(baPalBits, RLS_BKI_PU_USEB, nBkPix);
					RLS_BKI_PU_WB(tBkInfo.baPalBits, RLS_BKI_PAL_EP, nBkPix);
					break;
				}
			}
			if (bBkReuse == false)
			{
				uint16_t nMpalIdx = RLS_Encode_ColInSPal(pIn[nBkPix]);
				RLS_BKI_BI_WB(baBkIdx, nPalCols, nBkPix);
				RLS_BKI_PU_WB(baPalBits, RLS_BKI_PU_USEP, nBkPix);
				if (nMpalIdx < RLS_SPAL_SIZE)
				{
					RLS_BKI_PU_WB(tBkInfo.baPalBits, RLS_BKI_PAL_SP, nBkPix);
					bMpalIdxs[nSPalItems++] = (uint8_t)nMpalIdx;
				}
				else
				{
					RLS_BKI_PU_WB(tBkInfo.baPalBits, RLS_BKI_PAL_EP, nBkPix);
					RLS_Common_ExtPal[RLS_Common_ExtPal_CIdx++]=pIn[nBkPix];
				}
				nPalCols++;
			}
		}
		for (tBkInfo.nPbIdx = 0; tBkInfo.nPbIdx < 15; tBkInfo.nPbIdx++)
		{
			if (RLS_Common_PalBits[tBkInfo.nPbIdx] == baPalBits
				&& RLS_Common_BkIdx[tBkInfo.nPbIdx] == baBkIdx)
				break;
		}
	}
	memcpy(pOut, &tBkInfo, sizeof(RLSBkInfo_T));
	memcpy(pOut + sizeof(RLSBkInfo_T), bMpalIdxs, nSPalItems);
	return sizeof(RLSBkInfo_T) + nSPalItems;
}

/*
**----------------------------------------------------------------------------
**  Function(external use only) Declarations
**----------------------------------------------------------------------------
*/

/*
** ---------------------------------------------------------------------------
**
** Function:
**     RLS_Encode
**
** Description:
**     Encode an image from input data
**
** Input:
**     pIn - input data
**     pOut - output data
**     bAlpha - alpha flag
**     wAlpha - alpha color
**     nWidth - image width
**     nHeight - image height
**
** Output:
**     Encoded image to pOut
**
** Return value:
**     nDataOffs+nDataSize
**
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 08/25/2024	raulmrio28-git	Initial version
** ---------------------------------------------------------------------------
*/

uint32_t RLS_Encode(uint16_t* pIn,uint8_t* pOut,bool bAlpha,uint16_t wAlpha,
					int nWidth, int nHeight)
{
	uint8_t* pCurrOutput = pOut;
	uint8_t* pWriteOutput = pOut;
	int nDataOffs = 0;
	int nDataSize = 0;
	int nCols = RLS_CEIL(nWidth, 2);
	int nRows = RLS_CEIL(nHeight, 2);
	int nCurrCol, nCurrRow;
	bool doCalc = true;

	if (!pOut)
		return 0;
	RLS_Encode_MakeSPal(pIn, nWidth, nHeight);
	RLS_Common_ExtPal_CIdx = 0;
 	for (nCurrRow = 0; nCurrRow < nRows; nCurrRow++)
	{
		for (nCurrCol = 0; nCurrCol < nCols; nCurrCol++)
		{
			int nBkSize;
			if (RLS_Common_ExtractBlock(pIn, nWidth,
				nHeight, nCurrCol, nCurrRow) == false)
				return 0;
			nBkSize = RLS_Encode_EncodeBlk(RLS_Common_Block, bAlpha,
										   wAlpha, pCurrOutput);
			nDataSize += nBkSize;
			pCurrOutput += nBkSize;
		}
	}
	nDataOffs = (RLS_SPAL_SIZE + RLS_Common_ExtPal_CIdx)*RLS_PAL_BYTES
			  + 2*sizeof(uint32_t);
	RLS_Encode_MoveMem(pWriteOutput, nDataSize, nDataOffs);
	RLS_COPY(pWriteOutput, RLS_Common_StdPal, RLS_PAL_BYTES * RLS_SPAL_SIZE);
	RLS_WRITESZ(pWriteOutput, RLS_PAL_BYTES*RLS_Common_ExtPal_CIdx);
	RLS_COPY(pWriteOutput, RLS_Common_ExtPal,
			 RLS_PAL_BYTES * RLS_Common_ExtPal_CIdx);
	RLS_WRITESZ(pWriteOutput, nDataSize);
	return nDataOffs+nDataSize;
}
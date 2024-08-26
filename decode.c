/*
** ===========================================================================
** File: decode.c
** Description: ReakoLite library decoder code
** Copyright (c) 2024 raulmrio28-git.
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 08/23/2024	raulmrio28-git	Initial version
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
#include <memory.h>

/*
**----------------------------------------------------------------------------
**  Definitions
**----------------------------------------------------------------------------
*/

#define RLS_BKI_PU_GB(v, i) ((v>>(3-i))&1)
#define RLS_BKI_BI_GB(v, i) ((v>>((3-i)<<1))&3)

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
**     RLS_Decode_DecodeBlk
**
** Description:
**     Decode a block from input data
**
** Input:
**     pIn - input data
**     pOut - output data
**
** Output:
**     Decoded block to pOut
**
** Return value:
**     nOffset
**
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 08/23/2024	raulmrio28-git	Initial version
** ---------------------------------------------------------------------------
*/

uint32_t RLS_Decode_DecodeBlk(uint8_t* pIn, uint16_t* pOut)
{
	RLSBkInfo_T tBkInfo;
	uint8_t nBkPix;
	uint32_t nOffset = 0;
	memcpy(&tBkInfo, &pIn[nOffset++], sizeof(RLSBkInfo_T));
	if (tBkInfo.nPbIdx == 0xf) {
		for (nBkPix = 0; nBkPix < 2*2; nBkPix++)
		{
			if (RLS_BKI_PU_GB(tBkInfo.baPalBits, nBkPix) == RLS_BKI_PAL_EP)
				RLS_Common_Block[nBkPix]
			  = RLS_Common_ExtPal[RLS_Common_ExtPal_CIdx++];
		}
	}
	else {
		uint8_t pal_bits = RLS_Common_PalBits[tBkInfo.nPbIdx];
		uint8_t bk_idx = RLS_Common_BkIdx[tBkInfo.nPbIdx];
		for (nBkPix = 0; nBkPix < 2*2; nBkPix++)
		{
			if (RLS_BKI_PU_GB(pal_bits, nBkPix)==RLS_BKI_PU_USEB && nBkPix>0)
				RLS_Common_Block[nBkPix]
			   =RLS_Common_Block[RLS_BKI_BI_GB(bk_idx,nBkPix)];
			else
			{
				if (RLS_BKI_PU_GB(tBkInfo.baPalBits, nBkPix)==RLS_BKI_PAL_SP)
					RLS_Common_Block[nBkPix]
				  = RLS_Common_StdPal[pIn[nOffset++]];
				else
					RLS_Common_Block[nBkPix]
				  = RLS_Common_ExtPal[RLS_Common_ExtPal_CIdx++];

			}
		}
	}
	return nOffset;
}

/*
** ---------------------------------------------------------------------------
**
** Function:
**     RLS_Decode_Frame
**
** Description:
**     Decode a frame from input data
**
** Input:
**     pIn - input data
**     pOut - output data
**     nWidth - width
**     nHeight - height
**
** Output:
**     Decoded frame to pOut
**
** Return value:
**     pCurrInput - pIn
**
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 08/23/2024	raulmrio28-git	Initial version
** ---------------------------------------------------------------------------
*/
uint32_t RLS_Decode_Frame(uint8_t* pIn,uint16_t* pOut,int nWidth,int nHeight)
{
	uint8_t* pCurrInput = pIn;
	uint8_t* pInputEnd;
	bool bNoWrite = false;
	int nCols = RLS_CEIL(nWidth, 2);
	int nRows = RLS_CEIL(nHeight, 2);
	int nCurrCol, nCurrRow;

	if (!pOut)
		bNoWrite = true;
	memcpy(RLS_Common_StdPal, pCurrInput, RLS_SPAL_SIZE * RLS_PAL_BYTES);
	pCurrInput += RLS_SPAL_SIZE * RLS_PAL_BYTES;
	if (*(uint32_t*)pCurrInput > RLS_EPAL_SIZE * RLS_PAL_BYTES)
		return 0;
	memcpy(RLS_Common_ExtPal, pCurrInput + sizeof(uint32_t),
		   *(uint32_t*)pCurrInput);
	RLS_Common_ExtPal_CIdx = 0;
	pCurrInput += *(uint32_t*)pCurrInput + sizeof(uint32_t);
	pInputEnd = pCurrInput + *(uint32_t*)pCurrInput + sizeof(uint32_t);
	pCurrInput += sizeof(uint32_t);
	while (pCurrInput < pInputEnd)
	{
		for (nCurrRow = 0; nCurrRow < nRows; nCurrRow++)
		{
			for (nCurrCol = 0; nCurrCol < nCols; nCurrCol++)
			{
				if (bNoWrite == false && RLS_Common_ExtractBlock(pOut, nWidth,
					nHeight, nCurrCol, nCurrRow) == false)
					return 0;
				pCurrInput += RLS_Decode_DecodeBlk(pCurrInput,
												   RLS_Common_Block);
				if (bNoWrite == false && RLS_Common_WriteBlock(pOut, nWidth,
					nHeight, nCurrCol, nCurrRow) == false)
					return 0;
			}
		}
	}
	return (uint32_t)(pCurrInput - pIn);
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
**     RLS_Decode
**
** Description:
**     Decode a frame from container
**
** Input:
**     pIn - input data
**     nFrame - frame to decode
**     pOut - output data
**
** Output:
**     Decoded frame to pOut
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

bool RLS_Decode(uint8_t* pIn, int nFrame, uint16_t* pOut)
{
	uint8_t* pCurrInput = pIn;

	if (*(uint16_t*)pCurrInput == RLS_MAGIC) {
		int nSkipFrames;
		int nStart;
		int nFrames;
		int nWidth;
		int nHeight;
		nStart = RLS_Common_GetInfo(pCurrInput, &nFrames, &nWidth,
									&nHeight, NULL);
		if (nStart == -1) /* Invalid information */
			return false;
		pCurrInput+=nStart;
		if (nFrame >= nFrames)
			return false;
		for (nSkipFrames = 0; nSkipFrames < nFrame-1; nSkipFrames++)
			pCurrInput+=RLS_Decode_Frame(pCurrInput, NULL, nWidth, nHeight);
		if (RLS_Decode_Frame(pCurrInput, pOut, nWidth, nHeight))
			return true;
	}
	return false;
}
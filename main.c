/*
** ===========================================================================
** File: main.c
** Description: Program main code
** Copyright (c) 2024 raulmrio28-git.
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 08/26/2024	raulmrio28-git	Add encode support (PNG only!)
** 08/25/2024	raulmrio28-git	Initial version
** ===========================================================================
*/

/*
**----------------------------------------------------------------------------
**  Includes
**----------------------------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "common.h"
#include "convert.h"
#include "decode.h"
#include "encode.h"

/*
**----------------------------------------------------------------------------
**  Definitions
**----------------------------------------------------------------------------
*/

#define RLS_ENCODE_BSIZE(nWidth, nHeight) \
	(2*sizeof(uint32_t)+(RLS_EPAL_SIZE*(RLS_PAL_BYTES+RLS_SPAL_SIZE)) \
	+((nWidth*nHeight*5)/4))

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

extern void RLS_Quantize(RGB565_T* pImg, int nWidth, int nHeight);

const char* FileExt(const char* pszFn)
{
	const char* pszFnSrch = pszFn + strlen(pszFn) - 1;
	while (pszFnSrch >= pszFn)
	{
		if (*pszFnSrch == '.')
			return pszFnSrch + 1;
		pszFnSrch--;
	}
	return NULL;
}

int main(int argc, char* argv[])
{
	if (argc >= 2)
	{
		char szFn[256];
		if (strcmp(argv[1], "-d") == 0)
		{
			FILE* pFile = fopen(argv[2], "rb");
			uint8_t* pData;
			uint16_t* pDec;
			int nSize;
			int nWidth = 0, nHeight = 0, nFrames = 0;
			int nCurrFrame;
			if (!pFile)
			{
				printf("Failed to open file %s\n", argv[2]);
				return 1;
			}
			fseek(pFile, 0, SEEK_END);
			nSize = ftell(pFile);
			fseek(pFile, 0, SEEK_SET);
			pData = (uint8_t*)malloc(nSize);
			if (!pData)
			{
				printf("Failed to allocate memory\n");
				return 1;
			}
			fread(pData, 1, nSize, pFile);
			fclose(pFile);
			RLS_Common_GetInfo(pData, &nFrames, &nWidth, &nHeight, NULL);
			if (nFrames == 0 || nWidth == 0 || nHeight == 0)
			{
				printf("Failed to get info from file %s\n", argv[2]);
				return 1;
			}
			printf("Width: %d, Height: %d, Frames: %d\n",
				   nWidth,nHeight,nFrames);
			
			pDec = (uint16_t*)malloc(nWidth * nHeight * sizeof(uint16_t));
			if (!pDec)
			{
				printf("Failed to allocate memory\n");
				return 1;
			}
			for (nCurrFrame = 0; nCurrFrame < nFrames; nCurrFrame++)
			{
				if (RLS_Decode(pData, nCurrFrame, pDec) == false)
				{
					printf("Failed to decode frame %d\n", nCurrFrame);
					return 1;
				}
				sprintf(szFn, "%s_%d.png", argv[2], nCurrFrame);
				if (RLS_Convert_565toPNG(pDec,szFn,nWidth,nHeight)==false)
				{
					printf("Failed to convert frame %d\n", nCurrFrame);
					return 1;
				}
			}
			free(pDec);
		}
		else if (strcmp(argv[1], "-e") == 0)
		{
			FILE* pFile;
			uint8_t* pData;
			uint8_t* pEnc;
			uint16_t* pDec;
			uint8_t* pCurrEnc;
			int nSize;
			int nWidth = 0, nHeight = 0;
			int nFrames = argc - 3;
			int nCurrFrame;
			int nSavingCalcSize = 0;
			if (_stricmp(FileExt(argv[3]), "png") != 0)
			{
				printf("File %s is not a PNG file\n", argv[3]);
				return 1;
			}
			pDec = RLS_Convert_PNGto565(argv[3], &nWidth, &nHeight);
			if (!pDec)
			{
				printf("Failed to convert PNG file %s\n", argv[3]);
				return 1;
			}
			pEnc=(uint8_t*)malloc(12+(nFrames*RLS_ENCODE_BSIZE(nWidth,nHeight)));
			if (!pEnc)
			{
				printf("Failed to allocate memory\n");
				return 1;
			}
			pCurrEnc = pEnc + 12;
			for (nCurrFrame = 0; nCurrFrame < nFrames; nCurrFrame++)
			{
				int nSize;
				int nVW = nWidth, nVH = nHeight;
				RLS_Quantize(pDec, nWidth, nHeight);
				nSize = RLS_Encode(pDec, pCurrEnc, false, 0, nWidth, nHeight);
				pCurrEnc += nSize;
				nSavingCalcSize += nSize-(2*sizeof(uint32_t));
				free(pDec);
				if (nCurrFrame == nFrames - 1)
					break;
				if (_stricmp(FileExt(argv[4+nCurrFrame]), "png") != 0)
				{
					printf("File %s is not a PNG file\n", argv[4+nCurrFrame]);
					return 1;
				}
				pDec = RLS_Convert_PNGto565(argv[4+nCurrFrame], &nWidth, &nHeight);
				if (!pDec)
				{
					printf("Failed to convert PNG file %s\n",argv[4+nCurrFrame]);
					return 1;
				}
				if (nVW != nWidth || nVH != nHeight)
				{
					printf("%s has different dimensions\n",argv[4+nCurrFrame]);
					return 1;
				}
			}
			RLS_Common_MakeInfo(pEnc, nFrames, nWidth, nHeight,
			RLS_CALC_SAVING((nWidth*nHeight*nFrames), nSavingCalcSize), false);
			pFile = fopen(argv[2], "wb");
			if (!pFile)
			{
				printf("Failed to open file %s\n", argv[2]);
				return 1;
			}
			fwrite(pEnc, 1, pCurrEnc-pEnc, pFile);
			fclose(pFile);
			free(pEnc);
		}
		else
		{
			printf("Usage: %s -d/e <input>\n", argv[0]);
			return 1;
		}
		return 0;
	}
}
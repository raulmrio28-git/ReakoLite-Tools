/*
** ===========================================================================
** File: main.c
** Description: Program main code
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "common.h"
#include "convert.h"
#include "decode.h"

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
		else
		{
			printf("Usage: %s -d/e <input>\n", argv[0]);
			return 1;
		}
		return 0;
	}
}
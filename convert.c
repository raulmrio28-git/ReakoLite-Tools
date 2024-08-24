/*
** ===========================================================================
** File: convert.c
** Description: ReakoLite library conversion coode
** Copyright (c) 2024 raulmrio28-git.
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 08/24/2024	raulmrio28-git	Initial version
** ===========================================================================
*/

/*
**----------------------------------------------------------------------------
**  Includes
**----------------------------------------------------------------------------
*/

#include "convert.h"
#include "spng/spng.h"

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

/*
**----------------------------------------------------------------------------
**  Function(internal and external use) Declarations
**----------------------------------------------------------------------------
*/

/*
** ---------------------------------------------------------------------------
**
** Function:
**     RLS_Convert_565to888
**
** Description:
**     Convert an RGB565 image to RGB888
**
** pData:
**     pSrc - Source image
**     pDest - Destination image
**     nWidth - Width
**     nHeight - Height
**
** Output:
**     Converted image
**
** Return value:
**     true/false
**
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 08/24/2024	raulmrio28-git	Initial version
** ---------------------------------------------------------------------------
*/

bool RLS_Convert_565to888(RGB565_T* pSrc,RGB888_T* pDest,int nWidth,
						  int nHeight)
{
	int nSize;
	int nCurrSize;
	if (!pSrc || !pDest || nWidth <= 0 || nHeight <= 0)
		return false;
	nSize = nWidth * nHeight;
	for (nCurrSize = 0; nCurrSize < nSize; nCurrSize++)
	{
		pDest[nCurrSize].b = CVT_16BPP_EX_B(pSrc[nCurrSize]);
		pDest[nCurrSize].g = CVT_16BPP_EX_G(pSrc[nCurrSize]);
		pDest[nCurrSize].r = CVT_16BPP_EX_R(pSrc[nCurrSize]);
	}
	return true;
}

/*
** ---------------------------------------------------------------------------
**
** Function:
**     RLS_Convert_888to565
**
** Description:
**     Convert an RGB888 image to RGB565
**
** pData:
**     pSrc - Source image
**     pDest - Destination image
**     nWidth - Width
**     nHeight - Height
**
** Output:
**     Converted image
**
** Return value:
**     true/false
**
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 08/24/2024	raulmrio28-git	Initial version
** ---------------------------------------------------------------------------
*/

bool RLS_Convert_888to565(RGB888_T* pSrc,RGB565_T* pDest,int nWidth,
						  int nHeight)
{
	int nSize;
	int nCurrSize;
	if (!pSrc || !pDest || nWidth <= 0 || nHeight <= 0)
		return false;
	nSize = nWidth * nHeight;
	for (nCurrSize = 0; nCurrSize < nSize; nCurrSize++)
		pDest[nCurrSize] = ((pSrc[nCurrSize].b >> 3) << 11)
					   | ((pSrc[nCurrSize].g >> 2) << 5)
					   | (pSrc[nCurrSize].r >> 3);
	return true;
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
**     RLS_Convert_565toPNG
**
** Description:
**     Convert an RGB565 image to a PNG file
**
** pData:
**     pszFn - File name
**     pSrc - Source image
**     nWidth - Width
**     nHeight - Height
**
** Output:
**     Converted image
**
** Return value:
**     true/false
**
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 08/24/2024	raulmrio28-git	Initial version
** ---------------------------------------------------------------------------
*/
bool RLS_Convert_565toPNG(RGB565_T* pImg, const char* pszFn, int nWidth,
						  int nHeight)
{
	int iSPNGResult;
	void* pPNGBuff;
	size_t nPNGSize;
	FILE* pFile;
	spng_ctx* ptPNGCtx = NULL;
	struct spng_ihdr tPNGIHDR = { 0 }; /* zero-init to set valid defaults */
	RGB888_T* cvt_buff;

	tPNGIHDR.width = nWidth;
	tPNGIHDR.height = nHeight;
	tPNGIHDR.bit_depth = 8;
	tPNGIHDR.color_type = SPNG_COLOR_TYPE_TRUECOLOR;

	ptPNGCtx = spng_ctx_new(SPNG_CTX_ENCODER);
	spng_set_option(ptPNGCtx, SPNG_IMG_COMPRESSION_LEVEL, 9);
	spng_set_option(ptPNGCtx, SPNG_ENCODE_TO_BUFFER, 1);
	spng_set_ihdr(ptPNGCtx, &tPNGIHDR);

	cvt_buff = (RGB888_T*)malloc(nWidth * nHeight * sizeof(RGB888_T));
	if (!cvt_buff)
		return false;
	if (RLS_Convert_565to888(pImg, cvt_buff, nWidth, nHeight) == false)
		return false;
	iSPNGResult = spng_encode_image(ptPNGCtx, cvt_buff,
									nWidth * nHeight * sizeof(RGB888_T),
								    SPNG_FMT_PNG, SPNG_ENCODE_FINALIZE);

	if (iSPNGResult) {
		spng_ctx_free(ptPNGCtx);
		return false;
	}

	pPNGBuff = spng_get_png_buffer(ptPNGCtx, &nPNGSize, &iSPNGResult);

	if (pPNGBuff == NULL)
	{
		spng_ctx_free(ptPNGCtx);
		return false;
	}

	pFile = fopen(pszFn, "wb");
	if (pFile == NULL) {
		free(pPNGBuff);
		spng_ctx_free(ptPNGCtx);
		return false;
	}

	fwrite(pPNGBuff, nPNGSize, 1, pFile);
	fclose(pFile);
	free(pPNGBuff);
	spng_ctx_free(ptPNGCtx);
	return true;
}
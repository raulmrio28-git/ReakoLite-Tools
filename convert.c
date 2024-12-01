/*
** ===========================================================================
** File: convert.c
** Description: ReakoLite library conversion coode
** Copyright (c) 2024 raulmrio28-git.
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 08/26/2024	raulmrio28-git	PNG to RGB565
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
** Input:
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
** Input:
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
** Input:
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
	int iSPNGResult = 0;
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

/*
** ---------------------------------------------------------------------------
**
** Function:
**     RLS_Convert_PNGto565
**
** Description:
**     Convert a PNG file to an RGB565 image
**
** Input:
**     pszFn - File name
**     pnWidth - Width
**     pnHeight - Height
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
** 08/26/2024	raulmrio28-git	Initial version
** ---------------------------------------------------------------------------
*/

RGB565_T* RLS_Convert_PNGto565(const char* pszFn, int* pnWidth, int* pnHeight)
{
	int iSPNGResult = 0;
	uint8_t* pPNGBuff;
	size_t nPNGSize;
	FILE* pFile;
	spng_ctx* ptPNGCtx = NULL;
	struct spng_ihdr tPNGIHDR = { 0 }; /* zero-init to set valid defaults */
	RGB888_T* tmp_sbuff;
	RGB565_T* cvt_buff;

	pFile = fopen(pszFn, "rb");
	if (pFile == NULL) {
		return false;
	}
	fseek(pFile, 0, SEEK_END);
	nPNGSize = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);
	pPNGBuff = (uint8_t*)malloc(nPNGSize);
	if (pPNGBuff == NULL) {
		fclose(pFile);
		return false;
	}
	fread(pPNGBuff, nPNGSize, 1, pFile);
	fclose(pFile);

	ptPNGCtx = spng_ctx_new(0);

	if (!ptPNGCtx)
		return NULL;

	spng_set_png_buffer(ptPNGCtx, pPNGBuff, nPNGSize);

	//if (iSPNGResult) {
	//	spng_ctx_free(ptPNGCtx);
	//	return NULL;
	//}

	if(spng_get_ihdr(ptPNGCtx, &tPNGIHDR))
	{
		spng_ctx_free(ptPNGCtx);
		return NULL;
	}

	*pnWidth = tPNGIHDR.width;
	*pnHeight = tPNGIHDR.height;

	tmp_sbuff = (RGB888_T*)malloc(tPNGIHDR.width * tPNGIHDR.height
							    * sizeof(RGB888_T));
	if (!tmp_sbuff)
		return NULL;
	if (spng_decode_image(ptPNGCtx, tmp_sbuff,
						  tPNGIHDR.width * tPNGIHDR.height
						* sizeof(RGB888_T), SPNG_FMT_RGB8, 0))
		return NULL;
	cvt_buff = (RGB565_T*)malloc(tPNGIHDR.width * tPNGIHDR.height
							    * sizeof(RGB565_T));
	if (!cvt_buff)
		return NULL;
	if (RLS_Convert_888to565(tmp_sbuff, cvt_buff,
							 tPNGIHDR.width,
							 tPNGIHDR.height) == false)
		return NULL;
	free(tmp_sbuff);
	spng_ctx_free(ptPNGCtx);
	return cvt_buff;
}
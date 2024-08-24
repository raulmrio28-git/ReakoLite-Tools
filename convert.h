/*
** ===========================================================================
** File: common.h
** Description: ReakoLite library common header
** Copyright (c) 2024 raulmrio28-git.
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 08/23/2024	raulmrio28-git	Initial version
** ===========================================================================
*/

#ifndef RLS_CONVERT_H
#define RLS_CONVERT_H

/*
**----------------------------------------------------------------------------
**  Includes
**----------------------------------------------------------------------------
*/

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
**----------------------------------------------------------------------------
**  Definitions
**----------------------------------------------------------------------------
*/

#define CVT_16BPP_EX_B(x) ((x>>11) << 3)
#define CVT_16BPP_EX_G(x) (((x>>5)&0x3f) << 2)
#define CVT_16BPP_EX_R(x) ((x&0x1f) << 3)

/*
**----------------------------------------------------------------------------
**  Type Definitions
**----------------------------------------------------------------------------
*/

typedef uint16_t RGB565_T;
typedef struct tagRGB888_T RGB888_T;

typedef struct tagRGB888_T
{
	uint8_t b;
	uint8_t g;
	uint8_t r;
};

/*
**----------------------------------------------------------------------------
**  Variable Declarations
**----------------------------------------------------------------------------
*/

/*
**----------------------------------------------------------------------------
**  Function(internal and external use) Declarations
**----------------------------------------------------------------------------
*/

extern bool RLS_Convert_565to888(RGB565_T* pSrc, RGB888_T* pDest, int nWidth,
								 int nHeight);
extern bool RLS_Convert_888to565(RGB888_T* pSrc, RGB565_T* pDest, int nWidth,
								 int nHeight);

/*
**----------------------------------------------------------------------------
**  Function(external use only) Declarations
**----------------------------------------------------------------------------
*/

extern bool RLS_Convert_565toPNG(RGB565_T* pImg,const char* pszFn, int nWidth,
								 int nHeight);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // RLS_CONVERT_H
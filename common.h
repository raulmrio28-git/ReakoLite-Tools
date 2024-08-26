/*
** ===========================================================================
** File: common.h
** Description: ReakoLite library common header
** Copyright (c) 2024 raulmrio28-git.
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 08/26/2024	raulmrio28-git	Add header creation
** 08/23/2024	raulmrio28-git	Initial version
** ===========================================================================
*/

#ifndef RLS_COMMON_H
#define RLS_COMMON_H

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

#define RLS_MAGIC 0x4C52 //stored as 'RL'

#define RLS_PAL_BYTES sizeof(uint16_t)

#define RLS_SPAL_SIZE (UINT8_MAX + 1)
#define RLS_EPAL_SIZE (INT16_MAX + 1)

#define RLS_CEIL(n, d) ((n/d)+((n%d)!=0)) //portable ceil

/*
   Savings algorithm : ceil(((<szsum>-<isz>)/2)/<stdpal size>), where:
   szsum = <stdpal size>+<extpal size>+<data size>
   isz = <width>*<height>
*/

#define RLS_CALC_SAVING(ts, os) RLS_CEIL(((ts-os)/2), RLS_SPAL_SIZE)

/*
**----------------------------------------------------------------------------
**  Type Definitions
**----------------------------------------------------------------------------
*/

typedef struct tagRLSBaseHeader_T RLSBaseHeader_T;
typedef struct tagRLSInfoHeader12_T RLSInfoHeader12_T;
typedef struct tagRLSInfoHeader13_T RLSInfoHeader13_T;
typedef struct tagRLSAttributes12_T RLSAttributes12_T;
typedef struct tagRLSAttributes13_T RLSAttributes13_T;
typedef struct tagRLSBkInfo_T RLSBkInfo_T;

#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */
typedef struct tagRLSAttributes12_T
{
	uint8_t nSavings;
	bool bWOdd;
	bool bHOdd;
	bool bReserved;
};

typedef struct tagRLSAttributes13_T
{
	uint8_t nSavings;
	bool bWOdd : 1;
	bool bHOdd : 1;
	bool bReserved : 6;
};

typedef struct tagRLSBaseHeader_T
{
	uint16_t wMagic;
	uint16_t wVersion;
	uint8_t nFrames;
};

typedef struct tagRLSInfoHeader12_T
{
	uint8_t nWidth;
	uint8_t nHeight;
	uint8_t nPixBytes;
	RLSAttributes12_T tAttributes;
};

typedef struct tagRLSInfoHeader13_T
{
	uint16_t nWidth;
	uint16_t nHeight;
	uint8_t nPixBytes;
	RLSAttributes13_T tAttributes;
};

typedef struct tagRLSBkInfo_T
{
	uint8_t nPbIdx : 4;
	uint8_t baPalBits : 4;
};
#pragma pack(pop)   /* restore original alignment from stack */

typedef enum  tagRLS_PU_E
{
	RLS_BKI_PU_USEB = 0,
	RLS_BKI_PU_USEP = 1
};

typedef enum tagRLS_PT_E
{
	RLS_BKI_PAL_EP = 0,
	RLS_BKI_PAL_SP = 1
};

/*
**----------------------------------------------------------------------------
**  Variable Declarations
**----------------------------------------------------------------------------
*/

#ifdef RLS_EXTERN_VAR
extern uint16_t RLS_Common_StdPal[RLS_SPAL_SIZE];
extern uint16_t RLS_Common_ExtPal[RLS_EPAL_SIZE];
extern uint16_t RLS_Common_Block[2*2];
extern uint32_t RLS_Common_ExtPal_CIdx;

extern uint8_t RLS_Common_BkIdx[16];
extern uint8_t RLS_Common_PalBits[16];
#endif

/*
**----------------------------------------------------------------------------
**  Function(external use only) Declarations
**----------------------------------------------------------------------------
*/

extern int RLS_Common_GetInfo(uint8_t* pData, int* pnFrames, int* pnWidth,
							  int* pnHeight, int* pnPixBytes);
extern bool RLS_Common_MakeInfo(uint8_t* pData, int nFrames, int nWidth,
								int nHeight, int nSavings, bool bReserved);
extern bool RLS_Common_ExtractBlock(uint16_t* pImg, int nWidth, int nHeight,
									int nX, int nY);
extern bool RLS_Common_WriteBlock(uint16_t* pImg, int nWidth, int nHeight,
								  int nX, int nY);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // RLS_COMMON_H
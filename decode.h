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

#ifndef RLS_DECODE_H
#define RLS_DECODE_H

/*
**----------------------------------------------------------------------------
**  Includes
**----------------------------------------------------------------------------
*/

#ifdef __cplusplus
extern "C" {
#endif

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
**  Variable Declarations
**----------------------------------------------------------------------------
*/

/*
**----------------------------------------------------------------------------
**  Function(external use only) Declarations
**----------------------------------------------------------------------------
*/

extern bool RLS_Decode(uint8_t* pIn, int nFrame, uint16_t* pOut);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // RLS_DECODE_H
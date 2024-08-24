#include <stdint.h>
#include <stdbool.h>

#ifndef COMMON_H
#define COMMON_H

#define RLS_MAGIC 0x4C52

#define RLS_PAL_BYTES sizeof(uint16_t)

#define RLS_SPAL_SIZE (UINT8_MAX + 1)
#define RLS_EPAL_SIZE (INT16_MAX + 1)

#define RLS_BKI_PU_USEP	  1
#define RLS_BKI_PU_USEB	  0

#define RLS_BKI_PAL_SP	  1
#define RLS_BKI_PAL_EP	  0

#define RLS_BKI_PU_GB(v, i) ((v>>(3-i))&1)
#define RLS_BKI_BI_GB(v, i) ((v>>((3-i)<<1))&3)

#define RLS_CEIL(n, d) ((n/d)+((n&d)!=0))
#ifdef __cplusplus
extern "C" {
#endif
#pragma pack(push)  /* push current alignment to stack */
#pragma pack(1)     /* set alignment to 1 byte boundary */
typedef struct
{
	uint16_t magic;
	uint16_t version;
	uint8_t frames;
} rls_base_header_t;
#pragma pack(pop)   /* restore original alignment from stack */

typedef struct
{
	uint8_t width;
	uint8_t height;
	uint8_t p_bytes;
	uint32_t attributes;
} rls_img_header_12_t;

typedef struct
{
	uint16_t width;
	uint16_t height;
	uint8_t p_bytes;
	uint16_t attributes;
} rls_img_header_13_t;

typedef struct
{
	uint8_t savings;
	bool w_odd;
	bool h_odd;
} rls_attributes_12_t;

typedef struct
{
	uint8_t savings;
	bool w_odd : 1;
	bool h_odd : 1;
} rls_attributes_13_t;

typedef struct
{
	uint8_t pb_idx : 4;
	uint8_t pt_bits : 4;
} rls_bki_t;

#ifdef RLS_EXTERN_VAR
extern uint16_t rls_common_spal[RLS_SPAL_SIZE];
extern uint16_t rls_common_epal[RLS_EPAL_SIZE];
extern uint16_t rls_common_block[2*2];
extern uint32_t rls_common_epal_cidx;

extern uint8_t rls_common_pbits[16];
extern uint8_t rls_common_bkidx[16];
#endif

extern int rls_common_getinfo(uint8_t* input, int* frames, int* width, int* height, int* p_bytes);
extern bool rls_extract_block(uint16_t* img, int w, int h, int bkx, int bky);
extern bool rls_write_block(uint16_t* img, int w, int h, int bkx, int bky);

#ifdef __cplusplus
}
#endif
#endif
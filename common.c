#include "common.h"
#include <stdio.h>
uint16_t rls_common_spal[RLS_SPAL_SIZE];
uint16_t rls_common_epal[RLS_EPAL_SIZE];

uint16_t rls_common_block[2*2];
uint32_t rls_common_epal_cidx;

/* 
   Blocks legend: P - pal idx, I - reused pixel idx in blk
       ---------   ---------   ---------   ---------
	  | P0 | I0 | | P0 | P1 | | P0 | P1 | | P0 | I0 |
	   ---------   ---------   ---------   ---------
	  | I0 | I0 | | I1 | I1 | | I0 | I0 | | P1 | I0 |
       ---------   ---------   ---------   --------- 
      | P0 | I0 | | P0 | P1 | | P0 | I0 | | P0 | P1 |
       ---------   ---------   ---------   ---------
      | I0 | P1 | | I0 | I1 | | P1 | I2 | | I1 | I0 |
       ---------   ---------   ---------   --------- 
      | P0 | P1 | | P0 | I0 | | P0 | P1 | | P0 | P1 |
       ---------   ---------   ---------   ---------
      | I0 | P2 | | P1 | P2 | | P2 | I0 | | P2 | I1 |
       ---------   ---------   ---------   --------- 
      | P0 | P1 | | P0 | P1 | | P0 | P1 | | TR\|/AN |
       ---------   ---------   ---------   -UNUSED!-
      | P2 | I2 | | I1 | P2 | | P2 | P3 | | SP/|\BK |
       ---------   ---------   ---------   ---------     
*/

uint8_t rls_common_bkidx[16] =
{
	0b00000000, 0b00010101, 0b00010000, 0b00000100,
	0b00000001, 0b00010001, 0b00000110, 0b00010100,
	0b00010010, 0b00000110, 0b00011000, 0b00011001,
	0b00011010, 0b00010110, 0b00011011, 0b00011011
};

uint8_t rls_common_pbits[16] =
{
	0b1000, 0b1100, 0b1100, 0b1010,
	0b1001, 0b1100, 0b1010, 0b1100,
	0b1101, 0b1011, 0b1110, 0b1110,
	0b1110, 0b1101, 0b1111, 0b0000
};

int rls_common_getinfo(uint8_t* input, int* frames, int* width, int* height, int* p_bytes)
{
	rls_base_header_t* header = (rls_base_header_t*)input;
	if (header->magic == RLS_MAGIC)
	{
		*frames = header->frames;
		if (header->version = 0x1210)
		{
			rls_img_header_12_t* info =	(rls_img_header_12_t*)
										(input+sizeof(rls_base_header_t));
			if (width)
				*width = info->width;
			if (height)
				*height = info->height;
			if (p_bytes)
				*p_bytes = info->p_bytes;
		}
		else if (header->version = 0x1013)
		{
			rls_img_header_13_t* info = (rls_img_header_13_t*)
										(input+sizeof(rls_base_header_t));
			if (width)
				*width = info->width;
			if (height)
				*height = info->height;
			if (p_bytes)
				*p_bytes = info->p_bytes;
		}
		if (p_bytes && *p_bytes != RLS_PAL_BYTES)
			return 0;
		return 12;
	}
	return 0;
}

bool rls_extract_block(uint16_t* img, int w, int h, int bkx, int bky)
{
	if (!img || bkx >= (w >> 1) || bky >= (h >> 1))
		return false;
	rls_common_block[0] = img[w * (bky << 1) + (bkx << 1)];
	if ((w&1 && (w+1) == (bkx<<1)) && (h&1 && (h+1) == (bky<<1)))
	{
		rls_common_block[1] = rls_common_block[2]
	  = rls_common_block[3] = rls_common_block[0];
	}
	else if (w&1 && (w+1) == (bkx<<1))
	{
		rls_common_block[1] = rls_common_block[0];
		rls_common_block[2] = img[w * ((bky<<1) + 1) + (bkx<<1)];
		rls_common_block[3] = rls_common_block[2];
	}
	else
	{
		rls_common_block[1] = img[w * (bky<<1) + ((bkx<<1) + 1)];
		rls_common_block[2] = img[w * ((bky<<1) + 1) + (bkx<<1)];
		rls_common_block[3] = img[w * ((bky<<1) + 1) + ((bkx<<1) + 1)];
	}

	return true;
}

bool rls_write_block(uint16_t* img, int w, int h, int bkx, int bky)
{
	if (!img || bkx >= (w >> 1) || bky >= (h >> 1))
		return false;
	img[w * (bky<<1) + (bkx<<1)] = rls_common_block[0];
	if (w&1 && (w+1) == (bkx<<1))
	{
		img[w * ((bky<<1)+1) + (bkx<<1)] = rls_common_block[2];
	}
	else
	{
		img[w * (bky<<1) + (bkx<<1)+1] = rls_common_block[1];
		img[w * ((bky<<1)+1) + (bkx<<1)] = rls_common_block[2];
		img[w * ((bky<<1)+1) + (bkx<<1)+1] = rls_common_block[3];
	}

	return true;
}
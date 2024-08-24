#define RLS_EXTERN_VAR
#include "common.h"
#include <stdio.h>
#include <memory.h>

int rls_decode_block(unsigned char* input, unsigned short* output)
{
	rls_bki_t bk_info;
	int bk_pix;
	int o = 0;
	memcpy(&bk_info, &input[o++], sizeof(rls_bki_t));
	if (bk_info.pb_idx == 0xf) {
		for (bk_pix = 0; bk_pix < 2*2; bk_pix++)
		{
			if (RLS_BKI_PU_GB(bk_info.pt_bits, bk_pix) == RLS_BKI_PAL_EP)
				rls_common_block[bk_pix] = rls_common_epal[rls_common_epal_cidx++];
		}
	}
	else {
		uint8_t pal_bits = rls_common_pbits[bk_info.pb_idx];
		uint8_t bk_idx = rls_common_bkidx[bk_info.pb_idx];
		for (bk_pix = 0; bk_pix < 2*2; bk_pix++)
		{
			if (RLS_BKI_PU_GB(pal_bits, bk_pix) == RLS_BKI_PU_USEB && bk_pix > 0)
				rls_common_block[bk_pix]=rls_common_block[RLS_BKI_BI_GB(bk_idx,bk_pix)];
			else
			{
				if (RLS_BKI_PU_GB(bk_info.pt_bits, bk_pix) == RLS_BKI_PAL_SP)
					rls_common_block[bk_pix] = rls_common_spal[input[o++]];
				else
					rls_common_block[bk_pix] = rls_common_epal[rls_common_epal_cidx++];

			}
		}
	}
	return o;
}

uint32_t rls_decode_frame(uint8_t* input, uint16_t* output, int width, int height)
{
	uint8_t* curr_input = input;
	uint8_t* dat_end;
	bool skip_write = false;
	int bk_cols = RLS_CEIL(width, 2);
	int bk_rows = RLS_CEIL(height, 2);
	int c_bk_col, c_bk_row;
	if (!output)
		skip_write = true;
	memcpy(rls_common_spal, curr_input, RLS_SPAL_SIZE * RLS_PAL_BYTES);
	curr_input += RLS_SPAL_SIZE * RLS_PAL_BYTES;
	if (*(uint32_t*)curr_input > RLS_EPAL_SIZE * RLS_PAL_BYTES) /* Ext. pal. buffer over limit */
		return 0;
	memcpy(rls_common_epal, curr_input + sizeof(uint32_t), *(uint32_t*)curr_input);
	rls_common_epal_cidx = 0;
	curr_input += *(uint32_t*)curr_input + sizeof(uint32_t);
	dat_end = curr_input + *(uint32_t*)curr_input + sizeof(uint32_t);
	curr_input += sizeof(uint32_t);
	while (curr_input < dat_end)
	{
		for (c_bk_row = 0; c_bk_row < bk_rows; c_bk_row++)
		{
			for (c_bk_col = 0; c_bk_col < bk_cols; c_bk_col++)
			{
				if (skip_write == false && rls_extract_block(output, width, height, c_bk_col, c_bk_row) == false)
					return 0;
				curr_input += rls_decode_block(curr_input, rls_common_block);
				if (skip_write == false && rls_write_block(output, width, height, c_bk_col, c_bk_row) == false)
					return 0;
			}
		}
	}
	return (uint32_t)(curr_input - input);
}
bool rls_decode(uint8_t* input, int frame, uint16_t* output)
{
	uint8_t* curr_input = input;

	if (*(uint16_t*)curr_input == RLS_MAGIC) {
		int skip_frame;
		int offs_to_palette;
		int frames;
		int palette_bytes;
		int width;
		int height;
		offs_to_palette = rls_common_getinfo(curr_input, &frames, &width, &height, &palette_bytes);
		if (offs_to_palette == -1) /* Invalid information */
			return false;
		curr_input+=offs_to_palette;
		if (frame >= frames)
			return false;
		for (skip_frame = 0; skip_frame < frames-1; skip_frame++)
			curr_input+=rls_decode_frame(curr_input, NULL, width, height);
		if (rls_decode_frame(curr_input, output, width, height))
			return true;

	}
	return false;
}
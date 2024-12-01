/*
** ===========================================================================
** File: quant.c
** Description: ReakoLite library encoder quantizer
** Notes: Names of variables and some code rules have been kept intact.
** This does not apply to function that quantizes the whole image.
** This code contains part of ffmpeg code (libavcodec/rpzaenc.c)
** Copyright (c) 2024 raulmrio28-git.
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 12/01/2024	raulmrio28-git	Initial version
** ===========================================================================
*/

// Original copyright and base code information below

/*
 * QuickTime RPZA Video Encoder
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file rpzaenc.c
 * QT RPZA Video Encoder by Todd Kirby <doubleshot@pacbell.net> and David Adler
 */

// Original copyright and base code information above

/*
**----------------------------------------------------------------------------
**  Includes
**----------------------------------------------------------------------------
*/

#include "convert.h"
#include "math.h"
#include "string.h"

/*
**----------------------------------------------------------------------------
**  Definitions
**----------------------------------------------------------------------------
*/

#define QUANT_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define QUANT_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define QUANT_CLIP(x) (((x) > 255) ? 255 : ((x) < 0) ? 0 : (x))
#define SQR(x) ((x) * (x))
#define R(color) RLS_Quant_GetChan(color, RED)
#define G(color) RLS_Quant_GetChan(color, GREEN)
#define B(color) RLS_Quant_GetChan(color, BLUE)

/*
**----------------------------------------------------------------------------
**  Type Definitions
**----------------------------------------------------------------------------
*/

typedef enum channel_offset
{
    RED = 2,
    GREEN = 1,
    BLUE = 0,
} channel_offset;

typedef struct rgb
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb;

typedef struct BlockInfo
{
    int row;
    int col;
    int block_width;
    int block_height;
    int image_width;
    int image_height;
    int block_index;
    uint16_t start;
    int rowstride;
    int blocks_per_row;
    int total_blocks;
} BlockInfo;

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
** ---------------------------------------------------------------------------
**
** Function:
**     RLS_Quant_GetChan
**
** Description:
**     Get channel color
**
** Input:
**     color - color
**     chan - channel
**
** Output:
**     Color from channel
**
** Return value:
**     varies
**
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 12/01/2024	raulmrio28-git  Adapt to RLS encoder
** 08/21/2020	richardpl       avcodec: add RPZA encoder
** ---------------------------------------------------------------------------
*/

static uint8_t RLS_Quant_GetChan(uint16_t color, channel_offset chan)
{
    if (chan == RED)
        return ((color) >> 11 & 0x1F) * 8;
    else if (chan == GREEN)
        return ((color) >> 5 & 0x3F) * 4;
    else if (chan == BLUE)
        return ((color) & 0x1F) * 8;

    return 0;
}

/*
** ---------------------------------------------------------------------------
**
** Function:
**     RLS_Quant_GetCols
**
** Description:
**     Get quantized colors
**
** Input:
**     min - min color
**     max - max color
**     color4 - colors
**
** Output:
**     Colors
**
** Return value:
**     none
**
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 12/01/2024	raulmrio28-git  Adapt to RLS encoder
** 08/21/2020	richardpl       avcodec: add RPZA encoder
** ---------------------------------------------------------------------------
*/

static void RLS_Quant_GetCols(uint8_t* min, uint8_t* max,
    uint8_t color4[4][3])
{
    uint8_t nStep;

    color4[0][0] = min[0];
    color4[0][1] = min[1];
    color4[0][2] = min[2];

    color4[3][0] = max[0];
    color4[3][1] = max[1];
    color4[3][2] = max[2];

    // red components
    nStep = (color4[3][0] - color4[0][0] + 1) / 3;
    color4[1][0] = color4[0][0] + nStep;
    color4[2][0] = color4[3][0] - nStep;

    // green components
    nStep = (color4[3][1] - color4[0][1] + 1) / 3;
    color4[1][1] = color4[0][1] + nStep;
    color4[2][1] = color4[3][1] - nStep;

    // blue components
    nStep = (color4[3][2] - color4[0][2] + 1) / 3;
    color4[1][2] = color4[0][2] + nStep;
    color4[2][2] = color4[3][2] - nStep;
}

/*
** ---------------------------------------------------------------------------
**
** Function:
**     RLS_Quant_GetBkInfo
**
** Description:
**     Get block info
**
** Input:
**     bi - block info
**     block - block
**
** Output:
**     Block info
**
** Return value:
**     block ? (bi->col * 2) + (bi->row * bi->rowstride * 2) : 0
**
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 12/01/2024	raulmrio28-git  Adapt to RLS encoder
** 08/21/2020	richardpl       avcodec: add RPZA encoder
** ---------------------------------------------------------------------------
*/

static int RLS_Quant_GetBkInfo(BlockInfo* bi, int block)
{
    bi->row = block / bi->blocks_per_row;
    bi->col = block % bi->blocks_per_row;

    // test for right edge block
    if (bi->col == bi->blocks_per_row - 1 && (bi->image_width % 2) != 0) {
        bi->block_width = bi->image_width % 2;
    }
    else {
        bi->block_width = 2;
    }

    // test for bottom edge block
    if (bi->row == (bi->image_height / 2) && (bi->image_height % 2) != 0) {
        bi->block_height = bi->image_height % 2;
    }
    else {
        bi->block_height = 2;
    }

    return block ? (bi->col * 2) + (bi->row * bi->rowstride * 2) : 0;
}

/*
** ---------------------------------------------------------------------------
**
** Function:
**     RLS_Quant_888to565
**
** Description:
**     Encode a 24-bit RGB color to 16-bit RGB565
**
** Input:
**     rgb888 - 24-bit RGB color
**
** Output:
**     Converted color
**
** Return value:
**     wRGB565
**
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 12/01/2024	raulmrio28-git  Adapt to RLS encoder
** 08/21/2020	richardpl       avcodec: add RPZA encoder
** ---------------------------------------------------------------------------
*/

static uint16_t RLS_Quant_888to565(uint8_t* rgb888)
{
    uint16_t wRGB565 = 0;
    uint32_t nR, nG, nB;

    nR = rgb888[0] >> 3;
    nG = rgb888[1] >> 2;
    nB = rgb888[2] >> 3;

    wRGB565 |= (nR << 11);
    wRGB565 |= (nG << 5);
    wRGB565 |= (nB << 0);

    return wRGB565;
}

/*
** ---------------------------------------------------------------------------
**
** Function:
**     RLS_Quant_DiffColors
**
** Description:
**     Returns the total difference between two 24 bit color values
**
** Input:
**     colorA - 24-bit RGB color
**     colorB - 24-bit RGB color
**
** Output:
**     Total difference between two 24 bit color values
**
** Return value:
**     nTotal
**
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 12/01/2024	raulmrio28-git  Adapt to RLS encoder
** 08/21/2020	richardpl       avcodec: add RPZA encoder
** ---------------------------------------------------------------------------
*/

static int RLS_Quant_DiffColors(uint8_t* colorA, uint8_t* colorB)
{
    int nTotal;

    nTotal = SQR(colorA[0] - colorB[0]);
    nTotal += SQR(colorA[1] - colorB[1]);
    nTotal += SQR(colorA[2] - colorB[2]);

    return nTotal;
}

/*
** ---------------------------------------------------------------------------
**
** Function:
**     RLS_Quant_MaxCompDiff
**
** Description:
**     Returns the maximum component difference
**
** Input:
**     colorA - 24-bit RGB color
**     colorB - 24-bit RGB color
**
** Output:
**     Maximum component difference
**
** Return value:
**     nMax * 8
**
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 12/01/2024	raulmrio28-git  Adapt to RLS encoder
** 08/21/2020	richardpl       avcodec: add RPZA encoder
** ---------------------------------------------------------------------------
*/

static int RLS_Quant_MaxCompDiff(uint16_t* colorA, uint16_t* colorB)
{
    int nDiff, nMax = 0;

    nDiff = abs(R(colorA[0]) - R(colorB[0]));
    if (nDiff > nMax) {
        nMax = nDiff;
    }
    nDiff = abs(G(colorA[0]) - G(colorB[0]));
    if (nDiff > nMax) {
        nMax = nDiff;
    }
    nDiff = abs(B(colorA[0]) - B(colorB[0]));
    if (nDiff > nMax) {
        nMax = nDiff;
    }
    return nMax * 8;
}

/*
** ---------------------------------------------------------------------------
**
** Function:
**     RLS_Quant_MaxCompDiff
**
** Description:
**     Get maximum component difference
**
** Input:
**     bi - block info
**     block_ptr - pointer to block
**     min - minimum color
**     max - maximum color
**     chan - channel
**
** Output:
**     Maximum component difference
**
** Return value:
**     none
**
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 12/01/2024	raulmrio28-git  Adapt to RLS encoder
** 08/21/2020	richardpl       avcodec: add RPZA encoder
** ---------------------------------------------------------------------------
*/

static void RLS_Quant_GetMaxCompDiff(BlockInfo* bi, uint16_t* block_ptr,
    uint8_t* min, uint8_t* max,
    channel_offset* chan)
{
    int x, y;
    uint8_t min_r, max_r, min_g, max_g, min_b, max_b;
    uint8_t r, g, b;

    // fix warning about uninitialized vars
    min_r = min_g = min_b = UINT8_MAX;
    max_r = max_g = max_b = 0;

    // loop thru and compare pixels
    for (y = 0; y < bi->block_height; y++) {
        for (x = 0; x < bi->block_width; x++) {
            // TODO:  optimize
            min_r = QUANT_MIN(R(block_ptr[x]), min_r);
            min_g = QUANT_MIN(G(block_ptr[x]), min_g);
            min_b = QUANT_MIN(B(block_ptr[x]), min_b);

            max_r = QUANT_MAX(R(block_ptr[x]), max_r);
            max_g = QUANT_MAX(G(block_ptr[x]), max_g);
            max_b = QUANT_MAX(B(block_ptr[x]), max_b);
        }
        block_ptr += bi->rowstride;
    }

    r = max_r - min_r;
    g = max_g - min_g;
    b = max_b - min_b;

    if (r > g && r > b) {
        *max = max_r;
        *min = min_r;
        *chan = RED;
    }
    else if (g > b && g >= r) {
        *max = max_g;
        *min = min_g;
        *chan = GREEN;
    }
    else {
        *max = max_b;
        *min = min_b;
        *chan = BLUE;
    }
}

/*
** ---------------------------------------------------------------------------
**
** Function:
**     RLS_Quant_CompareBlock
**
** Description:
**     Compare two 2x2 blocks to determine if the total difference between the
**     blocks is greater than the thresh parameter
**
** Input:
**     block1 - 2x2 block
**     block2 - 2x2 block
**     bi - block info
**     thresh - threshold
**
** Output:
**     Comparison
**
** Return value:
**     0/-1
**
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 12/01/2024	raulmrio28-git  Adapt to RLS encoder
** 08/21/2020	richardpl       avcodec: add RPZA encoder
** ---------------------------------------------------------------------------
*/

static int RLS_Quant_CompareBlock(uint16_t* block1, uint16_t* block2,
    BlockInfo* bi, int thresh)
{
    int x, y, diff = 0;
    for (y = 0; y < bi->block_height; y++) {
        for (x = 0; x < bi->block_width; x++) {
            diff = RLS_Quant_MaxCompDiff(&block1[x], &block2[x]);
            if (diff >= thresh) {
                return -1;
            }
        }
        block1 += bi->rowstride;
        block2 += bi->rowstride;
    }
    return 0;
}

/*
** ---------------------------------------------------------------------------
**
** Function:
**     RLS_Quant_LeastSq
**
** Description:
**     Determine the fit of one channel to another within a 2x2 block
**
** Input:
**     block_ptr - 2x2 block
**     bi - block info
**     xchannel - x channel
**     ychannel - y channel
**     slope - slope
**     y_intercept - y intercept
**     correlation_coef - correlation coefficient
**
** Output:
**     Least square
**
** Return value:
**     0/-1/-2
**
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 12/01/2024	raulmrio28-git  Adapt to RLS encoder
** 08/21/2020	richardpl       avcodec: add RPZA encoder
** ---------------------------------------------------------------------------
*/

static int RLS_Quant_LeastSq(uint16_t* block_ptr, BlockInfo* bi,
    channel_offset xchannel, channel_offset ychannel,
    double* slope, double* y_intercept,
    double* correlation_coef)
{
    double sumx = 0, sumy = 0, sumx2 = 0, sumy2 = 0, sumxy = 0,
        sumx_sq = 0, sumy_sq = 0, tmp, tmp2;
    int i, j, count;
    uint8_t x, y;

    count = bi->block_height * bi->block_width;

    if (count < 2)
        return -1;

    for (i = 0; i < bi->block_height; i++) {
        for (j = 0; j < bi->block_width; j++) {
            x = RLS_Quant_GetChan(block_ptr[j], xchannel);
            y = RLS_Quant_GetChan(block_ptr[j], ychannel);
            sumx += x;
            sumy += y;
            sumx2 += x * x;
            sumy2 += y * y;
            sumxy += x * y;
        }
        block_ptr += bi->rowstride;
    }

    sumx_sq = sumx * sumx;
    tmp = (count * sumx2 - sumx_sq);

    // guard against div/0
    if (tmp == 0)
        return -2;

    sumy_sq = sumy * sumy;

    *slope = (sumx * sumy - sumxy) / tmp;
    *y_intercept = (sumy - (*slope) * sumx) / count;

    tmp2 = count * sumy2 - sumy_sq;
    if (tmp2 == 0) {
        *correlation_coef = 0.0;
    }
    else {
        *correlation_coef = (count * sumxy - sumx * sumy) /
            sqrt(tmp * tmp2);
    }

    return 0; // success
}

/*
** ---------------------------------------------------------------------------
**
** Function:
**     RLS_Quant_MaxLsqFitError
**
** Description:
**     Determine the amount of error in the lsq fit
**
** Input:
**     block_ptr - 2x2 block
**     bi - block info
**     min - minimum color
**     max - maximum color
**     tmp_min - temporary minimum color
**     tmp_max - temporary maximum color
**     xchannel - x channel
**     ychannel - y channel
**
** Output:
**     Least square error
**
** Return value:
**     max_err
**
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 12/01/2024	raulmrio28-git  Adapt to RLS encoder
** 08/21/2020	richardpl       avcodec: add RPZA encoder
** ---------------------------------------------------------------------------
*/

static int RLS_Quant_MaxLsqFitError(uint16_t* block_ptr, BlockInfo* bi,
    int min, int max, int tmp_min, int tmp_max,
    channel_offset xchannel,
    channel_offset ychannel)
{
    int i, j, x, y;
    int err;
    int max_err = 0;

    for (i = 0; i < bi->block_height; i++) {
        for (j = 0; j < bi->block_width; j++) {
            int x_inc, lin_y, lin_x;
            x = RLS_Quant_GetChan(block_ptr[j], xchannel);
            y = RLS_Quant_GetChan(block_ptr[j], ychannel);

            /* calculate x_inc as the 4-color index (0..3) */
            x_inc = (int)floor((x - min) * 3.0 / (max - min));
            x_inc = QUANT_MAX(QUANT_MIN(3, x_inc), 0);

            /* calculate lin_y corresponding to x_inc */
            lin_y = (int)(tmp_min + (tmp_max - tmp_min) * x_inc / 3.0);

            err = abs(lin_y - y);
            if (err > max_err)
                max_err = err;

            /* calculate lin_x corresponding to x_inc */
            lin_x = (int)(min + (max - min) * x_inc / 3.0);

            err = abs(lin_x - x);
            if (err > max_err)
                max_err += err;
        }
        block_ptr += bi->rowstride;
    }

    return max_err;
}

/*
** ---------------------------------------------------------------------------
**
** Function:
**     RLS_Quant_ApproxColor
**
** Description:
**     Determine approximate color
**
** Input:
**     color - 24-bit RGB color
**     colors - 4 colors
**
** Output:
**     Approximate colors
**
** Return value:
**     ret
**
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 12/01/2024	raulmrio28-git  Adapt to RLS encoder
** 08/21/2020	richardpl       avcodec: add RPZA encoder
** ---------------------------------------------------------------------------
*/

static int RLS_Quant_ApproxColor(uint16_t* color, uint8_t colors[4][3])
{
    int ret = 0;
    int smallest_variance = 0x7fffffff;
    uint8_t dithered_color[3];

    for (int channel = 0; channel < 3; channel++) {
        dithered_color[channel] = RLS_Quant_GetChan(color[0], channel);
    }

    for (int palette_entry = 0; palette_entry < 4; palette_entry++) {
        int variance = RLS_Quant_DiffColors(dithered_color,
            colors[palette_entry]);

        if (variance < smallest_variance) {
            smallest_variance = variance;
            ret = palette_entry;
        }
    }

    return ret;
}

/*
** ---------------------------------------------------------------------------
**
** Function:
**     RLS_Quant_QuantBlock
**
** Description:
**     Quantize a block
**
** Input:
**     min_color - minimum color
**     max_color - maximum color
**     block_ptr - 2x2 block
**     bi - block info
**
** Output:
**     Quantized block
**
** Return value:
**     1
**
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 12/01/2024	raulmrio28-git  Adapt to RLS encoder
** 11/12/2022	richardpl	    avcodec/rpzaenc: stop accessing out of bounds
**                              frame
** 08/21/2020	richardpl       avcodec: add RPZA encoder
** ---------------------------------------------------------------------------
*/

static int RLS_Quant_QuantBlock(uint8_t* min_color, uint8_t* max_color,
    uint16_t* block_ptr, BlockInfo* bi)
{
    int x, y, idx;
    const int y_size = QUANT_MIN(2, bi->image_height - bi->row * 2);
    const int x_size = QUANT_MIN(2, bi->image_width - bi->col * 2);
    uint8_t color4[4][3];
    uint16_t rounded_max, rounded_min;

    // round min and max wider
    rounded_min = RLS_Quant_888to565(min_color);
    rounded_max = RLS_Quant_888to565(max_color);

    RLS_Quant_GetCols(min_color, max_color, color4);

    for (y = 0; y < y_size; y++) {
        for (x = 0; x < x_size; x++) {
            idx = RLS_Quant_ApproxColor(&block_ptr[x], color4);
            block_ptr[x] = RLS_Quant_888to565(color4[idx]);
        }
        block_ptr += bi->rowstride;
    }
    return 1; // num blocks encoded
}

/*
** ---------------------------------------------------------------------------
**
** Function:
**     RLS_Quant_UpdStats
**
** Description:
**     Update block stats
**
** Input:
**     bi - block info
**     block - 2x2 block
**     min_color - minimum color
**     max_color - maximum color
**     total_rgb - total rgb
**     total_pixels - total pixels
**     avg_color - average color
**     first_block - first block
**
** Output:
**     Quantized block
**
** Return value:
**     1
**
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 12/01/2024	raulmrio28-git  Adapt to RLS encoder
** 11/12/2022	richardpl	    avcodec/rpzaenc: stop accessing out of bounds
**                              frame
** 08/21/2020	richardpl       avcodec: add RPZA encoder
** ---------------------------------------------------------------------------
*/

static int RLS_Quant_UpdStats(BlockInfo* bi, uint16_t* block,
    uint8_t min_color[3], uint8_t max_color[3],
    int* total_rgb, int* total_pixels,
    uint8_t avg_color[3], int first_block)
{
    int x, y;
    int is_in_range;
    int total_pixels_blk;
    int threshold;

    uint8_t min_color_blk[3], max_color_blk[3];
    int total_rgb_blk[3];
    uint8_t avg_color_blk[3];

    if (first_block) {
        min_color[0] = UINT8_MAX;
        min_color[1] = UINT8_MAX;
        min_color[2] = UINT8_MAX;
        max_color[0] = 0;
        max_color[1] = 0;
        max_color[2] = 0;
        total_rgb[0] = 0;
        total_rgb[1] = 0;
        total_rgb[2] = 0;
        *total_pixels = 0;
        threshold = 16;
    }
    else {
        threshold = 32;
    }

    /*
       The *_blk variables will include the current block.
       Initialize them based on the blocks so far.
     */
    min_color_blk[0] = min_color[0];
    min_color_blk[1] = min_color[1];
    min_color_blk[2] = min_color[2];
    max_color_blk[0] = max_color[0];
    max_color_blk[1] = max_color[1];
    max_color_blk[2] = max_color[2];
    total_rgb_blk[0] = total_rgb[0];
    total_rgb_blk[1] = total_rgb[1];
    total_rgb_blk[2] = total_rgb[2];
    total_pixels_blk = *total_pixels + bi->block_height * bi->block_width;

    /*
       Update stats for this block's pixels
     */
    for (y = 0; y < bi->block_height; y++) {
        for (x = 0; x < bi->block_width; x++) {
            total_rgb_blk[0] += R(block[x]);
            total_rgb_blk[1] += G(block[x]);
            total_rgb_blk[2] += B(block[x]);

            min_color_blk[0] = QUANT_MIN(R(block[x]), min_color_blk[0]);
            min_color_blk[1] = QUANT_MIN(G(block[x]), min_color_blk[1]);
            min_color_blk[2] = QUANT_MIN(B(block[x]), min_color_blk[2]);

            max_color_blk[0] = QUANT_MAX(R(block[x]), max_color_blk[0]);
            max_color_blk[1] = QUANT_MAX(G(block[x]), max_color_blk[1]);
            max_color_blk[2] = QUANT_MAX(B(block[x]), max_color_blk[2]);
        }
        block += bi->rowstride;
    }

    /*
       Calculate average color including current block.
     */
    avg_color_blk[0] = total_rgb_blk[0] / total_pixels_blk;
    avg_color_blk[1] = total_rgb_blk[1] / total_pixels_blk;
    avg_color_blk[2] = total_rgb_blk[2] / total_pixels_blk;

    /*
       Are all the pixels within threshold of the average color?
     */
    is_in_range = (max_color_blk[0] - avg_color_blk[0] <= threshold &&
        max_color_blk[1] - avg_color_blk[1] <= threshold &&
        max_color_blk[2] - avg_color_blk[2] <= threshold &&
        avg_color_blk[0] - min_color_blk[0] <= threshold &&
        avg_color_blk[1] - min_color_blk[1] <= threshold &&
        avg_color_blk[2] - min_color_blk[2] <= threshold);

    if (is_in_range) {
        /*
           Set the output variables to include this block.
         */
        min_color[0] = min_color_blk[0];
        min_color[1] = min_color_blk[1];
        min_color[2] = min_color_blk[2];
        max_color[0] = max_color_blk[0];
        max_color[1] = max_color_blk[1];
        max_color[2] = max_color_blk[2];
        total_rgb[0] = total_rgb_blk[0];
        total_rgb[1] = total_rgb_blk[1];
        total_rgb[2] = total_rgb_blk[2];
        *total_pixels = total_pixels_blk;
        avg_color[0] = avg_color_blk[0];
        avg_color[1] = avg_color_blk[1];
        avg_color[2] = avg_color_blk[2];
    }

    return is_in_range;
}

/*
** ---------------------------------------------------------------------------
**
** Function:
**     RLS_Quantize
**
** Description:
**     Quantize the whole image
**
** Input:
**     pImg: Image to quantize
**     nWidth: Width of image
**     nHeight: Height of image
**
** Output:
**     Quantized image
**
** Return value:
**     none
**
** History:
** when			who				what, where, why
** MM-DD-YYYY-- --------------- --------------------------------
** 12/01/2024	raulmrio28-git  Adapt to RLS encoder
** 08/13/2024	jamrial 	    avcodec/rpzaenc: don't use buffer data beyond
**                              the end of a row
** 11/12/2022	richardpl	    avcodec/rpzaenc: stop accessing out of bounds
**                              frame
** 08/22/2020	mkver   	    avcodec/rzpaenc: Remove set-but-unused
**                              variable
** 08/21/2020	richardpl       avcodec: add RPZA encoder
** ---------------------------------------------------------------------------
*/

void RLS_Quantize(RGB565_T* pImg, int nWidth, int nHeight)
{
    BlockInfo bi;
    int block_counter = 0;
    int total_blocks;
    int block_offset = 0;
    uint8_t min = 0, max = 0;
    channel_offset chan;
    int i;
    int tmp_min, tmp_max;
    int total_rgb[3];
    uint8_t avg_color[3];
    int pixel_count;
    uint8_t min_color[3], max_color[3];
    double slope, y_intercept, correlation_coef;
    uint16_t* src_pixels = (uint16_t*)pImg;

    /* Number of 2x2 blocks in frame. */
    total_blocks = ((nWidth + 1) / 2) * ((nHeight + 1) / 2);

    bi.image_width = nWidth;
    bi.image_height = nHeight;
    bi.rowstride = nWidth;

    bi.blocks_per_row = (nWidth + 1) / 2;

    while (block_counter < total_blocks) {
        block_offset = RLS_Quant_GetBkInfo(&bi, block_counter);
        // ONE COLOR CHECK
        if (RLS_Quant_UpdStats(&bi, &src_pixels[block_offset],
            min_color, max_color,
            total_rgb, &pixel_count, avg_color, 1)) {
                {
                    uint16_t* row_ptr;
                    int y_size, x_size, rgb555;

                    block_offset = RLS_Quant_GetBkInfo(&bi, block_counter);

                    row_ptr = &src_pixels[block_offset];
                    y_size = QUANT_MIN(2, bi.image_height - bi.row * 2);
                    x_size = QUANT_MIN(2, bi.image_width - bi.col * 2);

                    for (int y = 0; y < y_size; y++) {
                        for (int x = 0; x < x_size; x++) {
                            rgb555 = RLS_Quant_888to565(avg_color);
                            row_ptr[x] = rgb555;
                        }

                        row_ptr += bi.rowstride;
                    }
                }
                block_counter++;
        }
        else { // FOUR COLOR CHECK
            int err = 0;

            // get max component diff for block
            RLS_Quant_GetMaxCompDiff(&bi, &src_pixels[block_offset], &min,
                &max, &chan);

            min_color[0] = 0;
            max_color[0] = 0;
            min_color[1] = 0;
            max_color[1] = 0;
            min_color[2] = 0;
            max_color[2] = 0;

            // run least squares against other two components
            for (i = 0; i < 3; i++) {
                if (i == chan) {
                    min_color[i] = min;
                    max_color[i] = max;
                    continue;
                }

                slope = y_intercept = correlation_coef = 0;

                if (RLS_Quant_LeastSq(&src_pixels[block_offset], &bi, chan, i,
                    &slope, &y_intercept, &correlation_coef)) {
                    min_color[i] = RLS_Quant_GetChan(src_pixels[block_offset],
                        i);
                    max_color[i] = RLS_Quant_GetChan(src_pixels[block_offset],
                        i);
                }
                else {
                    tmp_min = (int)(min * slope + y_intercept);
                    tmp_max = (int)(max * slope + y_intercept);

                    // clamp min and max color values
                    tmp_min = QUANT_CLIP(tmp_min);
                    tmp_max = QUANT_CLIP(tmp_max);

                    err = QUANT_MAX(RLS_Quant_MaxLsqFitError
                    (&src_pixels[block_offset], &bi,
                        min, max, tmp_min, tmp_max, chan, i), err);

                    min_color[i] = tmp_min;
                    max_color[i] = tmp_max;
                }
            }
            if (err <= 8) // FOUR COLOR BLOCK
                RLS_Quant_QuantBlock(min_color, max_color,
                    &src_pixels[block_offset], &bi);
            block_counter++;
        }
    }
}
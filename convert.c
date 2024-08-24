#include "convert.h"
#include "spng/spng.h"

bool rls_convert_565_888(rls_convert_16bpp_t* src, rls_convert_24bpp_t* dst, int w, int h)
{
	int sz;
	int co;
	if (!src || !dst || w <= 0 || h <= 0)
		return false;
	sz = w * h;
	for (co=0; co<sz; co++)
	{
		dst[co].b = CVT_16BPP_EX_B(src[co]);
		dst[co].g = CVT_16BPP_EX_G(src[co]);
		dst[co].r = CVT_16BPP_EX_R(src[co]);
	}
	return true;
}

bool rls_convert_888_565(rls_convert_24bpp_t* src, rls_convert_16bpp_t* dst, int w, int h)
{
	int sz;
	int co;
	if (!src || !dst || w <= 0 || h <= 0)
		return false;
	sz = w * h;
	for (co = 0; co < sz; co++)
		dst[co] = ((src[co].b >> 3) << 11) | ((src[co].g >> 2) << 5) | (src[co].r >> 3);
	return true;
}

bool rls_convert_5652png(const char* dfn, rls_convert_16bpp_t* src, int w, int h)
{
	int png_result;
	void* png_buf;
	size_t png_size;
	FILE* f;
	spng_ctx* png_ctx = NULL;
	struct spng_ihdr png_ihdr = { 0 }; /* zero-initialize to set valid defaults */
	rls_convert_24bpp_t* cvt_buff;

	png_ihdr.width = w;
	png_ihdr.height = h;
	png_ihdr.bit_depth = 8;
	png_ihdr.color_type = SPNG_COLOR_TYPE_TRUECOLOR;

	png_ctx = spng_ctx_new(SPNG_CTX_ENCODER);
	spng_set_option(png_ctx, SPNG_IMG_COMPRESSION_LEVEL, 9);
	spng_set_option(png_ctx, SPNG_ENCODE_TO_BUFFER, 1);
	spng_set_ihdr(png_ctx, &png_ihdr);

	cvt_buff = (rls_convert_24bpp_t*)malloc(w * h * sizeof(rls_convert_24bpp_t));
	if (!cvt_buff)
	{
		printf("Failed to allocate memory\n");
		return false;
	}
	if (rls_convert_565_888(src, cvt_buff, w, h) == false)
	{
		printf("Failed to convert 565 to 888\n");
		return false;
	}
	png_result = spng_encode_image(png_ctx, cvt_buff, w * h * sizeof(rls_convert_24bpp_t),
								   SPNG_FMT_PNG, SPNG_ENCODE_FINALIZE);

	if (png_result) {
		printf("PNG encoded failed with error: %s\n", spng_strerror(png_result));
		spng_ctx_free(png_ctx);
		return false;
	}

	png_buf = spng_get_png_buffer(png_ctx, &png_size, &png_result);

	if (png_buf == NULL)
	{
		printf("PNG buffer grab with error: %s\n", spng_strerror(png_result));
		spng_ctx_free(png_ctx);
		return false;
	}

	f = fopen(dfn, "wb");
	if (f == NULL) {
		printf("Cannot open output file\n");
		free(png_buf);
		spng_ctx_free(png_ctx);
		return false;
	}

	fwrite(png_buf, png_size, 1, f);
	fclose(f);
	free(png_buf);
	spng_ctx_free(png_ctx);
	return true;
}
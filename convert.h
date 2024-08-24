#include <stdint.h>
#include <stdbool.h>

#ifndef CONVERT_H
#define CONVERT_H

#define CVT_16BPP_EX_B(x) ((x>>11) << 3)
#define CVT_16BPP_EX_G(x) (((x>>5)&0x3f) << 2)
#define CVT_16BPP_EX_R(x) ((x&0x1f) << 3)

#ifdef __cplusplus
extern "C" {
#endif
typedef uint16_t rls_convert_16bpp_t;

typedef struct {
	uint8_t b;
	uint8_t g;
	uint8_t r;
} rls_convert_24bpp_t;

extern bool rls_convert_565_888(rls_convert_16bpp_t* src, rls_convert_24bpp_t* dst, int w, int h);
extern bool rls_convert_888_565(rls_convert_24bpp_t* src, rls_convert_16bpp_t* dst, int w, int h);
extern bool rls_convert_5652png(const char* dfn, rls_convert_16bpp_t* src, int w, int h);
#ifdef __cplusplus
}
#endif

#endif
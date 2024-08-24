#ifndef DECODE_H
#define DECODE_H
#ifdef __cplusplus
extern "C" {
#endif
extern bool rls_decode(uint8_t* input, int frame, uint16_t* output);
#ifdef __cplusplus
}
#endif
#endif
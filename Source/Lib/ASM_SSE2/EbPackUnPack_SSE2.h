/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#ifndef EbPackUnPack_asm_h
#define EbPackUnPack_asm_h

#include "EbDefinitions.h"

#ifdef __cplusplus
extern "C" {
#endif

    void EB_ENC_msbPack2D_SSE2_INTRIN(
        uint8_t     *in8_bit_buffer,
        uint32_t     in8_stride,
        uint8_t     *inn_bit_buffer,
        uint16_t    *out16_bit_buffer,
        uint32_t     inn_stride,
        uint32_t     out_stride,
        uint32_t     width,
        uint32_t     height);

    void EB_ENC_msbUnPack2D_SSE2_INTRIN(
        uint16_t      *in16BitBuffer,
        uint32_t       inStride,
        uint8_t       *out8BitBuffer,
        uint8_t       *outnBitBuffer,
        uint32_t       out8Stride,
        uint32_t       outnStride,
        uint32_t       width,
        uint32_t       height);
    void EB_ENC_UnPack8BitData_SSE2_INTRIN(
        uint16_t      *in16BitBuffer,
        uint32_t       inStride,
        uint8_t       *out8BitBuffer,
        uint32_t       out8Stride,
        uint32_t       width,
        uint32_t       height);

    void EB_ENC_UnPack8BitDataSafeSub_SSE2_INTRIN(
        uint16_t      *in16BitBuffer,
        uint32_t       inStride,
        uint8_t       *out8BitBuffer,
        uint32_t       out8Stride,
        uint32_t       width,
        uint32_t       height,
        EbBool      sub_pred
    );

    void UnpackAvg_SSE2_INTRIN(
        uint16_t *ref16_l0,
        uint32_t  ref_l0_stride,
        uint16_t *ref16_l1,
        uint32_t  ref_l1_stride,
        uint8_t  *dst_ptr,
        uint32_t  dst_stride,
        uint32_t  width,
        uint32_t  height);


#ifdef __cplusplus
}
#endif
#endif // EbPackUnPack_asm_h


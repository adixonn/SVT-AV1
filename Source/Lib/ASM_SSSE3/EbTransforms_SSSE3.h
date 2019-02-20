/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/
#ifndef EbTransforms_SSSE3_h
#define EbTransforms_SSSE3_h

#include "EbDefinitions.h"
#ifdef __cplusplus
extern "C" {
#endif

    void QuantizeInvQuantizeNxN_SSE3(
        int16_t          *coeff,
        const uint32_t     coeff_stride,
        int16_t          *quant_coeff,
        int16_t          *recon_coeff,
        const uint32_t     q_func,
        const uint32_t     q_offset,
        const int32_t     shifted_q_bits,
        const int32_t     shifted_f_func,
        const int32_t     iq_offset,
        const int32_t     shift_num,
        const uint32_t     area_size,
        uint32_t          *nonzerocoeff);

    void QuantizeInvQuantize8x8_SSE3(
        int16_t          *coeff,
        const uint32_t     coeff_stride,
        int16_t          *quant_coeff,
        int16_t          *recon_coeff,
        const uint32_t     q_func,
        const uint32_t     q_offset,
        const int32_t     shifted_q_bits,
        const int32_t     shifted_f_func,
        const int32_t     iq_offset,
        const int32_t     shift_num,
        const uint32_t     area_size,
        uint32_t          *nonzerocoeff);

    void QuantizeInvQuantize4x4_SSE3(
        int16_t          *coeff,
        const uint32_t     coeff_stride,
        int16_t          *quant_coeff,
        int16_t          *recon_coeff,
        const uint32_t     q_func,
        const uint32_t     q_offset,
        const int32_t     shifted_q_bits,
        const int32_t     shifted_f_func,
        const int32_t     iq_offset,
        const int32_t     shift_num,
        const uint32_t     area_size,
        uint32_t          *nonzerocoeff);


    void lowPrecisionTransform16x16_SSSE3(int16_t *src, uint32_t src_stride, int16_t *dst, uint32_t dst_stride, int16_t *intermediate, uint32_t addshift);
    void lowPrecisionTransform32x32_SSSE3(int16_t *src, uint32_t src_stride, int16_t *dst, uint32_t dst_stride, int16_t *intermediate, uint32_t addshift);

    void PFinvTransform32x32_SSSE3(int16_t *src, uint32_t src_stride, int16_t *dst, uint32_t dst_stride, int16_t *intermediate, uint32_t addshift);
    void PFinvTransform16x16_SSSE3(int16_t *src, uint32_t src_stride, int16_t *dst, uint32_t dst_stride, int16_t *intermediate, uint32_t addshift);

#ifdef __cplusplus
}
#endif
#endif // EbTransforms_SSSE3_h


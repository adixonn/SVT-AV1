/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/
#ifndef EbPackUnPack_C_h
#define EbPackUnPack_C_h
#ifdef __cplusplus
extern "C" {
#endif

#include "EbDefinitions.h"


    void EB_ENC_msbPack2D(
        uint8_t     *in8_bit_buffer,
        uint32_t     in8_stride,
        uint8_t     *inn_bit_buffer,
        uint16_t    *out16_bit_buffer,
        uint32_t     inn_stride,
        uint32_t     out_stride,
        uint32_t     width,
        uint32_t     height);

    void CompressedPackmsb(
        uint8_t     *in8_bit_buffer,
        uint32_t     in8_stride,
        uint8_t     *inn_bit_buffer,
        uint16_t    *out16_bit_buffer,
        uint32_t     inn_stride,
        uint32_t     out_stride,
        uint32_t     width,
        uint32_t     height);
    void CPack_C(
        const uint8_t     *inn_bit_buffer,
        uint32_t     inn_stride,
        uint8_t     *in_compn_bit_buffer,
        uint32_t     out_stride,
        uint8_t    *local_cache,
        uint32_t     width,
        uint32_t     height);



    void EB_ENC_msbUnPack2D(
        uint16_t      *in16_bit_buffer,
        uint32_t       in_stride,
        uint8_t       *out8_bit_buffer,
        uint8_t       *outn_bit_buffer,
        uint32_t       out8_stride,
        uint32_t       outn_stride,
        uint32_t       width,
        uint32_t       height);
    void UnPack8BitData(
        uint16_t      *in16_bit_buffer,
        uint32_t       in_stride,
        uint8_t       *out8_bit_buffer,
        uint32_t       out8_stride,
        uint32_t       width,
        uint32_t       height);
    void UnpackAvg(
        uint16_t      *ref16_l0,
        uint32_t       ref_l0_stride,
        uint16_t      *ref16_l1,
        uint32_t       ref_l1_stride,
        uint8_t       *dst_ptr,
        uint32_t       dst_stride,
        uint32_t       width,
        uint32_t       height);

    void UnpackAvgSafeSub(
        uint16_t      *ref16_l0,
        uint32_t       ref_l0_stride,
        uint16_t      *ref16_l1,
        uint32_t       ref_l1_stride,
        uint8_t       *dst_ptr,
        uint32_t       dst_stride,
        EbBool      sub_pred,
        uint32_t       width,
        uint32_t       height);


#ifdef __cplusplus
}
#endif
#endif // EbPackUnPack_C_h
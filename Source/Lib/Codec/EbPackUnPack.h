/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#ifndef EbPackUnPack_h
#define EbPackUnPack_h
#ifdef __cplusplus
extern "C" {
#endif

#include "EbPackUnPack_C.h"
#include "EbPackUnPack_SSE2.h"
#include "EbPictureOperators.h"

    typedef void(*EB_ENC_Pack2D_TYPE)(
        uint8_t     *in8_bit_buffer,
        uint32_t     in8_stride,
        uint8_t     *inn_bit_buffer,
        uint16_t    *out16_bit_buffer,
        uint32_t     inn_stride,
        uint32_t     out_stride,
        uint32_t     width,
        uint32_t     height);

    EB_ENC_Pack2D_TYPE Pack2D_funcPtrArray_16Bit_SRC[2][ASM_TYPE_TOTAL] =
    {
        {
            // NON_AVX2
            EB_ENC_msbPack2D,
            // AVX2
            EB_ENC_msbPack2D,
        },
        {
            // NON_AVX2
            eb_enc_msb_pack2d_sse2_intrin,
            // AVX2
            eb_enc_msb_pack2d_avx2_intrin_al,//EB_ENC_msbPack2D_AVX2
        }
    };


    EB_ENC_Pack2D_TYPE CompressedPack_funcPtrArray[ASM_TYPE_TOTAL] =
    {
        // NON_AVX2
        CompressedPackmsb,
        // AVX2
        compressed_packmsb_avx2_intrin,

    };

    typedef void(*COMPPack_TYPE)(
        const uint8_t     *inn_bit_buffer,
        uint32_t     inn_stride,
        uint8_t     *in_compn_bit_buffer,
        uint32_t     out_stride,
        uint8_t    *local_cache,
        uint32_t     width,
        uint32_t     height);

    COMPPack_TYPE  Convert_Unpack_CPack_funcPtrArray[ASM_TYPE_TOTAL] =
    {
        // NON_AVX2
        CPack_C,
        // AVX2
        c_pack_avx2_intrin,

    };

    typedef void(*EB_ENC_UnPack2D_TYPE)(
        uint16_t      *in16_bit_buffer,
        uint32_t       in_stride,
        uint8_t       *out8_bit_buffer,
        uint8_t       *outn_bit_buffer,
        uint32_t       out8_stride,
        uint32_t       outn_stride,
        uint32_t       width,
        uint32_t       height);

    EB_ENC_UnPack2D_TYPE UnPack2D_funcPtrArray_16Bit[2][ASM_TYPE_TOTAL] =
    {
        {
            // NON_AVX2
            EB_ENC_msbUnPack2D,
            // AVX2
            EB_ENC_msbUnPack2D,
        },
        {
            // NON_AVX2
            eb_enc_msb_un_pack2d_sse2_intrin,
            // AVX2
            eb_enc_msb_un_pack2d_sse2_intrin,
        }
    };

    typedef void(*EB_ENC_UnpackAvg_TYPE)(
        uint16_t *ref16_l0,
        uint32_t  ref_l0_stride,
        uint16_t *ref16_l1,
        uint32_t  ref_l1_stride,
        uint8_t  *dst_ptr,
        uint32_t  dst_stride,
        uint32_t  width,
        uint32_t  height);
    EB_ENC_UnpackAvg_TYPE UnPackAvg_funcPtrArray[ASM_TYPE_TOTAL] =
    {
        // NON_AVX2
        UnpackAvg,
        // AVX2
        unpack_avg_avx2_intrin,//unpack_avg_sse2_intrin,

    };
    typedef void(*EB_ENC_UnpackAvgSub_TYPE)(
        uint16_t *ref16_l0,
        uint32_t  ref_l0_stride,
        uint16_t *ref16_l1,
        uint32_t  ref_l1_stride,
        uint8_t  *dst_ptr,
        uint32_t  dst_stride,
        EbBool      sub_pred,
        uint32_t  width,
        uint32_t  height);
    EB_ENC_UnpackAvgSub_TYPE UnPackAvgSafeSub_funcPtrArray[ASM_TYPE_TOTAL] =
    {
        // NON_AVX2
        UnpackAvgSafeSub,
        // AVX2  SafeSub
        unpack_avg_safe_sub_avx2_intrin,//unpack_avg_sse2_intrin,

    };

    typedef void(*EB_ENC_UnPack8BitData_TYPE)(
        uint16_t      *in16_bit_buffer,
        uint32_t       in_stride,
        uint8_t       *out8_bit_buffer,
        uint32_t       out8_stride,
        uint32_t       width,
        uint32_t       height);
    EB_ENC_UnPack8BitData_TYPE UnPack8BIT_funcPtrArray_16Bit[2][ASM_TYPE_TOTAL] =
    {
        {
           UnPack8BitData,
           UnPack8BitData,
        },
        {
            // NON_AVX2
            eb_enc_un_pack8_bit_data_sse2_intrin,
            // AVX2
            eb_enc_un_pack8_bit_data_sse2_intrin,
        }
    };
    typedef void(*EB_ENC_UnPack8BitDataSUB_TYPE)(
        uint16_t      *in16_bit_buffer,
        uint32_t       in_stride,
        uint8_t       *out8_bit_buffer,
        uint32_t       out8_stride,
        uint32_t       width,
        uint32_t       height,
        EbBool      sub_pred
        );
    EB_ENC_UnPack8BitDataSUB_TYPE UnPack8BITSafeSub_funcPtrArray_16Bit[ASM_TYPE_TOTAL] =
    {
        // NON_AVX2
        eb_enc_un_pack8_bit_data_safe_sub_sse2_intrin,
        // AVX2
        eb_enc_un_pack8_bit_data_safe_sub_sse2_intrin,

    };

#ifdef __cplusplus
}
#endif
#endif // EbPackUnPack_h
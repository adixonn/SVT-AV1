/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/
#ifndef EbPictureOperators_C_h
#define EbPictureOperators_C_h

#include "EbDefinitions.h"
#ifdef __cplusplus
extern "C" {
#endif

    void PictureCopyKernel(
        EbByte                  src,
        uint32_t                   src_stride,
        EbByte                  dst,
        uint32_t                   dst_stride,
        uint32_t                   area_width,
        uint32_t                   area_height,
        uint32_t                   bytesPerSample);

    uint64_t SpatialFullDistortionKernel(
        uint8_t   *input,
        uint32_t   inputStride,
        uint8_t   *recon,
        uint32_t   recon_stride,
        uint32_t   area_width,
        uint32_t   area_height);

    extern void PictureAdditionKernel(
        uint8_t  *pred_ptr,
        uint32_t  pred_stride,
        int32_t *residual_ptr,
        uint32_t  residual_stride,
        uint8_t  *recon_ptr,
        uint32_t  recon_stride,
        uint32_t  width,
        uint32_t  height,
        int32_t     bd);

#ifdef __cplusplus
}
#endif
#endif // EbPictureOperators_C_h
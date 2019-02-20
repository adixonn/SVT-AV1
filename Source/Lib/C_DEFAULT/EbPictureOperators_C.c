/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#include "EbPictureOperators_C.h"
#include "EbUtility.h"

/*********************************
* Picture Copy Kernel
*********************************/
void PictureCopyKernel(
    EbByte                  src,
    uint32_t                   src_stride,
    EbByte                  dst,
    uint32_t                   dst_stride,
    uint32_t                   area_width,
    uint32_t                   area_height,
    uint32_t                   bytesPerSample)  //=1 always)
{
    uint32_t sampleCount = 0;
    const uint32_t sampleTotalCount = area_width * area_height;
    const uint32_t copyLength = area_width * bytesPerSample;

    src_stride *= bytesPerSample;
    dst_stride *= bytesPerSample;

    while (sampleCount < sampleTotalCount) {
        EB_MEMCPY(dst, src, copyLength);
        src += src_stride;
        dst += dst_stride;
        sampleCount += area_width;
    }

    return;
}

// C equivalents

uint64_t SpatialFullDistortionKernel(
    uint8_t   *input,
    uint32_t   inputStride,
    uint8_t   *recon,
    uint32_t   recon_stride,
    uint32_t   area_width,
    uint32_t   area_height)
{
    uint32_t  columnIndex;
    uint32_t  rowIndex = 0;

    uint64_t  spatialDistortion = 0;

    while (rowIndex < area_height) {

        columnIndex = 0;
        while (columnIndex < area_width) {
            spatialDistortion += (int64_t)SQR((int64_t)(input[columnIndex]) - (recon[columnIndex]));
            ++columnIndex;
        }

        input += inputStride;
        recon += recon_stride;
        ++rowIndex;
    }
    return spatialDistortion;
}






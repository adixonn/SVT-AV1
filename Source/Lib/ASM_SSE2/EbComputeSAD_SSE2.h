/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#ifndef EbComputeSAD_SSE2_h
#define EbComputeSAD_SSE2_h

#include "EbDefinitions.h"
#ifdef __cplusplus
extern "C" {
#endif

    extern uint32_t compute4x_m_sad_sub_sse2_intrin(
        uint8_t  *src,                             // input parameter, source samples Ptr
        uint32_t  src_stride,                      // input parameter, source stride
        uint8_t  *ref,                             // input parameter, reference samples Ptr
        uint32_t  ref_stride,                      // input parameter, reference stride
        uint32_t  height,                          // input parameter, block height (M)
        uint32_t  width);                          // input parameter, block width (N)

    extern uint32_t combined_averaging4x_msad_sse2_intrin(
        uint8_t  *src,
        uint32_t  src_stride,
        uint8_t  *ref1,
        uint32_t  ref1_stride,
        uint8_t  *ref2,
        uint32_t  ref2_stride,
        uint32_t  height,
        uint32_t  width);

#ifdef __cplusplus
}
#endif
#endif // EbComputeSAD_SSE2_h
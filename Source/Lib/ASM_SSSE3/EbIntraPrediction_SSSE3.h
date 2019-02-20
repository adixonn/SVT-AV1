/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#ifndef EbIntraPrediction_SSSE3_h
#define EbIntraPrediction_SSSE3_h

#include "EbDefinitions.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void IntraModeAngular_Vertical_Kernel_SSSE3_INTRIN(
    uint32_t            size,
    uint8_t            *ref_samp_main,
    uint8_t            *prediction_ptr,
    uint32_t            prediction_buffer_stride,
    const EbBool     skip,
    int32_t            intra_pred_angle);

extern void IntraModeAngular_Horizontal_Kernel_SSSE3_INTRIN(
    uint32_t            size,
    uint8_t            *ref_samp_main,
    uint8_t            *prediction_ptr,
    uint32_t            prediction_buffer_stride,
    const EbBool     skip,
    int32_t            intra_pred_angle);


#ifdef __cplusplus
}
#endif
#endif // EbIntraPrediction_SSSE3_h
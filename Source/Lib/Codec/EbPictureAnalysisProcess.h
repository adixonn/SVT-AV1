/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#ifndef EbPictureAnalysis_h
#define EbPictureAnalysis_h

#include "EbDefinitions.h"
#include "EbSystemResourceManager.h"
#include "EbNoiseExtractAVX2.h"

/**************************************
 * Context
 **************************************/
typedef struct PictureAnalysisContext_s
{
    EB_ALIGN(64) uint8_t            local_cache[64];
    EbFifo_t                     *resourceCoordinationResultsInputFifoPtr;
    EbFifo_t                     *pictureAnalysisResultsOutputFifoPtr;
    EbPictureBufferDesc_t        *denoised_picture_ptr;
    EbPictureBufferDesc_t        *noise_picture_ptr;
    double                          picNoiseVarianceFloat;
} PictureAnalysisContext_t;

/***************************************
 * Extern Function Declaration
 ***************************************/
extern EbErrorType PictureAnalysisContextCtor(
    EbPictureBufferDescInitData_t * inputPictureBufferDescInitData,
    EbBool                         denoiseFlag,
    PictureAnalysisContext_t    **context_dbl_ptr,
    EbFifo_t                     *resourceCoordinationResultsInputFifoPtr,
    EbFifo_t                     *pictureAnalysisResultsOutputFifoPtr);

extern void* PictureAnalysisKernel(void *input_ptr);

void noiseExtractLumaWeak(
    EbPictureBufferDesc_t       *input_picture_ptr,
    EbPictureBufferDesc_t       *denoised_picture_ptr,
    EbPictureBufferDesc_t       *noise_picture_ptr,
    uint32_t                       sb_origin_y,
    uint32_t                         sb_origin_x
);
typedef void(*EB_WEAKLUMAFILTER_TYPE)(
    EbPictureBufferDesc_t       *input_picture_ptr,
    EbPictureBufferDesc_t       *denoised_picture_ptr,
    EbPictureBufferDesc_t       *noise_picture_ptr,
    uint32_t                       sb_origin_y,
    uint32_t                         sb_origin_x
    );
static EB_WEAKLUMAFILTER_TYPE FUNC_TABLE WeakLumaFilter_funcPtrArray[ASM_TYPE_TOTAL] =
{
    // NON_AVX2
    noiseExtractLumaWeak,
    // AVX2
    noise_extract_luma_weak_avx2_intrin,

};

void noiseExtractLumaWeakLcu(
    EbPictureBufferDesc_t       *input_picture_ptr,
    EbPictureBufferDesc_t       *denoised_picture_ptr,
    EbPictureBufferDesc_t       *noise_picture_ptr,
    uint32_t                       sb_origin_y,
    uint32_t                         sb_origin_x
);
static EB_WEAKLUMAFILTER_TYPE FUNC_TABLE WeakLumaFilterLcu_funcPtrArray[ASM_TYPE_TOTAL] =
{
    // NON_AVX2
    noiseExtractLumaWeakLcu,
    // AVX2
    noise_extract_luma_weak_lcu_avx2_intrin,

};


void noiseExtractLumaStrong(
    EbPictureBufferDesc_t       *input_picture_ptr,
    EbPictureBufferDesc_t       *denoised_picture_ptr,
    uint32_t                       sb_origin_y,
    uint32_t                         sb_origin_x
);
typedef void(*EB_STRONGLUMAFILTER_TYPE)(
    EbPictureBufferDesc_t       *input_picture_ptr,
    EbPictureBufferDesc_t       *denoised_picture_ptr,
    uint32_t                       sb_origin_y,
    uint32_t                         sb_origin_x
    );
static EB_STRONGLUMAFILTER_TYPE FUNC_TABLE StrongLumaFilter_funcPtrArray[ASM_TYPE_TOTAL] =
{
    // NON_AVX2
    noiseExtractLumaStrong,
    // AVX2
    noise_extract_luma_strong_avx2_intrin,

};
void noiseExtractChromaStrong(
    EbPictureBufferDesc_t       *input_picture_ptr,
    EbPictureBufferDesc_t       *denoised_picture_ptr,
    uint32_t                       sb_origin_y,
    uint32_t                         sb_origin_x
);
typedef void(*EB_STRONGCHROMAFILTER_TYPE)(
    EbPictureBufferDesc_t       *input_picture_ptr,
    EbPictureBufferDesc_t       *denoised_picture_ptr,
    uint32_t                       sb_origin_y,
    uint32_t                         sb_origin_x
    );
static EB_STRONGCHROMAFILTER_TYPE FUNC_TABLE StrongChromaFilter_funcPtrArray[ASM_TYPE_TOTAL] =
{
    // NON_AVX2
    noiseExtractChromaStrong,
    // AVX2
    noise_extract_chroma_strong_avx2_intrin,

};

void noiseExtractChromaWeak(
    EbPictureBufferDesc_t       *input_picture_ptr,
    EbPictureBufferDesc_t       *denoised_picture_ptr,
    uint32_t                       sb_origin_y,
    uint32_t                         sb_origin_x
);
typedef void(*EB_WEAKCHROMAFILTER_TYPE)(
    EbPictureBufferDesc_t       *input_picture_ptr,
    EbPictureBufferDesc_t       *denoised_picture_ptr,
    uint32_t                       sb_origin_y,
    uint32_t                         sb_origin_x
    );
static EB_WEAKCHROMAFILTER_TYPE FUNC_TABLE WeakChromaFilter_funcPtrArray[ASM_TYPE_TOTAL] =
{
    // NON_AVX2
    noiseExtractChromaWeak,
    // AVX2
    noise_extract_chroma_weak_avx2_intrin,

};


#endif // EbPictureAnalysis_h
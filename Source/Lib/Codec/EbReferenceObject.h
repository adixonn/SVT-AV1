/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#ifndef EbReferenceObject_h
#define EbReferenceObject_h

#include "EbDefinitions.h"
#include "EbDefinitions.h"
#include "EbAdaptiveMotionVectorPrediction.h"

typedef struct EbReferenceObject {
    EbPictureBufferDesc          *referencePicture;
    EbPictureBufferDesc          *referencePicture16bit;
    EbPictureBufferDesc          *refDenSrcPicture;

    TmvpUnit                     *tmvpMap;
    EbBool                          tmvpEnableFlag;
    uint64_t                        refPOC;
#if ADD_DELTA_QP_SUPPORT
    uint16_t                        qp;
#else
    uint8_t                         qp;
#endif
    EB_SLICE                        slice_type;
    uint8_t                         intra_coded_area;//percentage of intra coded area 0-100%
    uint8_t                         intra_coded_area_sb[MAX_NUMBER_OF_TREEBLOCKS_PER_PICTURE];//percentage of intra coded area 0-100%
    uint32_t                        non_moving_index_array[MAX_NUMBER_OF_TREEBLOCKS_PER_PICTURE];//array to hold non-moving blocks in reference frames
    uint32_t                        picSampleValue[MAX_NUMBER_OF_REGIONS_IN_WIDTH][MAX_NUMBER_OF_REGIONS_IN_HEIGHT][3];// [Y U V];
    EbBool                          penalizeSkipflag;
    uint8_t                         tmpLayerIdx;
    EbBool                          isSceneChange;
    uint16_t                        pic_avg_variance;
    uint8_t                         average_intensity;
    aom_film_grain_t                film_grain_params; //Film grain parameters for a reference frame
#if FAST_CDEF
    uint32_t                        cdef_frame_strength;
#endif
#if FAST_SG
    int8_t                          sg_frame_ep;
#endif
} EbReferenceObject;

typedef struct EbReferenceObjectDescInitData {
    EbPictureBufferDescInitData   referencePictureDescInitData;
} EbReferenceObjectDescInitData;

typedef struct EbPaReferenceObject {
    EbPictureBufferDesc          *inputPaddedPicturePtr;
    EbPictureBufferDesc          *quarterDecimatedPicturePtr;
    EbPictureBufferDesc          *sixteenthDecimatedPicturePtr;
    uint16_t                        variance[MAX_NUMBER_OF_TREEBLOCKS_PER_PICTURE];
    uint8_t                         yMean[MAX_NUMBER_OF_TREEBLOCKS_PER_PICTURE];
    EB_SLICE                        slice_type;
    uint32_t                        dependentPicturesCount; //number of pic using this reference frame
    PictureParentControlSet      *pPcsPtr;

} EbPaReferenceObject;

typedef struct EbPaReferenceObjectDescInitData {
    EbPictureBufferDescInitData   referencePictureDescInitData;
    EbPictureBufferDescInitData   quarterPictureDescInitData;
    EbPictureBufferDescInitData   sixteenthPictureDescInitData;
} EbPaReferenceObjectDescInitData;

/**************************************
 * Extern Function Declarations
 **************************************/
extern EbErrorType eb_reference_object_ctor(
    EbPtr *object_dbl_ptr,
    EbPtr  object_init_data_ptr);

extern EbErrorType eb_pa_reference_object_ctor(
    EbPtr *object_dbl_ptr,
    EbPtr  object_init_data_ptr);


#endif //EbReferenceObject_h
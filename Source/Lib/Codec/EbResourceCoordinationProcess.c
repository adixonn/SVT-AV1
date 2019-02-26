/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#include <stdlib.h>
#include <string.h>

#include "EbDefinitions.h"
#include "EbSystemResourceManager.h"
#include "EbPictureControlSet.h"
#include "EbSequenceControlSet.h"
#include "EbPictureBufferDesc.h"
#include "EbResourceCoordinationProcess.h"
#include "EbResourceCoordinationResults.h"
#include "EbTransforms.h"
#include "EbTime.h"

#if !ME_HME_OQ
// HME LEVEL 0
uint8_t enable_hme_level0_flag[5][MAX_SUPPORTED_MODES] = {
    { 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1 },
};
uint16_t hme_level0_total_search_area_width[5][MAX_SUPPORTED_MODES] = {
    { 48, 48, 32, 32, 32, 24, 24, 24 },
    { 64, 64, 48, 48, 48, 32, 32, 32 },
    { 96, 96, 64, 64, 64, 48, 48, 48 },
    { 96, 96, 64, 64, 48, 48, 48, 48 },
    { 128, 128, 96, 64, 64, 64, 64, 64 }
};
uint16_t hme_level0_total_search_area_height[5][MAX_SUPPORTED_MODES] = {
    { 24, 24, 16, 16, 16, 12, 12, 12 },
    { 32, 32, 32, 32, 32, 24, 24, 24 },
    { 32, 32, 32, 32, 32, 16, 16, 16 },
    { 48, 48, 48, 48, 32, 40, 40, 40 },
    { 80, 80, 64, 32, 32, 32, 32, 32 }
};
uint16_t HmeLevel0SearchAreaInWidthArray_Left[5][MAX_SUPPORTED_MODES] = {
    { 24, 24, 16, 16, 16, 12, 12, 12 },
    { 32, 32, 24, 24, 24, 16, 16, 16 },
    { 48, 48, 32, 32, 32, 24, 24, 24 },
    { 48, 48, 32, 32, 24, 24, 24, 24 },
    { 64, 64, 48, 32, 32, 32, 32, 32 }
};
uint16_t HmeLevel0SearchAreaInWidthArray_Right[5][MAX_SUPPORTED_MODES] = {
    { 24, 24, 16, 16, 16, 12, 12, 12 },
    { 32, 32, 24, 24, 24, 16, 16, 16 },
    { 48, 48, 32, 32, 32, 24, 24, 24 },
    { 48, 48, 32, 32, 24, 24, 24, 24 },
    { 64, 64, 48, 32, 32, 32, 32, 32 }
};
uint16_t HmeLevel0SearchAreaInHeightArray_Top[5][MAX_SUPPORTED_MODES] = {
    { 12, 12, 8, 8, 8, 6, 6, 6 },
    { 16, 16, 16, 16, 16, 12, 12, 12 },
    { 16, 16, 16, 16, 16, 8, 8, 8 },
    { 24, 24, 24, 24, 16, 20, 20, 20 },
    { 40, 40, 32, 16, 16, 16, 16, 16 }
};
uint16_t HmeLevel0SearchAreaInHeightArray_Bottom[5][MAX_SUPPORTED_MODES] = {
    { 12, 12, 8, 8, 8, 6, 6, 6 },
    { 16, 16, 16, 16, 16, 12, 12, 12 },
    { 16, 16, 16, 16, 16, 8, 8, 8 },
    { 24, 24, 24, 24, 16, 20, 20, 20 },
    { 40, 40, 32, 16, 16, 16, 16, 16 }
};
// HME LEVEL 1
uint8_t enable_hme_level1_flag[5][MAX_SUPPORTED_MODES] = {
    { 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1, 1, 1, 1 },
    { 1, 1, 0, 0, 0, 0, 0, 0 }
};
uint16_t HmeLevel1SearchAreaInWidthArray_Left[5][MAX_SUPPORTED_MODES] = {
    { 16, 16, 16, 16, 8, 8, 8, 8 },
    { 16, 16, 16, 16, 8, 8, 8, 8 },
    { 16, 16, 16, 16, 8, 8, 8, 8 },
    { 16, 16, 8, 8, 8, 8, 8, 8 },
    { 16, 16, 0, 0, 0, 0, 0, 0 }
};
uint16_t HmeLevel1SearchAreaInWidthArray_Right[5][MAX_SUPPORTED_MODES] = {
    { 16, 16, 16, 16, 8, 8, 8, 8 },
    { 16, 16, 16, 16, 8, 8, 8, 8 },
    { 16, 16, 16, 16, 8, 8, 8, 8 },
    { 16, 16, 8, 8, 8, 8, 8, 8 },
    { 16, 16, 0, 0, 0, 0, 0, 0 }
};
uint16_t HmeLevel1SearchAreaInHeightArray_Top[5][MAX_SUPPORTED_MODES] = {
    { 7, 7, 7, 7, 3, 3, 3, 3 },
    { 7, 7, 7, 7, 3, 3, 3, 3 },
    { 7, 7, 7, 7, 3, 3, 3, 3 },
    { 7, 7, 3, 3, 3, 3, 3, 3 },
    { 9, 9, 0, 0, 0, 0, 0, 0 }
};
uint16_t HmeLevel1SearchAreaInHeightArray_Bottom[5][MAX_SUPPORTED_MODES] = {
    { 7, 7, 7, 7, 3, 3, 3, 3 },
    { 7, 7, 7, 7, 3, 3, 3, 3 },
    { 7, 7, 7, 7, 3, 3, 3, 3 },
    { 7, 7, 3, 3, 3, 3, 3, 3 },
    { 9, 9, 0, 0, 0, 0, 0, 0 }
};
// HME LEVEL 2
uint8_t enable_hme_level2_flag[5][MAX_SUPPORTED_MODES] = {
    { 1, 1, 1, 1, 0, 0, 0, 0 },
    { 1, 1, 1, 1, 0, 0, 0, 0 },
    { 1, 1, 1, 1, 0, 0, 0, 0 },
    { 1, 1, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }
};
uint16_t HmeLevel2SearchAreaInWidthArray_Left[5][MAX_SUPPORTED_MODES] = {
    { 8, 8, 8, 8, 0, 0, 0, 0 },
    { 8, 8, 8, 8, 0, 0, 0, 0 },
    { 8, 8, 8, 8, 0, 0, 0, 0 },
    { 8, 8, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }
};
uint16_t HmeLevel2SearchAreaInWidthArray_Right[5][MAX_SUPPORTED_MODES] = {
    { 8, 8, 8, 8, 0, 0, 0, 0 },
    { 8, 8, 8, 8, 0, 0, 0, 0 },
    { 8, 8, 8, 8, 0, 0, 0, 0 },
    { 8, 8, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }
};
uint16_t HmeLevel2SearchAreaInHeightArray_Top[5][MAX_SUPPORTED_MODES] = {
    { 3, 3, 3, 3, 0, 0, 0, 0 },
    { 3, 3, 3, 3, 0, 0, 0, 0 },
    { 3, 3, 3, 3, 0, 0, 0, 0 },
    { 3, 3, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }
};
uint16_t HmeLevel2SearchAreaInHeightArray_Bottom[5][MAX_SUPPORTED_MODES] = {
    { 3, 3, 3, 3, 0, 0, 0, 0 },
    { 3, 3, 3, 3, 0, 0, 0, 0 },
    { 3, 3, 3, 3, 0, 0, 0, 0 },
    { 3, 3, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }
};

uint8_t search_area_width[5][MAX_SUPPORTED_MODES] = {

    { 16, 16, 16, 16, 8, 8, 8, 8 },
    { 16, 16, 16, 16, 8, 8, 8, 8 },
    { 16, 16, 16, 16, 8, 8, 8, 8 },
    { 16, 16, 8, 8, 8, 8, 8, 8 },
    { 24, 24, 16, 16, 16, 8, 8, 8 }

};

uint8_t search_area_height[5][MAX_SUPPORTED_MODES] = {

    {  7, 7, 7, 7, 5, 5, 5, 5 },
    { 9, 9, 9, 9, 7, 7, 7, 7 },
    { 7, 7, 7, 7, 5, 5, 5, 5 },
    { 13, 13, 7, 7, 7, 7, 5, 5 },
    { 23, 23, 13, 7, 7, 7, 7, 7 }

};


#endif
/************************************************
 * Resource Coordination Context Constructor
 ************************************************/
EbErrorType resource_coordination_context_ctor(
    ResourceCoordinationContext  **context_dbl_ptr,
    EbFifo                        *inputBufferFifoPtr,
    EbFifo                        *resource_coordination_results_output_fifo_ptr,
    EbFifo                        **picture_control_set_fifo_ptr_array,
    EbSequenceControlSetInstance  **sequence_control_set_instance_array,
    EbFifo                         *sequence_control_set_empty_fifo_ptr,
    EbCallback                    **app_callback_ptr_array,
    uint32_t                         *compute_segments_total_count_array,
    uint32_t                          encode_instances_total_count)
{
    uint32_t instance_index;

    ResourceCoordinationContext *context_ptr;
    EB_MALLOC(ResourceCoordinationContext*, context_ptr, sizeof(ResourceCoordinationContext), EB_N_PTR);

    *context_dbl_ptr = context_ptr;

    context_ptr->input_buffer_fifo_ptr = inputBufferFifoPtr;
    context_ptr->resource_coordination_results_output_fifo_ptr = resource_coordination_results_output_fifo_ptr;
    context_ptr->picture_control_set_fifo_ptr_array = picture_control_set_fifo_ptr_array;
    context_ptr->sequence_control_set_instance_array = sequence_control_set_instance_array;
    context_ptr->sequence_control_set_empty_fifo_ptr = sequence_control_set_empty_fifo_ptr;
    context_ptr->app_callback_ptr_array = app_callback_ptr_array;
    context_ptr->compute_segments_total_count_array = compute_segments_total_count_array;
    context_ptr->encode_instances_total_count = encode_instances_total_count;

    // Allocate SequenceControlSetActiveArray
    EB_MALLOC(EbObjectWrapper**, context_ptr->sequenceControlSetActiveArray, sizeof(EbObjectWrapper*) * context_ptr->encode_instances_total_count, EB_N_PTR);

    for (instance_index = 0; instance_index < context_ptr->encode_instances_total_count; ++instance_index) {
        context_ptr->sequenceControlSetActiveArray[instance_index] = 0;
    }

    // Picture Stats
    EB_MALLOC(uint64_t*, context_ptr->pictureNumberArray, sizeof(uint64_t) * context_ptr->encode_instances_total_count, EB_N_PTR);

    for (instance_index = 0; instance_index < context_ptr->encode_instances_total_count; ++instance_index) {
        context_ptr->pictureNumberArray[instance_index] = 0;
    }

    context_ptr->averageEncMod = 0;
    context_ptr->prevEncMod = 0;
    context_ptr->prevEncModeDelta = 0;
    context_ptr->curSpeed = 0; // speed x 1000
    context_ptr->previousModeChangeBuffer = 0;
    context_ptr->firstInPicArrivedTimeSeconds = 0;
    context_ptr->firstInPicArrivedTimeuSeconds = 0;
    context_ptr->previousFrameInCheck1 = 0;
    context_ptr->previousFrameInCheck2 = 0;
    context_ptr->previousFrameInCheck3 = 0;
    context_ptr->previousModeChangeFrameIn = 0;
    context_ptr->prevsTimeSeconds = 0;
    context_ptr->prevsTimeuSeconds = 0;
    context_ptr->prevFrameOut = 0;
    context_ptr->startFlag = EB_FALSE;

    context_ptr->previousBufferCheck1 = 0;
    context_ptr->prevChangeCond = 0;

    return EB_ErrorNone;
}

#if !ME_HME_OQ
// Set ME/HME Params
void* SetMeHmeParams(
    PictureParentControlSet       *picture_control_set_ptr,
    SequenceControlSet                *sequence_control_set_ptr,
    EbInputResolution                 input_resolution)
{

    uint8_t  encModeIndex = picture_control_set_ptr->enc_mode;

    uint32_t inputRatio = sequence_control_set_ptr->luma_width / sequence_control_set_ptr->luma_height;

    uint8_t resolutionIndex = input_resolution <= INPUT_SIZE_576p_RANGE_OR_LOWER ? 0 : // 480P
        (input_resolution <= INPUT_SIZE_1080i_RANGE && inputRatio < 2) ? 1 : // 720P
        (input_resolution <= INPUT_SIZE_1080i_RANGE && inputRatio > 3) ? 2 : // 1080I
        (input_resolution <= INPUT_SIZE_1080p_RANGE) ? 3 : // 1080I
        4; // 4K

#if V2_HME_ME_SR
    if (picture_control_set_ptr->enc_mode > ENC_M1 /*|| sequence_control_set_ptr->static_config.tune == TUNE_VQ*/) {

#endif
        // HME/ME default settings
        picture_control_set_ptr->enable_hme_flag = EB_TRUE;
        picture_control_set_ptr->number_hme_search_region_in_width = 2;
        picture_control_set_ptr->number_hme_search_region_in_height = 2;
        picture_control_set_ptr->enable_hme_level0_flag = enable_hme_level0_flag[resolutionIndex][encModeIndex];
        picture_control_set_ptr->hme_level0_total_search_area_width = hme_level0_total_search_area_width[resolutionIndex][encModeIndex];
        picture_control_set_ptr->hme_level0_total_search_area_height = hme_level0_total_search_area_height[resolutionIndex][encModeIndex];
        picture_control_set_ptr->hme_level0_search_area_in_width_array[0] = HmeLevel0SearchAreaInWidthArray_Right[resolutionIndex][encModeIndex];
        picture_control_set_ptr->hme_level0_search_area_in_width_array[1] = HmeLevel0SearchAreaInWidthArray_Left[resolutionIndex][encModeIndex];
        picture_control_set_ptr->hme_level0_search_area_in_height_array[0] = HmeLevel0SearchAreaInHeightArray_Top[resolutionIndex][encModeIndex];
        picture_control_set_ptr->hme_level0_search_area_in_height_array[1] = HmeLevel0SearchAreaInHeightArray_Bottom[resolutionIndex][encModeIndex];
        picture_control_set_ptr->enable_hme_level1_flag = enable_hme_level1_flag[resolutionIndex][encModeIndex];
        picture_control_set_ptr->hme_level1_search_area_in_width_array[0] = HmeLevel1SearchAreaInWidthArray_Right[resolutionIndex][encModeIndex];
        picture_control_set_ptr->hme_level1_search_area_in_width_array[1] = HmeLevel1SearchAreaInWidthArray_Left[resolutionIndex][encModeIndex];
        picture_control_set_ptr->hme_level1_search_area_in_height_array[0] = HmeLevel1SearchAreaInHeightArray_Top[resolutionIndex][encModeIndex];
        picture_control_set_ptr->hme_level1_search_area_in_height_array[1] = HmeLevel1SearchAreaInHeightArray_Bottom[resolutionIndex][encModeIndex];
        picture_control_set_ptr->enable_hme_level2_flag = enable_hme_level2_flag[resolutionIndex][encModeIndex];
        picture_control_set_ptr->hme_level2_search_area_in_width_array[0] = HmeLevel2SearchAreaInWidthArray_Right[resolutionIndex][encModeIndex];
        picture_control_set_ptr->hme_level2_search_area_in_width_array[1] = HmeLevel2SearchAreaInWidthArray_Left[resolutionIndex][encModeIndex];
        picture_control_set_ptr->hme_level2_search_area_in_height_array[0] = HmeLevel2SearchAreaInHeightArray_Top[resolutionIndex][encModeIndex];
        picture_control_set_ptr->hme_level2_search_area_in_height_array[1] = HmeLevel2SearchAreaInHeightArray_Bottom[resolutionIndex][encModeIndex];


        picture_control_set_ptr->search_area_width = search_area_width[resolutionIndex][encModeIndex];
        picture_control_set_ptr->search_area_height = search_area_height[resolutionIndex][encModeIndex];


        // HME Level0 adjustment for low frame rate contents (frame rate <= 30)

        if (input_resolution == INPUT_SIZE_4K_RANGE) {
            if ((sequence_control_set_ptr->static_config.frame_rate >> 16) <= 30) {

                if (picture_control_set_ptr->enc_mode == ENC_M2) {
                    picture_control_set_ptr->hme_level0_total_search_area_width = 124;
                    picture_control_set_ptr->hme_level0_total_search_area_height = 64;
                    picture_control_set_ptr->hme_level0_search_area_in_width_array[0] = 62;
                    picture_control_set_ptr->hme_level0_search_area_in_width_array[1] = 62;
                    picture_control_set_ptr->hme_level0_search_area_in_height_array[0] = 32;
                    picture_control_set_ptr->hme_level0_search_area_in_height_array[1] = 32;
                }
                else if (picture_control_set_ptr->enc_mode == ENC_M3) {
                    picture_control_set_ptr->hme_level0_total_search_area_width = 96;
                    picture_control_set_ptr->hme_level0_total_search_area_height = 64;
                    picture_control_set_ptr->hme_level0_search_area_in_width_array[0] = 48;
                    picture_control_set_ptr->hme_level0_search_area_in_width_array[1] = 48;
                    picture_control_set_ptr->hme_level0_search_area_in_height_array[0] = 32;
                    picture_control_set_ptr->hme_level0_search_area_in_height_array[1] = 32;
                }
                else if (picture_control_set_ptr->enc_mode == ENC_M4) {
                    picture_control_set_ptr->hme_level0_total_search_area_width = 64;
                    picture_control_set_ptr->hme_level0_total_search_area_height = 48;
                    picture_control_set_ptr->hme_level0_search_area_in_width_array[0] = 32;
                    picture_control_set_ptr->hme_level0_search_area_in_width_array[1] = 32;
                    picture_control_set_ptr->hme_level0_search_area_in_height_array[0] = 24;
                    picture_control_set_ptr->hme_level0_search_area_in_height_array[1] = 24;
                }
                else if (picture_control_set_ptr->enc_mode >= ENC_M5) {
                    picture_control_set_ptr->hme_level0_total_search_area_width = 64;
                    picture_control_set_ptr->hme_level0_total_search_area_height = 48;
                    picture_control_set_ptr->hme_level0_search_area_in_width_array[0] = 32;
                    picture_control_set_ptr->hme_level0_search_area_in_width_array[1] = 32;
                    picture_control_set_ptr->hme_level0_search_area_in_height_array[0] = 24;
                    picture_control_set_ptr->hme_level0_search_area_in_height_array[1] = 24;
                }
            }
        }
#if V2_HME_ME_SR
    }
    else {
        picture_control_set_ptr->enable_hme_flag = EB_TRUE;
        picture_control_set_ptr->enable_hme_level0_flag = EB_TRUE;
        picture_control_set_ptr->number_hme_search_region_in_width = 1;
        picture_control_set_ptr->number_hme_search_region_in_height = 1;
        picture_control_set_ptr->hme_level0_total_search_area_width = 64;
        picture_control_set_ptr->hme_level0_total_search_area_height = 64;
        picture_control_set_ptr->hme_level0_search_area_in_width_array[0] = 64;
        picture_control_set_ptr->hme_level0_search_area_in_height_array[0] = 64;
        picture_control_set_ptr->enable_hme_level1_flag = EB_TRUE;
        picture_control_set_ptr->hme_level1_search_area_in_width_array[0] = 32;
        picture_control_set_ptr->hme_level1_search_area_in_height_array[0] = 32;
        picture_control_set_ptr->enable_hme_level2_flag = EB_TRUE;
        picture_control_set_ptr->hme_level2_search_area_in_width_array[0] = 4;
        picture_control_set_ptr->hme_level2_search_area_in_height_array[0] = 4;
        picture_control_set_ptr->search_area_width = 32;
        picture_control_set_ptr->search_area_height = 16;

#if ME_128x128
        picture_control_set_ptr->search_area_width = 128;
        picture_control_set_ptr->search_area_height = 128;
#endif
#if ME_64x64
        picture_control_set_ptr->search_area_width = 64;
        picture_control_set_ptr->search_area_height = 64;
#endif
    }
#endif


    return EB_NULL;
};
#else
/******************************************************
* Derive Pre-Analysis settings for OQ
Input   : encoder mode and tune
Output  : Pre-Analysis signal(s)
******************************************************/
EbErrorType signal_derivation_pre_analysis_oq(
    SequenceControlSet       *sequence_control_set_ptr,
    PictureParentControlSet  *picture_control_set_ptr) {

    EbErrorType return_error = EB_ErrorNone;
    uint8_t input_resolution = sequence_control_set_ptr->input_resolution;

    // Derive HME Flag
    if (sequence_control_set_ptr->static_config.use_default_me_hme) {
        uint8_t  hme_me_level = picture_control_set_ptr->enc_mode;

        picture_control_set_ptr->enable_hme_flag = EB_TRUE;
        picture_control_set_ptr->enable_hme_level0_flag = enable_hme_level0_flag[input_resolution][hme_me_level];
        picture_control_set_ptr->enable_hme_level1_flag = enable_hme_level1_flag[input_resolution][hme_me_level];
        picture_control_set_ptr->enable_hme_level2_flag = enable_hme_level2_flag[input_resolution][hme_me_level];
    }
    else {
        picture_control_set_ptr->enable_hme_flag = sequence_control_set_ptr->static_config.enable_hme_flag;
        picture_control_set_ptr->enable_hme_level0_flag = sequence_control_set_ptr->static_config.enable_hme_level0_flag;
        picture_control_set_ptr->enable_hme_level1_flag = sequence_control_set_ptr->static_config.enable_hme_level1_flag;
        picture_control_set_ptr->enable_hme_level2_flag = sequence_control_set_ptr->static_config.enable_hme_level2_flag;
    }

    return return_error;
}

#endif
//******************************************************************************//
// Modify the Enc mode based on the buffer Status
// Inputs: TargetSpeed, Status of the SCbuffer
// Output: EncMod
//******************************************************************************//
void SpeedBufferControl(
    ResourceCoordinationContext   *context_ptr,
    PictureParentControlSet       *picture_control_set_ptr,
    SequenceControlSet            *sequence_control_set_ptr)
{

    uint64_t cursTimeSeconds = 0;
    uint64_t cursTimeuSeconds = 0;
    double overallDuration = 0.0;
    double instDuration = 0.0;
    int8_t  encoderModeDelta = 0;
    int64_t inputFramesCount = 0;
    int8_t changeCond = 0;
    int64_t targetFps = (sequence_control_set_ptr->static_config.injector_frame_rate >> 16);


    int64_t bufferTrshold1 = SC_FRAMES_INTERVAL_T1;
    int64_t bufferTrshold2 = SC_FRAMES_INTERVAL_T2;
    int64_t bufferTrshold3 = MIN(targetFps * 3, SC_FRAMES_INTERVAL_T3);
    eb_block_on_mutex(sequence_control_set_ptr->encode_context_ptr->sc_buffer_mutex);

    if (sequence_control_set_ptr->encode_context_ptr->sc_frame_in == 0) {
        eb_start_time(&context_ptr->firstInPicArrivedTimeSeconds, &context_ptr->firstInPicArrivedTimeuSeconds);
    }
    else if (sequence_control_set_ptr->encode_context_ptr->sc_frame_in == SC_FRAMES_TO_IGNORE) {
        context_ptr->startFlag = EB_TRUE;
    }

    // Compute duration since the start of the encode and since the previous checkpoint
    eb_finish_time(&cursTimeSeconds, &cursTimeuSeconds);

    eb_compute_overall_elapsed_time_ms(
        context_ptr->firstInPicArrivedTimeSeconds,
        context_ptr->firstInPicArrivedTimeuSeconds,
        cursTimeSeconds,
        cursTimeuSeconds,
        &overallDuration);

    eb_compute_overall_elapsed_time_ms(
        context_ptr->prevsTimeSeconds,
        context_ptr->prevsTimeuSeconds,
        cursTimeSeconds,
        cursTimeuSeconds,
        &instDuration);

    inputFramesCount = (int64_t)overallDuration *(sequence_control_set_ptr->static_config.injector_frame_rate >> 16) / 1000;
    sequence_control_set_ptr->encode_context_ptr->sc_buffer = inputFramesCount - sequence_control_set_ptr->encode_context_ptr->sc_frame_in;

    encoderModeDelta = 0;

    // Check every bufferTsshold1 for the changes (previousFrameInCheck1 variable)
    if ((sequence_control_set_ptr->encode_context_ptr->sc_frame_in > context_ptr->previousFrameInCheck1 + bufferTrshold1 && sequence_control_set_ptr->encode_context_ptr->sc_frame_in >= SC_FRAMES_TO_IGNORE)) {
        // Go to a slower mode based on the fullness and changes of the buffer
        if (sequence_control_set_ptr->encode_context_ptr->sc_buffer < targetFps && (context_ptr->prevEncModeDelta > -1 || (context_ptr->prevEncModeDelta < 0 && sequence_control_set_ptr->encode_context_ptr->sc_frame_in > context_ptr->previousModeChangeFrameIn + targetFps * 2))) {
            if (context_ptr->previousBufferCheck1 > sequence_control_set_ptr->encode_context_ptr->sc_buffer + bufferTrshold1) {
                encoderModeDelta += -1;
                changeCond = 2;
            }
            else if (context_ptr->previousModeChangeBuffer > bufferTrshold1 + sequence_control_set_ptr->encode_context_ptr->sc_buffer && sequence_control_set_ptr->encode_context_ptr->sc_buffer < bufferTrshold1) {
                encoderModeDelta += -1;
                changeCond = 4;
            }
        }

        // Go to a faster mode based on the fullness and changes of the buffer
        if (sequence_control_set_ptr->encode_context_ptr->sc_buffer > bufferTrshold1 + context_ptr->previousBufferCheck1) {
            encoderModeDelta += +1;
            changeCond = 1;
        }
        else if (sequence_control_set_ptr->encode_context_ptr->sc_buffer > bufferTrshold1 + context_ptr->previousModeChangeBuffer) {
            encoderModeDelta += +1;
            changeCond = 3;
        }

        // Update the encode mode based on the fullness of the buffer
        // If previous ChangeCond was the same, double the threshold2
        if (sequence_control_set_ptr->encode_context_ptr->sc_buffer > bufferTrshold3 &&
            (context_ptr->prevChangeCond != 7 || sequence_control_set_ptr->encode_context_ptr->sc_frame_in > context_ptr->previousModeChangeFrameIn + bufferTrshold2 * 2) &&
            sequence_control_set_ptr->encode_context_ptr->sc_buffer > context_ptr->previousModeChangeBuffer) {
            encoderModeDelta += 1;
            changeCond = 7;
        }
        encoderModeDelta = CLIP3(-1, 1, encoderModeDelta);
        sequence_control_set_ptr->encode_context_ptr->enc_mode = (EbEncMode)CLIP3(1, 6, (int8_t)sequence_control_set_ptr->encode_context_ptr->enc_mode + encoderModeDelta);

        // Update previous stats
        context_ptr->previousFrameInCheck1 = sequence_control_set_ptr->encode_context_ptr->sc_frame_in;
        context_ptr->previousBufferCheck1 = sequence_control_set_ptr->encode_context_ptr->sc_buffer;

        if (encoderModeDelta) {
            context_ptr->previousModeChangeBuffer = sequence_control_set_ptr->encode_context_ptr->sc_buffer;
            context_ptr->previousModeChangeFrameIn = sequence_control_set_ptr->encode_context_ptr->sc_frame_in;
            context_ptr->prevEncModeDelta = encoderModeDelta;
        }
    }

    // Check every bufferTrshold2 for the changes (previousFrameInCheck2 variable)
    if ((sequence_control_set_ptr->encode_context_ptr->sc_frame_in > context_ptr->previousFrameInCheck2 + bufferTrshold2 && sequence_control_set_ptr->encode_context_ptr->sc_frame_in >= SC_FRAMES_TO_IGNORE)) {
        encoderModeDelta = 0;

        // if no change in the encoder mode and buffer is low enough and level is not increasing, switch to a slower encoder mode
        // If previous ChangeCond was the same, double the threshold2
        if (encoderModeDelta == 0 && sequence_control_set_ptr->encode_context_ptr->sc_frame_in > context_ptr->previousModeChangeFrameIn + bufferTrshold2 &&
            (context_ptr->prevChangeCond != 8 || sequence_control_set_ptr->encode_context_ptr->sc_frame_in > context_ptr->previousModeChangeFrameIn + bufferTrshold2 * 2) &&
            ((sequence_control_set_ptr->encode_context_ptr->sc_buffer - context_ptr->previousModeChangeBuffer < (targetFps / 3)) || context_ptr->previousModeChangeBuffer == 0) &&
            sequence_control_set_ptr->encode_context_ptr->sc_buffer < bufferTrshold3) {
            encoderModeDelta = -1;
            changeCond = 8;
        }

        encoderModeDelta = CLIP3(-1, 1, encoderModeDelta);
        sequence_control_set_ptr->encode_context_ptr->enc_mode = (EbEncMode)CLIP3(1, 6, (int8_t)sequence_control_set_ptr->encode_context_ptr->enc_mode + encoderModeDelta);

        // Update previous stats
        context_ptr->previousFrameInCheck2 = sequence_control_set_ptr->encode_context_ptr->sc_frame_in;

        if (encoderModeDelta) {
            context_ptr->previousModeChangeBuffer = sequence_control_set_ptr->encode_context_ptr->sc_buffer;
            context_ptr->previousModeChangeFrameIn = sequence_control_set_ptr->encode_context_ptr->sc_frame_in;
            context_ptr->prevEncModeDelta = encoderModeDelta;
        }

    }
    // Check every SC_FRAMES_INTERVAL_SPEED frames for the speed calculation (previousFrameInCheck3 variable)
    if (context_ptr->startFlag || (sequence_control_set_ptr->encode_context_ptr->sc_frame_in > context_ptr->previousFrameInCheck3 + SC_FRAMES_INTERVAL_SPEED && sequence_control_set_ptr->encode_context_ptr->sc_frame_in >= SC_FRAMES_TO_IGNORE)) {
        if (context_ptr->startFlag) {
            context_ptr->curSpeed = (uint64_t)(sequence_control_set_ptr->encode_context_ptr->sc_frame_out - 0) * 1000 / (uint64_t)(overallDuration);
        }
        else {

            if (instDuration != 0)
                context_ptr->curSpeed = (uint64_t)(sequence_control_set_ptr->encode_context_ptr->sc_frame_out - context_ptr->prevFrameOut) * 1000 / (uint64_t)(instDuration);
        }
        context_ptr->startFlag = EB_FALSE;

        // Update previous stats
        context_ptr->previousFrameInCheck3 = sequence_control_set_ptr->encode_context_ptr->sc_frame_in;
        context_ptr->prevsTimeSeconds = cursTimeSeconds;
        context_ptr->prevsTimeuSeconds = cursTimeuSeconds;
        context_ptr->prevFrameOut = sequence_control_set_ptr->encode_context_ptr->sc_frame_out;

    }
    else if (sequence_control_set_ptr->encode_context_ptr->sc_frame_in < SC_FRAMES_TO_IGNORE && (overallDuration != 0)) {
        context_ptr->curSpeed = (uint64_t)(sequence_control_set_ptr->encode_context_ptr->sc_frame_out - 0) * 1000 / (uint64_t)(overallDuration);
    }

    if (changeCond) {
        context_ptr->prevChangeCond = changeCond;
    }
    sequence_control_set_ptr->encode_context_ptr->sc_frame_in++;
    if (sequence_control_set_ptr->encode_context_ptr->sc_frame_in >= SC_FRAMES_TO_IGNORE) {
        context_ptr->averageEncMod += sequence_control_set_ptr->encode_context_ptr->enc_mode;
    }
    else {
        context_ptr->averageEncMod = 0;
    }

    // Set the encoder level
    picture_control_set_ptr->enc_mode = sequence_control_set_ptr->encode_context_ptr->enc_mode;

    eb_release_mutex(sequence_control_set_ptr->encode_context_ptr->sc_buffer_mutex);
    context_ptr->prevEncMod = sequence_control_set_ptr->encode_context_ptr->enc_mode;
}

void ResetPcsAv1(
    PictureParentControlSet       *picture_control_set_ptr) {
    picture_control_set_ptr->is_skip_mode_allowed = 0;
    picture_control_set_ptr->skip_mode_flag = 0;
    picture_control_set_ptr->av1FrameType = KEY_FRAME;
    picture_control_set_ptr->showFrame = 1;
    picture_control_set_ptr->showable_frame = 1;  // frame can be used as show existing frame in future
    // Flag for a frame used as a reference - not written to the bitstream
    picture_control_set_ptr->is_reference_frame = 0;
    // Flag signaling that the frame is encoded using only INTRA modes.
    picture_control_set_ptr->intra_only = 0;
    // uint8_t last_intra_only;

    picture_control_set_ptr->disable_cdf_update = 0;
    picture_control_set_ptr->allow_high_precision_mv = 0;
    picture_control_set_ptr->cur_frame_force_integer_mv = 0;  // 0 the default in AOM, 1 only integer
    picture_control_set_ptr->allow_screen_content_tools = 0;
    picture_control_set_ptr->allow_intrabc = 0;
    picture_control_set_ptr->allow_warped_motion = 0;

    /* profile settings */
    picture_control_set_ptr->TxMode = TX_MODE_LARGEST;

#if CONFIG_ENTROPY_STATS
    int32_t coef_cdf_category;
#endif

    picture_control_set_ptr->base_qindex = 31;
    picture_control_set_ptr->y_dc_delta_q = 0;
    picture_control_set_ptr->u_dc_delta_q = 0;
    picture_control_set_ptr->v_dc_delta_q = 0;
    picture_control_set_ptr->u_ac_delta_q = 0;
    picture_control_set_ptr->v_ac_delta_q = 0;

    picture_control_set_ptr->separate_uv_delta_q = 0;
    // Encoder
    picture_control_set_ptr->using_qmatrix = 0;
    picture_control_set_ptr->qm_y = 5;
    picture_control_set_ptr->qm_u = 5;
    picture_control_set_ptr->qm_v = 5;
    // Whether to use previous frame's motion vectors for prediction.
    picture_control_set_ptr->allow_ref_frame_mvs = 0;
    picture_control_set_ptr->switchable_motion_mode = 0;
    // Flag signaling how frame contexts should be updated at the end of
    // a frame decode
    picture_control_set_ptr->refresh_frame_context = REFRESH_FRAME_CONTEXT_DISABLED;

    picture_control_set_ptr->lf.filter_level[0] = 0;
    picture_control_set_ptr->lf.filter_level[1] = 0;
    picture_control_set_ptr->lf.filter_level_u = 0;
    picture_control_set_ptr->lf.filter_level_v = 0;
    picture_control_set_ptr->lf.sharpness_level = 0;

    picture_control_set_ptr->lf.mode_ref_delta_enabled = 0;
    picture_control_set_ptr->lf.mode_ref_delta_update = 0;
    picture_control_set_ptr->lf.mode_deltas[0] = 0;
    picture_control_set_ptr->lf.mode_deltas[1] = 0;

    picture_control_set_ptr->lf.ref_deltas[0] = 1;
    picture_control_set_ptr->lf.ref_deltas[1] = 0;
    picture_control_set_ptr->lf.ref_deltas[2] = 0;
    picture_control_set_ptr->lf.ref_deltas[3] = 0;
    picture_control_set_ptr->lf.ref_deltas[4] = -1;
    picture_control_set_ptr->lf.ref_deltas[5] = 0;
    picture_control_set_ptr->lf.ref_deltas[6] = -1;
    picture_control_set_ptr->lf.ref_deltas[7] = -1;

    picture_control_set_ptr->all_lossless = 0;
    picture_control_set_ptr->coded_lossless = 0;
    picture_control_set_ptr->reduced_tx_set_used = 0;
    picture_control_set_ptr->reference_mode = SINGLE_REFERENCE;
    picture_control_set_ptr->frame_context_idx = 0; /* Context to use/update */
    for (int32_t i = 0; i < REF_FRAMES; i++) {
        picture_control_set_ptr->fb_of_context_type[i] = 0;
    }
    picture_control_set_ptr->primary_ref_frame = PRIMARY_REF_NONE;
    picture_control_set_ptr->frame_offset = picture_control_set_ptr->picture_number;
    picture_control_set_ptr->error_resilient_mode = 0;
    picture_control_set_ptr->uniform_tile_spacing_flag = 1;
    picture_control_set_ptr->large_scale_tile = 0;
    picture_control_set_ptr->film_grain_params_present = 0;
    picture_control_set_ptr->cdef_pri_damping = 0;
    picture_control_set_ptr->cdef_sec_damping = 0;
    picture_control_set_ptr->nb_cdef_strengths = 1;
    for (int32_t i = 0; i < CDEF_MAX_STRENGTHS; i++) {
        picture_control_set_ptr->cdef_strengths[i] = 0;
        picture_control_set_ptr->cdef_uv_strengths[i] = 0;
    }
    picture_control_set_ptr->cdef_bits = 0;


#if ADD_DELTA_QP_SUPPORT
    picture_control_set_ptr->delta_q_present_flag = 1;
    picture_control_set_ptr->delta_lf_present_flag = 0;
    picture_control_set_ptr->delta_q_res = DEFAULT_DELTA_Q_RES;
#else
    picture_control_set_ptr->delta_q_present_flag = 0;
#endif

    picture_control_set_ptr->delta_lf_present_flag = 0;
    picture_control_set_ptr->delta_lf_res = 0;
    picture_control_set_ptr->delta_lf_multi = 0;


    picture_control_set_ptr->current_frame_id = 0;
    picture_control_set_ptr->frame_refs_short_signaling = 0;
    picture_control_set_ptr->allow_comp_inter_inter = 0;
    //  int32_t all_one_sided_refs;

}
/***************************************
 * ResourceCoordination Kernel
 ***************************************/
void* resource_coordination_kernel(void *input_ptr)
{
    ResourceCoordinationContext   *context_ptr = (ResourceCoordinationContext*)input_ptr;

    EbObjectWrapper               *pictureControlSetWrapperPtr;

    PictureParentControlSet       *picture_control_set_ptr;

    EbObjectWrapper               *previousSequenceControlSetWrapperPtr;
    SequenceControlSet            *sequence_control_set_ptr;

    EbObjectWrapper               *ebInputWrapperPtr;
    EbBufferHeaderType              *ebInputPtr;
    EbObjectWrapper               *outputWrapperPtr;
    ResourceCoordinationResults   *outputResultsPtr;

    EbObjectWrapper               *input_picture_wrapper_ptr;
    EbObjectWrapper               *reference_picture_wrapper_ptr;

    uint32_t                         instance_index;
    EbBool                           end_of_sequence_flag = EB_FALSE;
    uint32_t                         aspectRatio;

    uint32_t                         input_size = 0;
    EbObjectWrapper               *prevPictureControlSetWrapperPtr = 0;
#if !ME_HME_OQ
    uint16_t                         hmeRegionIndex = 0;
#endif
    for (;;) {

        // Tie instance_index to zero for now...
        instance_index = 0;

        // Get the Next svt Input Buffer [BLOCKING]
        eb_get_full_object(
            context_ptr->input_buffer_fifo_ptr,
            &ebInputWrapperPtr);
        ebInputPtr = (EbBufferHeaderType*)ebInputWrapperPtr->object_ptr;
        sequence_control_set_ptr = context_ptr->sequence_control_set_instance_array[instance_index]->sequence_control_set_ptr;

        // If config changes occured since the last picture began encoding, then
        //   prepare a new sequence_control_set_ptr containing the new changes and update the state
        //   of the previous Active SequenceControlSet
        eb_block_on_mutex(context_ptr->sequence_control_set_instance_array[instance_index]->config_mutex);
        if (context_ptr->sequence_control_set_instance_array[instance_index]->encode_context_ptr->initial_picture) {

            // Update picture width, picture height, cropping right offset, cropping bottom offset, and conformance windows
            if (context_ptr->sequence_control_set_instance_array[instance_index]->encode_context_ptr->initial_picture)

            {
                context_ptr->sequence_control_set_instance_array[instance_index]->sequence_control_set_ptr->luma_width = context_ptr->sequence_control_set_instance_array[instance_index]->sequence_control_set_ptr->max_input_luma_width;
                context_ptr->sequence_control_set_instance_array[instance_index]->sequence_control_set_ptr->luma_height = context_ptr->sequence_control_set_instance_array[instance_index]->sequence_control_set_ptr->max_input_luma_height;
                context_ptr->sequence_control_set_instance_array[instance_index]->sequence_control_set_ptr->chroma_width = (context_ptr->sequence_control_set_instance_array[instance_index]->sequence_control_set_ptr->max_input_luma_width >> 1);
                context_ptr->sequence_control_set_instance_array[instance_index]->sequence_control_set_ptr->chroma_height = (context_ptr->sequence_control_set_instance_array[instance_index]->sequence_control_set_ptr->max_input_luma_height >> 1);

                context_ptr->sequence_control_set_instance_array[instance_index]->sequence_control_set_ptr->pad_right = context_ptr->sequence_control_set_instance_array[instance_index]->sequence_control_set_ptr->max_input_pad_right;
                context_ptr->sequence_control_set_instance_array[instance_index]->sequence_control_set_ptr->cropping_right_offset = context_ptr->sequence_control_set_instance_array[instance_index]->sequence_control_set_ptr->pad_right;
                context_ptr->sequence_control_set_instance_array[instance_index]->sequence_control_set_ptr->pad_bottom = context_ptr->sequence_control_set_instance_array[instance_index]->sequence_control_set_ptr->max_input_pad_bottom;
                context_ptr->sequence_control_set_instance_array[instance_index]->sequence_control_set_ptr->cropping_bottom_offset = context_ptr->sequence_control_set_instance_array[instance_index]->sequence_control_set_ptr->pad_bottom;

                if (context_ptr->sequence_control_set_instance_array[instance_index]->sequence_control_set_ptr->pad_right != 0 || context_ptr->sequence_control_set_instance_array[instance_index]->sequence_control_set_ptr->pad_bottom != 0) {
                    context_ptr->sequence_control_set_instance_array[instance_index]->sequence_control_set_ptr->conformance_window_flag = 1;
                }
                else {
                    context_ptr->sequence_control_set_instance_array[instance_index]->sequence_control_set_ptr->conformance_window_flag = 0;
                }

                input_size = context_ptr->sequence_control_set_instance_array[instance_index]->sequence_control_set_ptr->luma_width * context_ptr->sequence_control_set_instance_array[instance_index]->sequence_control_set_ptr->luma_height;
            }


            // Copy previous Active SequenceControlSetPtr to a place holder
            previousSequenceControlSetWrapperPtr = context_ptr->sequenceControlSetActiveArray[instance_index];

            // Get empty SequenceControlSet [BLOCKING]
            eb_get_empty_object(
                context_ptr->sequence_control_set_empty_fifo_ptr,
                &context_ptr->sequenceControlSetActiveArray[instance_index]);

            // Copy the contents of the active SequenceControlSet into the new empty SequenceControlSet
            copy_sequence_control_set(
                (SequenceControlSet*)context_ptr->sequenceControlSetActiveArray[instance_index]->object_ptr,
                context_ptr->sequence_control_set_instance_array[instance_index]->sequence_control_set_ptr);

            // Disable releaseFlag of new SequenceControlSet
            eb_object_release_disable(
                context_ptr->sequenceControlSetActiveArray[instance_index]);

            if (previousSequenceControlSetWrapperPtr != EB_NULL) {

                // Enable releaseFlag of old SequenceControlSet
                eb_object_release_enable(
                    previousSequenceControlSetWrapperPtr);

                // Check to see if previous SequenceControlSet is already inactive, if TRUE then release the SequenceControlSet
                if (previousSequenceControlSetWrapperPtr->liveCount == 0) {
                    eb_release_object(
                        previousSequenceControlSetWrapperPtr);
                }
            }
        }
        eb_release_mutex(context_ptr->sequence_control_set_instance_array[instance_index]->config_mutex);

        // Sequence Control Set is released by Rate Control after passing through MDC->MD->ENCDEC->Packetization->RateControl
        //   and in the PictureManager
        eb_object_inc_live_count(
            context_ptr->sequenceControlSetActiveArray[instance_index],
            2);

        // Set the current SequenceControlSet
        sequence_control_set_ptr = (SequenceControlSet*)context_ptr->sequenceControlSetActiveArray[instance_index]->object_ptr;

        // Init SB Params
        if (context_ptr->sequence_control_set_instance_array[instance_index]->encode_context_ptr->initial_picture) {
            derive_input_resolution(
                sequence_control_set_ptr,
                input_size);

            sb_params_init(sequence_control_set_ptr);
            sb_geom_init(sequence_control_set_ptr);

            // Sep PM mode
            sequence_control_set_ptr->pm_mode = sequence_control_set_ptr->input_resolution < INPUT_SIZE_4K_RANGE ?
                PM_MODE_2 :
                PM_MODE_1;

            // Construct PM Trans Coeff Shaping
            if (context_ptr->sequence_control_set_instance_array[instance_index]->encode_context_ptr->initial_picture) {
                if (sequence_control_set_ptr->pm_mode == PM_MODE_0) {
                    construct_pm_trans_coeff_shaping(sequence_control_set_ptr);
                }
            }

        }

        //Get a New ParentPCS where we will hold the new inputPicture
        eb_get_empty_object(
            context_ptr->picture_control_set_fifo_ptr_array[instance_index],
            &pictureControlSetWrapperPtr);

        // Parent PCS is released by the Rate Control after passing through MDC->MD->ENCDEC->Packetization
        eb_object_inc_live_count(
            pictureControlSetWrapperPtr,
            1);

        picture_control_set_ptr = (PictureParentControlSet*)pictureControlSetWrapperPtr->object_ptr;

        picture_control_set_ptr->p_pcs_wrapper_ptr = pictureControlSetWrapperPtr;

        // Set the Encoder mode
        picture_control_set_ptr->enc_mode = sequence_control_set_ptr->static_config.enc_mode;

        // Keep track of the previous input for the ZZ SADs computation
        picture_control_set_ptr->previous_picture_control_set_wrapper_ptr = (context_ptr->sequence_control_set_instance_array[instance_index]->encode_context_ptr->initial_picture) ?
            pictureControlSetWrapperPtr :
            sequence_control_set_ptr->encode_context_ptr->previous_picture_control_set_wrapper_ptr;

        sequence_control_set_ptr->encode_context_ptr->previous_picture_control_set_wrapper_ptr = pictureControlSetWrapperPtr;

        // Copy data from the svt buffer to the input frame
        // *Note - Assumes 4:2:0 planar
        input_picture_wrapper_ptr = ebInputWrapperPtr;
        picture_control_set_ptr->enhanced_picture_ptr = (EbPictureBufferDesc*)ebInputPtr->p_buffer;
        picture_control_set_ptr->input_ptr            = ebInputPtr;
        end_of_sequence_flag = (picture_control_set_ptr->input_ptr->flags & EB_BUFFERFLAG_EOS) ? EB_TRUE : EB_FALSE;
        eb_start_time(&picture_control_set_ptr->start_time_seconds, &picture_control_set_ptr->start_time_u_seconds);
        
        picture_control_set_ptr->sequence_control_set_wrapper_ptr = context_ptr->sequenceControlSetActiveArray[instance_index];
        picture_control_set_ptr->sequence_control_set_ptr = sequence_control_set_ptr;
        picture_control_set_ptr->input_picture_wrapper_ptr = input_picture_wrapper_ptr;
        picture_control_set_ptr->end_of_sequence_flag = end_of_sequence_flag;

        // Set Picture Control Flags
        picture_control_set_ptr->idr_flag = sequence_control_set_ptr->encode_context_ptr->initial_picture || (picture_control_set_ptr->input_ptr->pic_type == EB_IDR_PICTURE);
        picture_control_set_ptr->cra_flag = (picture_control_set_ptr->input_ptr->pic_type == EB_I_PICTURE) ? EB_TRUE : EB_FALSE;
        picture_control_set_ptr->scene_change_flag = EB_FALSE;
        picture_control_set_ptr->qp_on_the_fly = EB_FALSE;
        picture_control_set_ptr->sb_total_count = sequence_control_set_ptr->sb_total_count;
        picture_control_set_ptr->eos_coming = (ebInputPtr->flags & (EB_BUFFERFLAG_EOS << 1)) ? EB_TRUE : EB_FALSE;

        if (sequence_control_set_ptr->static_config.speed_control_flag) {
            SpeedBufferControl(
                context_ptr,
                picture_control_set_ptr,
                sequence_control_set_ptr);
        }
        else {
            picture_control_set_ptr->enc_mode = (EbEncMode)sequence_control_set_ptr->static_config.enc_mode;
        }

        aspectRatio = (sequence_control_set_ptr->luma_width * 10) / sequence_control_set_ptr->luma_height;
        aspectRatio = (aspectRatio <= ASPECT_RATIO_4_3) ? ASPECT_RATIO_CLASS_0 : (aspectRatio <= ASPECT_RATIO_16_9) ? ASPECT_RATIO_CLASS_1 : ASPECT_RATIO_CLASS_2;

        // Set the SCD Mode
        sequence_control_set_ptr->scd_mode = sequence_control_set_ptr->static_config.scene_change_detection == 0 ?
            SCD_MODE_0 :
            SCD_MODE_1;

        // Set the block mean calculation prec
#if ENCODER_MODE_CLEANUP
        sequence_control_set_ptr->block_mean_calc_prec = BLOCK_MEAN_PREC_SUB;
#else
        sequence_control_set_ptr->block_mean_calc_prec = picture_control_set_ptr->enc_mode == ENC_M0 ?
            BLOCK_MEAN_PREC_FULL :
            BLOCK_MEAN_PREC_SUB;
#endif
#if ME_HME_OQ
        // Pre-Analysis Signal(s) derivation
        signal_derivation_pre_analysis_oq(
            sequence_control_set_ptr,
            picture_control_set_ptr);
#else
        // Set ME/HME Params
        if (sequence_control_set_ptr->static_config.use_default_me_hme) { // Need third option
            SetMeHmeParams(
                picture_control_set_ptr,
                sequence_control_set_ptr,
                sequence_control_set_ptr->input_resolution);
        }
        else {
            picture_control_set_ptr->enable_hme_flag = sequence_control_set_ptr->static_config.enable_hme_flag;

            picture_control_set_ptr->enable_hme_level0_flag = sequence_control_set_ptr->static_config.enable_hme_level0_flag;
            picture_control_set_ptr->enable_hme_level1_flag = sequence_control_set_ptr->static_config.enable_hme_level1_flag;
            picture_control_set_ptr->enable_hme_level2_flag = sequence_control_set_ptr->static_config.enable_hme_level2_flag;

            picture_control_set_ptr->search_area_width = (uint8_t)sequence_control_set_ptr->static_config.search_area_width;
            picture_control_set_ptr->search_area_height = (uint8_t)sequence_control_set_ptr->static_config.search_area_height;

            picture_control_set_ptr->number_hme_search_region_in_width = (uint16_t)sequence_control_set_ptr->static_config.number_hme_search_region_in_width;
            picture_control_set_ptr->number_hme_search_region_in_height = (uint16_t)sequence_control_set_ptr->static_config.number_hme_search_region_in_height;

            picture_control_set_ptr->hme_level0_total_search_area_width = (uint16_t)sequence_control_set_ptr->static_config.hme_level0_total_search_area_width;
            picture_control_set_ptr->hme_level0_total_search_area_height = (uint16_t)sequence_control_set_ptr->static_config.hme_level0_total_search_area_height;

            for (hmeRegionIndex = 0; hmeRegionIndex < picture_control_set_ptr->number_hme_search_region_in_width; ++hmeRegionIndex) {
                picture_control_set_ptr->hme_level0_search_area_in_width_array[hmeRegionIndex] = (uint16_t)sequence_control_set_ptr->static_config.hme_level0_search_area_in_width_array[hmeRegionIndex];
                picture_control_set_ptr->hme_level1_search_area_in_width_array[hmeRegionIndex] = (uint16_t)sequence_control_set_ptr->static_config.hme_level1_search_area_in_width_array[hmeRegionIndex];
                picture_control_set_ptr->hme_level2_search_area_in_width_array[hmeRegionIndex] = (uint16_t)sequence_control_set_ptr->static_config.hme_level2_search_area_in_width_array[hmeRegionIndex];
            }

            for (hmeRegionIndex = 0; hmeRegionIndex < picture_control_set_ptr->number_hme_search_region_in_height; ++hmeRegionIndex) {
                picture_control_set_ptr->hme_level0_search_area_in_height_array[hmeRegionIndex] = (uint16_t)sequence_control_set_ptr->static_config.hme_level0_search_area_in_height_array[hmeRegionIndex];
                picture_control_set_ptr->hme_level1_search_area_in_height_array[hmeRegionIndex] = (uint16_t)sequence_control_set_ptr->static_config.hme_level1_search_area_in_height_array[hmeRegionIndex];
                picture_control_set_ptr->hme_level2_search_area_in_height_array[hmeRegionIndex] = (uint16_t)sequence_control_set_ptr->static_config.hme_level2_search_area_in_height_array[hmeRegionIndex];
            }
        }
#endif
        // Rate Control
        // Set the ME Distortion and OIS Historgrams to zero
        if (sequence_control_set_ptr->static_config.rate_control_mode) {
            EB_MEMSET(picture_control_set_ptr->me_distortion_histogram, 0, NUMBER_OF_SAD_INTERVALS * sizeof(uint16_t));
            EB_MEMSET(picture_control_set_ptr->ois_distortion_histogram, 0, NUMBER_OF_INTRA_SAD_INTERVALS * sizeof(uint16_t));
        }
        picture_control_set_ptr->full_sb_count = 0;
    
        if (sequence_control_set_ptr->static_config.use_qp_file == 1) {
            picture_control_set_ptr->qp_on_the_fly = EB_TRUE;
            if (picture_control_set_ptr->input_ptr->qp > MAX_QP_VALUE){
                SVT_LOG("SVT [WARNING]: INPUT QP OUTSIDE OF RANGE\n");
                picture_control_set_ptr->qp_on_the_fly = EB_FALSE;
                picture_control_set_ptr->picture_qp = (uint8_t)sequence_control_set_ptr->qp;
            }
            picture_control_set_ptr->picture_qp = (uint8_t)picture_control_set_ptr->input_ptr->qp;
        }
        else {
            picture_control_set_ptr->qp_on_the_fly = EB_FALSE;
            picture_control_set_ptr->picture_qp = (uint8_t)sequence_control_set_ptr->qp;
        }

        // Picture Stats
        picture_control_set_ptr->picture_number = context_ptr->pictureNumberArray[instance_index]++;
        ResetPcsAv1(picture_control_set_ptr);

        sequence_control_set_ptr->encode_context_ptr->initial_picture = EB_FALSE;

        // Get Empty Reference Picture Object
        eb_get_empty_object(
            sequence_control_set_ptr->encode_context_ptr->pa_reference_picture_pool_fifo_ptr,
            &reference_picture_wrapper_ptr);

        picture_control_set_ptr->pa_reference_picture_wrapper_ptr = reference_picture_wrapper_ptr;

        // Give the new Reference a nominal liveCount of 1
        eb_object_inc_live_count(
            picture_control_set_ptr->pa_reference_picture_wrapper_ptr,
            2);

        eb_object_inc_live_count(
            pictureControlSetWrapperPtr,
            2);
    
        ((EbPaReferenceObject*)picture_control_set_ptr->pa_reference_picture_wrapper_ptr->object_ptr)->inputPaddedPicturePtr->buffer_y = picture_control_set_ptr->enhanced_picture_ptr->buffer_y;
        // Get Empty Output Results Object
        if (picture_control_set_ptr->picture_number > 0 && (prevPictureControlSetWrapperPtr != NULL))
        {
            ((PictureParentControlSet       *)prevPictureControlSetWrapperPtr->object_ptr)->end_of_sequence_flag = end_of_sequence_flag;
            eb_get_empty_object(
                context_ptr->resource_coordination_results_output_fifo_ptr,
                &outputWrapperPtr);
            outputResultsPtr = (ResourceCoordinationResults*)outputWrapperPtr->object_ptr;
            outputResultsPtr->pictureControlSetWrapperPtr = prevPictureControlSetWrapperPtr;

            // Post the finished Results Object
            eb_post_full_object(outputWrapperPtr);
        }
        prevPictureControlSetWrapperPtr = pictureControlSetWrapperPtr;
    }

    return EB_NULL;
}

/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

/*
* Copyright (c) 2016, Alliance for Open Media. All rights reserved
*
* This source code is subject to the terms of the BSD 2 Clause License and
* the Alliance for Open Media Patent License 1.0. If the BSD 2 Clause License
* was not distributed with this source code in the LICENSE file, you can
* obtain it at www.aomedia.org/license/software. If the Alliance for Open
* Media Patent License 1.0 was not distributed with this source code in the
* PATENTS file, you can obtain it at www.aomedia.org/license/patent.
*/

#include <stdlib.h>

#include "EbEncDecTasks.h"
#include "EbEncDecResults.h"
#include "EbPictureDemuxResults.h"
#include "EbCodingLoop.h"
#include "EbSvtAv1ErrorCodes.h"
#include "EbDeblockingFilter.h"
#include "grainSynthesis.h"

void av1_cdef_search(
    EncDecContext_t                *context_ptr,
    SequenceControlSet           *sequence_control_set_ptr,
    PictureControlSet_t            *picture_control_set_ptr
);

void av1_cdef_frame(
    EncDecContext_t                *context_ptr,
    SequenceControlSet           *sequence_control_set_ptr,
    PictureControlSet_t            *pCs
);

void av1_cdef_search16bit(
    EncDecContext_t                *context_ptr,
    SequenceControlSet           *sequence_control_set_ptr,
    PictureControlSet_t            *picture_control_set_ptr
);
void av1_cdef_frame16bit(
    uint8_t is16bit,
    SequenceControlSet           *sequence_control_set_ptr,
    PictureControlSet_t            *pCs
);

void av1_add_film_grain(EbPictureBufferDesc_t *src,
    EbPictureBufferDesc_t *dst,
    aom_film_grain_t *film_grain_ptr);

void av1_loop_restoration_save_boundary_lines(const Yv12BufferConfig *frame, Av1Common *cm, int32_t after_cdef);
void av1_pick_filter_restoration(const Yv12BufferConfig *src, Yv12BufferConfig * trial_frame_rst /*AV1_COMP *cpi*/, Macroblock *x, Av1Common *const cm);
void av1_loop_restoration_filter_frame(Yv12BufferConfig *frame, Av1Common *cm, int32_t optimized_lr);

const int16_t encMinDeltaQpWeightTab[MAX_TEMPORAL_LAYERS] = { 100, 100, 100, 100, 100, 100 };
const int16_t encMaxDeltaQpWeightTab[MAX_TEMPORAL_LAYERS] = { 100, 100, 100, 100, 100, 100 };

const int8_t  encMinDeltaQpISliceTab[4] = { -5, -5, -3, -2 };

const int8_t  encMinDeltaQpTab[4][MAX_TEMPORAL_LAYERS] = {
    { -4, -2, -2, -1, -1, -1 },
    { -4, -2, -2, -1, -1, -1 },
    { -3, -1, -1, -1, -1, -1 },
    { -1, -0, -0, -0, -0, -0 },
};

const int8_t  encMaxDeltaQpTab[4][MAX_TEMPORAL_LAYERS] = {
    { 4, 5, 5, 5, 5, 5 },
    { 4, 5, 5, 5, 5, 5 },
    { 4, 5, 5, 5, 5, 5 },
    { 4, 5, 5, 5, 5, 5 }
};

/******************************************************
 * Enc Dec Context Constructor
 ******************************************************/
EbErrorType enc_dec_context_ctor(
    EncDecContext_t        **context_dbl_ptr,
    EbFifo                *mode_decision_configuration_input_fifo_ptr,
    EbFifo                *packetization_output_fifo_ptr,
    EbFifo                *feedback_fifo_ptr,
    EbFifo                *picture_demux_fifo_ptr,
    EbBool                  is16bit,
    EbColorFormat           color_format,
    uint32_t                max_input_luma_width,
    uint32_t                max_input_luma_height){

    (void)max_input_luma_width;
    (void)max_input_luma_height;
    EbErrorType return_error = EB_ErrorNone;
    EncDecContext_t *context_ptr;
    EB_MALLOC(EncDecContext_t*, context_ptr, sizeof(EncDecContext_t), EB_N_PTR);
    *context_dbl_ptr = context_ptr;

    context_ptr->is16bit = is16bit;
    context_ptr->color_format = color_format;

    // Input/Output System Resource Manager FIFOs
    context_ptr->mode_decision_input_fifo_ptr = mode_decision_configuration_input_fifo_ptr;
    context_ptr->enc_dec_output_fifo_ptr = packetization_output_fifo_ptr;
    context_ptr->enc_dec_feedback_fifo_ptr = feedback_fifo_ptr;
    context_ptr->picture_demux_output_fifo_ptr = picture_demux_fifo_ptr;

    // Trasform Scratch Memory
    EB_MALLOC(int16_t*, context_ptr->transform_inner_array_ptr, 3152, EB_N_PTR); //refer to EbInvTransform_SSE2.as. case 32x32
    // MD rate Estimation tables
    EB_MALLOC(MdRateEstimationContext_t*, context_ptr->md_rate_estimation_ptr, sizeof(MdRateEstimationContext_t), EB_N_PTR);


    // Prediction Buffer
    {
        EbPictureBufferDescInitData_t initData;

        initData.bufferEnableMask = PICTURE_BUFFER_DESC_FULL_MASK;
        initData.maxWidth = SB_STRIDE_Y;
        initData.maxHeight = SB_STRIDE_Y;
        initData.bit_depth = EB_8BIT;
        initData.left_padding = 0;
        initData.right_padding = 0;
        initData.top_padding = 0;
        initData.bot_padding = 0;
        initData.splitMode = EB_FALSE;
        initData.color_format = color_format;

        context_ptr->input_sample16bit_buffer = (EbPictureBufferDesc_t *)EB_NULL;
        if (is16bit) {
            initData.bit_depth = EB_16BIT;

            return_error = eb_picture_buffer_desc_ctor(
                (EbPtr*)&context_ptr->input_sample16bit_buffer,
                (EbPtr)&initData);
            if (return_error == EB_ErrorInsufficientResources) {
                return EB_ErrorInsufficientResources;
            }
        }

    }

    // Scratch Coeff Buffer
    {
        EbPictureBufferDescInitData_t initData;

        initData.bufferEnableMask = PICTURE_BUFFER_DESC_FULL_MASK;
        initData.maxWidth = SB_STRIDE_Y;
        initData.maxHeight = SB_STRIDE_Y;
        initData.bit_depth = EB_16BIT;
        initData.color_format = color_format;
        initData.left_padding = 0;
        initData.right_padding = 0;
        initData.top_padding = 0;
        initData.bot_padding = 0;
        initData.splitMode = EB_FALSE;

        EbPictureBufferDescInitData_t init32BitData;

        init32BitData.bufferEnableMask = PICTURE_BUFFER_DESC_FULL_MASK;
        init32BitData.maxWidth = SB_STRIDE_Y;
        init32BitData.maxHeight = SB_STRIDE_Y;
        init32BitData.bit_depth = EB_32BIT;
        init32BitData.color_format = color_format;
        init32BitData.left_padding = 0;
        init32BitData.right_padding = 0;
        init32BitData.top_padding = 0;
        init32BitData.bot_padding = 0;
        init32BitData.splitMode = EB_FALSE;
        return_error = eb_picture_buffer_desc_ctor(
            (EbPtr*)&context_ptr->inverse_quant_buffer,
            (EbPtr)&init32BitData);

        if (return_error == EB_ErrorInsufficientResources) {
            return EB_ErrorInsufficientResources;
        }
        return_error = eb_picture_buffer_desc_ctor(
            (EbPtr*)&context_ptr->transform_buffer,
            (EbPtr)&init32BitData);
        if (return_error == EB_ErrorInsufficientResources) {
            return EB_ErrorInsufficientResources;
        }

        return_error = eb_picture_buffer_desc_ctor(
            (EbPtr*)&context_ptr->residual_buffer,
            (EbPtr)&initData);
        if (return_error == EB_ErrorInsufficientResources) {
            return EB_ErrorInsufficientResources;
        }

    }

    // Intra Reference Samples
    return_error = IntraReferenceSamplesCtor(&context_ptr->intra_ref_ptr);
    if (return_error == EB_ErrorInsufficientResources) {
        return EB_ErrorInsufficientResources;
    }
    context_ptr->intra_ref_ptr16 = (IntraReference16bitSamples_t *)EB_NULL;
    if (is16bit) {
        return_error = IntraReference16bitSamplesCtor(&context_ptr->intra_ref_ptr16);
        if (return_error == EB_ErrorInsufficientResources) {
            return EB_ErrorInsufficientResources;
        }
    }
    // Mode Decision Context
    return_error = mode_decision_context_ctor(&context_ptr->md_context, color_format, 0, 0);

    if (return_error == EB_ErrorInsufficientResources) {
        return EB_ErrorInsufficientResources;
    }

    // Second Stage ME Context
    if (return_error == EB_ErrorInsufficientResources) {
        return EB_ErrorInsufficientResources;
    }


    context_ptr->md_context->enc_dec_context_ptr = context_ptr;

    return EB_ErrorNone;
}



/**************************************************
 * Reset Mode Decision Neighbor Arrays
 *************************************************/
static void ResetEncodePassNeighborArrays(PictureControlSet_t *picture_control_set_ptr)
{
    neighbor_array_unit_reset(picture_control_set_ptr->ep_intra_luma_mode_neighbor_array);
    neighbor_array_unit_reset(picture_control_set_ptr->ep_intra_chroma_mode_neighbor_array);
    neighbor_array_unit_reset(picture_control_set_ptr->ep_mv_neighbor_array);
    neighbor_array_unit_reset(picture_control_set_ptr->ep_skip_flag_neighbor_array);
    neighbor_array_unit_reset(picture_control_set_ptr->ep_mode_type_neighbor_array);
    neighbor_array_unit_reset(picture_control_set_ptr->ep_leaf_depth_neighbor_array);
    neighbor_array_unit_reset(picture_control_set_ptr->ep_luma_recon_neighbor_array);
    neighbor_array_unit_reset(picture_control_set_ptr->ep_cb_recon_neighbor_array);
    neighbor_array_unit_reset(picture_control_set_ptr->ep_cr_recon_neighbor_array);
    neighbor_array_unit_reset(picture_control_set_ptr->amvp_mv_merge_mv_neighbor_array);
    neighbor_array_unit_reset(picture_control_set_ptr->amvp_mv_merge_mode_type_neighbor_array);

    return;
}

/**************************************************
 * Reset Coding Loop
 **************************************************/
static void ResetEncDec(
    EncDecContext_t         *context_ptr,
    PictureControlSet_t     *picture_control_set_ptr,
    SequenceControlSet    *sequence_control_set_ptr,
    uint32_t                   segment_index)
{
    EB_SLICE                     slice_type;
    MdRateEstimationContext_t   *md_rate_estimation_array;
    context_ptr->is16bit = (EbBool)(sequence_control_set_ptr->static_config.encoder_bit_depth > EB_8BIT);


    // QP
    //context_ptr->qp          = picture_control_set_ptr->parent_pcs_ptr->tilePtrArray[tileIndex]->tileQp;
#if ADD_DELTA_QP_SUPPORT
    uint16_t picture_qp = picture_control_set_ptr->parent_pcs_ptr->base_qindex;
    context_ptr->qp = picture_qp;
    context_ptr->qp_index = context_ptr->qp;
#else
    context_ptr->qp = picture_control_set_ptr->picture_qp;
#endif
    // Asuming cb and cr offset to be the same for chroma QP in both slice and pps for lambda computation

    context_ptr->chroma_qp = context_ptr->qp;

    // Lambda Assignement
    context_ptr->qp_index = (uint8_t)picture_control_set_ptr->parent_pcs_ptr->base_qindex;
    (*av1_lambda_assignment_function_table[picture_control_set_ptr->parent_pcs_ptr->pred_structure])(
        &context_ptr->fast_lambda,
        &context_ptr->full_lambda,
        &context_ptr->fast_chroma_lambda,
        &context_ptr->full_chroma_lambda,
        (uint8_t)picture_control_set_ptr->parent_pcs_ptr->enhanced_picture_ptr->bit_depth,
        context_ptr->qp_index);

    // Slice Type
    slice_type =
        (picture_control_set_ptr->parent_pcs_ptr->idr_flag == EB_TRUE) ? I_SLICE :
        picture_control_set_ptr->slice_type;

    // Increment the MD Rate Estimation array pointer to point to the right address based on the QP and slice type
    md_rate_estimation_array = (MdRateEstimationContext_t*)sequence_control_set_ptr->encode_context_ptr->md_rate_estimation_array;
#if ADD_DELTA_QP_SUPPORT
    md_rate_estimation_array += slice_type * TOTAL_NUMBER_OF_QP_VALUES + picture_control_set_ptr->parent_pcs_ptr->picture_qp;
#else
    md_rate_estimation_array += slice_type * TOTAL_NUMBER_OF_QP_VALUES + context_ptr->qp;
#endif

    // Reset MD rate Estimation table to initial values by copying from md_rate_estimation_array

    context_ptr->md_rate_estimation_ptr = md_rate_estimation_array;

    // TMVP Map Writer pointer
    if (picture_control_set_ptr->parent_pcs_ptr->is_used_as_reference_flag == EB_TRUE)
        context_ptr->reference_object_write_ptr = (EbReferenceObject*)picture_control_set_ptr->parent_pcs_ptr->reference_picture_wrapper_ptr->object_ptr;
    else
        context_ptr->reference_object_write_ptr = (EbReferenceObject*)EB_NULL;
    if (segment_index == 0) {
        ResetEncodePassNeighborArrays(picture_control_set_ptr);
    }


    return;
}

/******************************************************
 * EncDec Configure LCU
 ******************************************************/
static void EncDecConfigureLcu(
    EncDecContext_t         *context_ptr,
    LargestCodingUnit_t     *sb_ptr,
    PictureControlSet_t     *picture_control_set_ptr,
    SequenceControlSet    *sequence_control_set_ptr,
    uint8_t                    picture_qp,
    uint8_t                    sb_qp)
{

    //RC is off
    if (sequence_control_set_ptr->static_config.rate_control_mode == 0 && sequence_control_set_ptr->static_config.improve_sharpness == 0) {
        context_ptr->qp = picture_qp;

    }
    //RC is on
    else {
        context_ptr->qp = sb_qp;
    }

    // Asuming cb and cr offset to be the same for chroma QP in both slice and pps for lambda computation
    context_ptr->chroma_qp = context_ptr->qp;
    /* Note(CHKN) : when Qp modulation varies QP on a sub-LCU(CU) basis,  Lamda has to change based on Cu->QP , and then this code has to move inside the CU loop in MD */
    (void)sb_ptr;
    context_ptr->qp_index = (uint8_t)picture_control_set_ptr->parent_pcs_ptr->base_qindex;
    (*av1_lambda_assignment_function_table[picture_control_set_ptr->parent_pcs_ptr->pred_structure])(
        &context_ptr->fast_lambda,
        &context_ptr->full_lambda,
        &context_ptr->fast_chroma_lambda,
        &context_ptr->full_chroma_lambda,
        (uint8_t)picture_control_set_ptr->parent_pcs_ptr->enhanced_picture_ptr->bit_depth,
        context_ptr->qp_index);

    return;
}

/******************************************************
 * Update MD Segments
 *
 * This function is responsible for synchronizing the
 *   processing of MD Segment-rows.
 *   In short, the function starts processing
 *   of MD segment-rows as soon as their inputs are available
 *   and the previous segment-row has completed.  At
 *   any given time, only one segment row per picture
 *   is being processed.
 *
 * The function has two functions:
 *
 * (1) Update the Segment Completion Mask which tracks
 *   which MD Segment inputs are available.
 *
 * (2) Increment the segment-row counter (current_row_idx)
 *   as the segment-rows are completed.
 *
 * Since there is the potentential for thread collusion,
 *   a MUTEX a used to protect the sensitive data and
 *   the execution flow is separated into two paths
 *
 * (A) Initial update.
 *  -Update the Completion Mask [see (1) above]
 *  -If the picture is not currently being processed,
 *     check to see if the next segment-row is available
 *     and start processing.
 * (B) Continued processing
 *  -Upon the completion of a segment-row, check
 *     to see if the next segment-row's inputs have
 *     become available and begin processing if so.
 *
 * On last important point is that the thread-safe
 *   code section is kept minimally short. The MUTEX
 *   should NOT be locked for the entire processing
 *   of the segment-row (B) as this would block other
 *   threads from performing an update (A).
 ******************************************************/
EbBool AssignEncDecSegments(
    EncDecSegments_t   *segmentPtr,
    uint16_t             *segmentInOutIndex,
    EncDecTasks_t      *taskPtr,
    EbFifo           *srmFifoPtr)
{
    EbBool continueProcessingFlag = EB_FALSE;
    EbObjectWrapper *wrapper_ptr;
    EncDecTasks_t *feedbackTaskPtr;

    uint32_t rowSegmentIndex = 0;
    uint32_t segment_index;
    uint32_t rightSegmentIndex;
    uint32_t bottomLeftSegmentIndex;

    int16_t feedbackRowIndex = -1;

    uint32_t selfAssigned = EB_FALSE;

    //static FILE *trace = 0;
    //
    //if(trace == 0) {
    //    trace = fopen("seg-trace.txt","w");
    //}


    switch (taskPtr->inputType) {

    case ENCDEC_TASKS_MDC_INPUT:

        // The entire picture is provided by the MDC process, so
        //   no logic is necessary to clear input dependencies.

        // Start on Segment 0 immediately
        *segmentInOutIndex = segmentPtr->rowArray[0].currentSegIndex;
        taskPtr->inputType = ENCDEC_TASKS_CONTINUE;
        ++segmentPtr->rowArray[0].currentSegIndex;
        continueProcessingFlag = EB_TRUE;

        //fprintf(trace, "Start  Pic: %u Seg: %u\n",
        //    (unsigned) ((PictureControlSet_t*) taskPtr->picture_control_set_wrapper_ptr->object_ptr)->picture_number,
        //    *segmentInOutIndex);

        break;

    case ENCDEC_TASKS_ENCDEC_INPUT:

        // Setup rowSegmentIndex to release the in_progress token
        //rowSegmentIndex = taskPtr->encDecSegmentRowArray[0];

        // Start on the assigned row immediately
        *segmentInOutIndex = segmentPtr->rowArray[taskPtr->enc_dec_segment_row].currentSegIndex;
        taskPtr->inputType = ENCDEC_TASKS_CONTINUE;
        ++segmentPtr->rowArray[taskPtr->enc_dec_segment_row].currentSegIndex;
        continueProcessingFlag = EB_TRUE;

        //fprintf(trace, "Start  Pic: %u Seg: %u\n",
        //    (unsigned) ((PictureControlSet_t*) taskPtr->picture_control_set_wrapper_ptr->object_ptr)->picture_number,
        //    *segmentInOutIndex);

        break;

    case ENCDEC_TASKS_CONTINUE:

        // Update the Dependency List for Right and Bottom Neighbors
        segment_index = *segmentInOutIndex;
        rowSegmentIndex = segment_index / segmentPtr->segmentBandCount;

        rightSegmentIndex = segment_index + 1;
        bottomLeftSegmentIndex = segment_index + segmentPtr->segmentBandCount;

        // Right Neighbor
        if (segment_index < segmentPtr->rowArray[rowSegmentIndex].endingSegIndex)
        {
            eb_block_on_mutex(segmentPtr->rowArray[rowSegmentIndex].assignmentMutex);

            --segmentPtr->depMap.dependencyMap[rightSegmentIndex];

            if (segmentPtr->depMap.dependencyMap[rightSegmentIndex] == 0) {
                *segmentInOutIndex = segmentPtr->rowArray[rowSegmentIndex].currentSegIndex;
                ++segmentPtr->rowArray[rowSegmentIndex].currentSegIndex;
                selfAssigned = EB_TRUE;
                continueProcessingFlag = EB_TRUE;

                //fprintf(trace, "Start  Pic: %u Seg: %u\n",
                //    (unsigned) ((PictureControlSet_t*) taskPtr->picture_control_set_wrapper_ptr->object_ptr)->picture_number,
                //    *segmentInOutIndex);
            }

            eb_release_mutex(segmentPtr->rowArray[rowSegmentIndex].assignmentMutex);
        }

        // Bottom-left Neighbor
        if (rowSegmentIndex < segmentPtr->segmentRowCount - 1 && bottomLeftSegmentIndex >= segmentPtr->rowArray[rowSegmentIndex + 1].startingSegIndex)
        {
            eb_block_on_mutex(segmentPtr->rowArray[rowSegmentIndex + 1].assignmentMutex);

            --segmentPtr->depMap.dependencyMap[bottomLeftSegmentIndex];

            if (segmentPtr->depMap.dependencyMap[bottomLeftSegmentIndex] == 0) {
                if (selfAssigned == EB_TRUE) {
                    feedbackRowIndex = (int16_t)rowSegmentIndex + 1;
                }
                else {
                    *segmentInOutIndex = segmentPtr->rowArray[rowSegmentIndex + 1].currentSegIndex;
                    ++segmentPtr->rowArray[rowSegmentIndex + 1].currentSegIndex;
                    selfAssigned = EB_TRUE;
                    continueProcessingFlag = EB_TRUE;

                    //fprintf(trace, "Start  Pic: %u Seg: %u\n",
                    //    (unsigned) ((PictureControlSet_t*) taskPtr->picture_control_set_wrapper_ptr->object_ptr)->picture_number,
                    //    *segmentInOutIndex);
                }
            }
            eb_release_mutex(segmentPtr->rowArray[rowSegmentIndex + 1].assignmentMutex);
        }

        if (feedbackRowIndex > 0) {
            eb_get_empty_object(
                srmFifoPtr,
                &wrapper_ptr);
            feedbackTaskPtr = (EncDecTasks_t*)wrapper_ptr->object_ptr;
            feedbackTaskPtr->inputType = ENCDEC_TASKS_ENCDEC_INPUT;
            feedbackTaskPtr->enc_dec_segment_row = feedbackRowIndex;
            feedbackTaskPtr->picture_control_set_wrapper_ptr = taskPtr->picture_control_set_wrapper_ptr;
            eb_post_full_object(wrapper_ptr);
        }

        break;

    default:
        break;
    }

    return continueProcessingFlag;
}
void ReconOutput(
    PictureControlSet_t    *picture_control_set_ptr,
    SequenceControlSet   *sequence_control_set_ptr) {

    EbObjectWrapper             *outputReconWrapperPtr;
    EbBufferHeaderType           *outputReconPtr;
    EncodeContext_t               *encode_context_ptr = sequence_control_set_ptr->encode_context_ptr;
    EbBool is16bit = (sequence_control_set_ptr->static_config.encoder_bit_depth > EB_8BIT);
    // The totalNumberOfReconFrames counter has to be write/read protected as
    //   it is used to determine the end of the stream.  If it is not protected
    //   the encoder might not properly terminate.
    eb_block_on_mutex(encode_context_ptr->total_number_of_recon_frame_mutex);

    // Get Recon Buffer
    eb_get_empty_object(
        sequence_control_set_ptr->encode_context_ptr->recon_output_fifo_ptr,
        &outputReconWrapperPtr);
    outputReconPtr = (EbBufferHeaderType*)outputReconWrapperPtr->object_ptr;
    outputReconPtr->flags = 0;

    // START READ/WRITE PROTECTED SECTION
    if (encode_context_ptr->total_number_of_recon_frames == encode_context_ptr->terminating_picture_number)
        outputReconPtr->flags = EB_BUFFERFLAG_EOS;

    encode_context_ptr->total_number_of_recon_frames++;

    //eb_release_mutex(encode_context_ptr->terminating_conditions_mutex);

    // STOP READ/WRITE PROTECTED SECTION
    outputReconPtr->n_filled_len = 0;

    // Copy the Reconstructed Picture to the Output Recon Buffer
    {
        uint32_t sampleTotalCount;
        uint8_t *reconReadPtr;
        uint8_t *reconWritePtr;

        EbPictureBufferDesc_t *recon_ptr;
        {
            if (picture_control_set_ptr->parent_pcs_ptr->is_used_as_reference_flag == EB_TRUE)
                recon_ptr = is16bit ?
                ((EbReferenceObject*)picture_control_set_ptr->parent_pcs_ptr->reference_picture_wrapper_ptr->object_ptr)->reference_picture16bit :
                ((EbReferenceObject*)picture_control_set_ptr->parent_pcs_ptr->reference_picture_wrapper_ptr->object_ptr)->reference_picture;
            else {
                if (is16bit)
                    recon_ptr = picture_control_set_ptr->recon_picture16bit_ptr;
                else
                    recon_ptr = picture_control_set_ptr->recon_picture_ptr;
            }
        }

        // FGN: Create a buffer if needed, copy the reconstructed picture and run the film grain synthesis algorithm

        if (sequence_control_set_ptr->film_grain_params_present) {
            EbPictureBufferDesc_t  *intermediateBufferPtr;
            {
                if (is16bit)
                    intermediateBufferPtr = picture_control_set_ptr->film_grain_picture16bit_ptr;
                else
                    intermediateBufferPtr = picture_control_set_ptr->film_grain_picture_ptr;
            }

            aom_film_grain_t *film_grain_ptr;

            if (picture_control_set_ptr->parent_pcs_ptr->is_used_as_reference_flag == EB_TRUE)
                film_grain_ptr = &((EbReferenceObject*)picture_control_set_ptr->parent_pcs_ptr->reference_picture_wrapper_ptr->object_ptr)->film_grain_params;
            else
                film_grain_ptr = &picture_control_set_ptr->parent_pcs_ptr->film_grain_params;

            av1_add_film_grain(recon_ptr, intermediateBufferPtr, film_grain_ptr);
            recon_ptr = intermediateBufferPtr;
        }

        // End running the film grain
        // Y Recon Samples
        sampleTotalCount = ((recon_ptr->maxWidth - sequence_control_set_ptr->max_input_pad_right) * (recon_ptr->maxHeight - sequence_control_set_ptr->max_input_pad_bottom)) << is16bit;
        reconReadPtr = recon_ptr->buffer_y + (recon_ptr->origin_y << is16bit) * recon_ptr->stride_y + (recon_ptr->origin_x << is16bit);
        reconWritePtr = &(outputReconPtr->p_buffer[outputReconPtr->n_filled_len]);

        CHECK_REPORT_ERROR(
            (outputReconPtr->n_filled_len + sampleTotalCount <= outputReconPtr->n_alloc_len),
            encode_context_ptr->app_callback_ptr,
            EB_ENC_ROB_OF_ERROR);

        // Initialize Y recon buffer
        picture_copy_kernel(
            reconReadPtr,
            recon_ptr->stride_y,
            reconWritePtr,
            recon_ptr->maxWidth - sequence_control_set_ptr->max_input_pad_right,
            recon_ptr->width - sequence_control_set_ptr->pad_right,
            recon_ptr->height - sequence_control_set_ptr->pad_bottom,
            1 << is16bit);

        outputReconPtr->n_filled_len += sampleTotalCount;

        // U Recon Samples
        sampleTotalCount = ((recon_ptr->maxWidth - sequence_control_set_ptr->max_input_pad_right) * (recon_ptr->maxHeight - sequence_control_set_ptr->max_input_pad_bottom) >> 2) << is16bit;
        reconReadPtr = recon_ptr->bufferCb + ((recon_ptr->origin_y << is16bit) >> 1) * recon_ptr->strideCb + ((recon_ptr->origin_x << is16bit) >> 1);
        reconWritePtr = &(outputReconPtr->p_buffer[outputReconPtr->n_filled_len]);

        CHECK_REPORT_ERROR(
            (outputReconPtr->n_filled_len + sampleTotalCount <= outputReconPtr->n_alloc_len),
            encode_context_ptr->app_callback_ptr,
            EB_ENC_ROB_OF_ERROR);

        // Initialize U recon buffer
        picture_copy_kernel(
            reconReadPtr,
            recon_ptr->strideCb,
            reconWritePtr,
            (recon_ptr->maxWidth - sequence_control_set_ptr->max_input_pad_right) >> 1,
            (recon_ptr->width - sequence_control_set_ptr->pad_right) >> 1,
            (recon_ptr->height - sequence_control_set_ptr->pad_bottom) >> 1,
            1 << is16bit);
        outputReconPtr->n_filled_len += sampleTotalCount;

        // V Recon Samples
        sampleTotalCount = ((recon_ptr->maxWidth - sequence_control_set_ptr->max_input_pad_right) * (recon_ptr->maxHeight - sequence_control_set_ptr->max_input_pad_bottom) >> 2) << is16bit;
        reconReadPtr = recon_ptr->bufferCr + ((recon_ptr->origin_y << is16bit) >> 1) * recon_ptr->strideCr + ((recon_ptr->origin_x << is16bit) >> 1);
        reconWritePtr = &(outputReconPtr->p_buffer[outputReconPtr->n_filled_len]);

        CHECK_REPORT_ERROR(
            (outputReconPtr->n_filled_len + sampleTotalCount <= outputReconPtr->n_alloc_len),
            encode_context_ptr->app_callback_ptr,
            EB_ENC_ROB_OF_ERROR);

        // Initialize V recon buffer

        picture_copy_kernel(
            reconReadPtr,
            recon_ptr->strideCr,
            reconWritePtr,
            (recon_ptr->maxWidth - sequence_control_set_ptr->max_input_pad_right) >> 1,
            (recon_ptr->width - sequence_control_set_ptr->pad_right) >> 1,
            (recon_ptr->height - sequence_control_set_ptr->pad_bottom) >> 1,
            1 << is16bit);
        outputReconPtr->n_filled_len += sampleTotalCount;
        outputReconPtr->pts = picture_control_set_ptr->picture_number;
    }

    // Post the Recon object
    eb_post_full_object(outputReconWrapperPtr);
    eb_release_mutex(encode_context_ptr->total_number_of_recon_frame_mutex);
}

void PsnrCalculations(
    PictureControlSet_t    *picture_control_set_ptr,
    SequenceControlSet   *sequence_control_set_ptr){

    EbBool is16bit = (sequence_control_set_ptr->static_config.encoder_bit_depth > EB_8BIT);

    if (!is16bit) {

        EbPictureBufferDesc_t *recon_ptr;

        if (picture_control_set_ptr->parent_pcs_ptr->is_used_as_reference_flag == EB_TRUE)
            recon_ptr = ((EbReferenceObject*)picture_control_set_ptr->parent_pcs_ptr->reference_picture_wrapper_ptr->object_ptr)->reference_picture;
        else
            recon_ptr = picture_control_set_ptr->recon_picture_ptr;

        EbPictureBufferDesc_t *input_picture_ptr = (EbPictureBufferDesc_t*)picture_control_set_ptr->parent_pcs_ptr->enhanced_picture_ptr;

        uint64_t sseTotal[3] = { 0 };
        uint32_t   columnIndex;
        uint32_t   row_index = 0;
        uint64_t   residualDistortion = 0;
        EbByte  inputBuffer;
        EbByte  reconCoeffBuffer;

        reconCoeffBuffer = &((recon_ptr->buffer_y)[recon_ptr->origin_x + recon_ptr->origin_y * recon_ptr->stride_y]);
        inputBuffer = &((input_picture_ptr->buffer_y)[input_picture_ptr->origin_x + input_picture_ptr->origin_y * input_picture_ptr->stride_y]);

        residualDistortion = 0;

        while (row_index < sequence_control_set_ptr->luma_height) {

            columnIndex = 0;
            while (columnIndex < sequence_control_set_ptr->luma_width) {
                residualDistortion += (int64_t)SQR((int64_t)(inputBuffer[columnIndex]) - (reconCoeffBuffer[columnIndex]));
                ++columnIndex;
            }

            inputBuffer += input_picture_ptr->stride_y;
            reconCoeffBuffer += recon_ptr->stride_y;
            ++row_index;
        }

        sseTotal[0] = residualDistortion;

        reconCoeffBuffer = &((recon_ptr->bufferCb)[recon_ptr->origin_x / 2 + recon_ptr->origin_y / 2 * recon_ptr->strideCb]);
        inputBuffer = &((input_picture_ptr->bufferCb)[input_picture_ptr->origin_x / 2 + input_picture_ptr->origin_y / 2 * input_picture_ptr->strideCb]);

        residualDistortion = 0;
        row_index = 0;
        while (row_index < sequence_control_set_ptr->chroma_height) {

            columnIndex = 0;
            while (columnIndex < sequence_control_set_ptr->chroma_width) {
                residualDistortion += (int64_t)SQR((int64_t)(inputBuffer[columnIndex]) - (reconCoeffBuffer[columnIndex]));
                ++columnIndex;
            }

            inputBuffer += input_picture_ptr->strideCb;
            reconCoeffBuffer += recon_ptr->strideCb;
            ++row_index;
        }

        sseTotal[1] = residualDistortion;

        reconCoeffBuffer = &((recon_ptr->bufferCr)[recon_ptr->origin_x / 2 + recon_ptr->origin_y / 2 * recon_ptr->strideCr]);
        inputBuffer = &((input_picture_ptr->bufferCr)[input_picture_ptr->origin_x / 2 + input_picture_ptr->origin_y / 2 * input_picture_ptr->strideCr]);
        residualDistortion = 0;
        row_index = 0;

        while (row_index < sequence_control_set_ptr->chroma_height) {

            columnIndex = 0;
            while (columnIndex < sequence_control_set_ptr->chroma_width) {
                residualDistortion += (int64_t)SQR((int64_t)(inputBuffer[columnIndex]) - (reconCoeffBuffer[columnIndex]));
                ++columnIndex;
            }

            inputBuffer += input_picture_ptr->strideCr;
            reconCoeffBuffer += recon_ptr->strideCr;
            ++row_index;
        }

        sseTotal[2] = residualDistortion;
        picture_control_set_ptr->parent_pcs_ptr->luma_sse = (uint32_t)sseTotal[0];
        picture_control_set_ptr->parent_pcs_ptr->cr_sse = (uint32_t)sseTotal[1];
        picture_control_set_ptr->parent_pcs_ptr->cb_sse = (uint32_t)sseTotal[2];
    }
    else {

        EbPictureBufferDesc_t *recon_ptr;

        if (picture_control_set_ptr->parent_pcs_ptr->is_used_as_reference_flag == EB_TRUE)
            recon_ptr = ((EbReferenceObject*)picture_control_set_ptr->parent_pcs_ptr->reference_picture_wrapper_ptr->object_ptr)->reference_picture16bit;
        else
            recon_ptr = picture_control_set_ptr->recon_picture16bit_ptr;
        EbPictureBufferDesc_t *input_picture_ptr = (EbPictureBufferDesc_t*)picture_control_set_ptr->parent_pcs_ptr->enhanced_picture_ptr;

        uint64_t sseTotal[3] = { 0 };
        uint32_t   columnIndex;
        uint32_t   row_index = 0;
        uint64_t   residualDistortion = 0;
        EbByte  inputBuffer;
        EbByte  inputBufferBitInc;
        uint16_t*  reconCoeffBuffer;

        if (sequence_control_set_ptr->static_config.ten_bit_format == 1) {

            const uint32_t luma_width = sequence_control_set_ptr->luma_width;
            const uint32_t luma_height = sequence_control_set_ptr->luma_height;
            const uint32_t chroma_width = sequence_control_set_ptr->chroma_width;
            const uint32_t picture_width_in_sb = (luma_width + 64 - 1) / 64;
            const uint32_t pictureHeighInLcu = (luma_height + 64 - 1) / 64;
            const uint32_t luma2BitWidth = luma_width / 4;
            const uint32_t chroma_height = luma_height / 2;
            const uint32_t chroma2BitWidth = luma_width / 8;
            uint32_t lcuNumberInHeight, lcuNumberInWidth;

            EbByte  inputBufferOrg = &((input_picture_ptr->buffer_y)[input_picture_ptr->origin_x + input_picture_ptr->origin_y * input_picture_ptr->stride_y]);
            uint16_t*  reconBufferOrg = (uint16_t*)(&((recon_ptr->buffer_y)[(recon_ptr->origin_x << is16bit) + (recon_ptr->origin_y << is16bit) * recon_ptr->stride_y]));;

            EbByte  inputBufferOrgU = &((input_picture_ptr->bufferCb)[input_picture_ptr->origin_x / 2 + input_picture_ptr->origin_y / 2 * input_picture_ptr->strideCb]);;
            uint16_t*  reconBufferOrgU = reconCoeffBuffer = (uint16_t*)(&((recon_ptr->bufferCb)[(recon_ptr->origin_x << is16bit) / 2 + (recon_ptr->origin_y << is16bit) / 2 * recon_ptr->strideCb]));;

            EbByte  inputBufferOrgV = &((input_picture_ptr->bufferCr)[input_picture_ptr->origin_x / 2 + input_picture_ptr->origin_y / 2 * input_picture_ptr->strideCr]);;
            uint16_t*  reconBufferOrgV = reconCoeffBuffer = (uint16_t*)(&((recon_ptr->bufferCr)[(recon_ptr->origin_x << is16bit) / 2 + (recon_ptr->origin_y << is16bit) / 2 * recon_ptr->strideCr]));;

            residualDistortion = 0;
            uint64_t residualDistortionU = 0;
            uint64_t residualDistortionV = 0;

            for (lcuNumberInHeight = 0; lcuNumberInHeight < pictureHeighInLcu; ++lcuNumberInHeight)
            {
                for (lcuNumberInWidth = 0; lcuNumberInWidth < picture_width_in_sb; ++lcuNumberInWidth)
                {

                    uint32_t tbOriginX = lcuNumberInWidth * 64;
                    uint32_t tbOriginY = lcuNumberInHeight * 64;
                    uint32_t sb_width = (luma_width - tbOriginX) < 64 ? (luma_width - tbOriginX) : 64;
                    uint32_t sb_height = (luma_height - tbOriginY) < 64 ? (luma_height - tbOriginY) : 64;

                    inputBuffer = inputBufferOrg + tbOriginY * input_picture_ptr->stride_y + tbOriginX;
                    inputBufferBitInc = input_picture_ptr->bufferBitIncY + tbOriginY * luma2BitWidth + (tbOriginX / 4)*sb_height;
                    reconCoeffBuffer = reconBufferOrg + tbOriginY * recon_ptr->stride_y + tbOriginX;

                    uint64_t   j, k;
                    uint16_t   outPixel;
                    uint8_t    nBitPixel;
                    uint8_t   four2bitPels;
                    uint32_t     inn_stride = sb_width / 4;

                    for (j = 0; j < sb_height; j++)
                    {
                        for (k = 0; k < sb_width / 4; k++)
                        {

                            four2bitPels = inputBufferBitInc[k + j * inn_stride];

                            nBitPixel = (four2bitPels >> 6) & 3;
                            outPixel = inputBuffer[k * 4 + 0 + j * input_picture_ptr->stride_y] << 2;
                            outPixel = outPixel | nBitPixel;
                            residualDistortion += (int64_t)SQR((int64_t)outPixel - (int64_t)reconCoeffBuffer[k * 4 + 0 + j * recon_ptr->stride_y]);


                            nBitPixel = (four2bitPels >> 4) & 3;
                            outPixel = inputBuffer[k * 4 + 1 + j * input_picture_ptr->stride_y] << 2;
                            outPixel = outPixel | nBitPixel;
                            residualDistortion += (int64_t)SQR((int64_t)outPixel - (int64_t)reconCoeffBuffer[k * 4 + 1 + j * recon_ptr->stride_y]);


                            nBitPixel = (four2bitPels >> 2) & 3;
                            outPixel = inputBuffer[k * 4 + 2 + j * input_picture_ptr->stride_y] << 2;
                            outPixel = outPixel | nBitPixel;
                            residualDistortion += (int64_t)SQR((int64_t)outPixel - (int64_t)reconCoeffBuffer[k * 4 + 2 + j * recon_ptr->stride_y]);

                            nBitPixel = (four2bitPels >> 0) & 3;
                            outPixel = inputBuffer[k * 4 + 3 + j * input_picture_ptr->stride_y] << 2;
                            outPixel = outPixel | nBitPixel;
                            residualDistortion += (int64_t)SQR((int64_t)outPixel - (int64_t)reconCoeffBuffer[k * 4 + 3 + j * recon_ptr->stride_y]);

                        }
                    }

                    //U+V

                    tbOriginX = lcuNumberInWidth * 32;
                    tbOriginY = lcuNumberInHeight * 32;
                    sb_width = (chroma_width - tbOriginX) < 32 ? (chroma_width - tbOriginX) : 32;
                    sb_height = (chroma_height - tbOriginY) < 32 ? (chroma_height - tbOriginY) : 32;

                    inn_stride = sb_width / 4;

                    inputBuffer = inputBufferOrgU + tbOriginY * input_picture_ptr->strideCb + tbOriginX;

                    inputBufferBitInc = input_picture_ptr->bufferBitIncCb + tbOriginY * chroma2BitWidth + (tbOriginX / 4)*sb_height;

                    reconCoeffBuffer = reconBufferOrgU + tbOriginY * recon_ptr->strideCb + tbOriginX;



                    for (j = 0; j < sb_height; j++)
                    {
                        for (k = 0; k < sb_width / 4; k++)
                        {

                            four2bitPels = inputBufferBitInc[k + j * inn_stride];

                            nBitPixel = (four2bitPels >> 6) & 3;
                            outPixel = inputBuffer[k * 4 + 0 + j * input_picture_ptr->strideCb] << 2;
                            outPixel = outPixel | nBitPixel;
                            residualDistortionU += (int64_t)SQR((int64_t)outPixel - (int64_t)reconCoeffBuffer[k * 4 + 0 + j * recon_ptr->strideCb]);


                            nBitPixel = (four2bitPels >> 4) & 3;
                            outPixel = inputBuffer[k * 4 + 1 + j * input_picture_ptr->strideCb] << 2;
                            outPixel = outPixel | nBitPixel;
                            residualDistortionU += (int64_t)SQR((int64_t)outPixel - (int64_t)reconCoeffBuffer[k * 4 + 1 + j * recon_ptr->strideCb]);


                            nBitPixel = (four2bitPels >> 2) & 3;
                            outPixel = inputBuffer[k * 4 + 2 + j * input_picture_ptr->strideCb] << 2;
                            outPixel = outPixel | nBitPixel;
                            residualDistortionU += (int64_t)SQR((int64_t)outPixel - (int64_t)reconCoeffBuffer[k * 4 + 2 + j * recon_ptr->strideCb]);

                            nBitPixel = (four2bitPels >> 0) & 3;
                            outPixel = inputBuffer[k * 4 + 3 + j * input_picture_ptr->strideCb] << 2;
                            outPixel = outPixel | nBitPixel;
                            residualDistortionU += (int64_t)SQR((int64_t)outPixel - (int64_t)reconCoeffBuffer[k * 4 + 3 + j * recon_ptr->strideCb]);

                        }
                    }


                    inputBuffer = inputBufferOrgV + tbOriginY * input_picture_ptr->strideCr + tbOriginX;
                    inputBufferBitInc = input_picture_ptr->bufferBitIncCr + tbOriginY * chroma2BitWidth + (tbOriginX / 4)*sb_height;
                    reconCoeffBuffer = reconBufferOrgV + tbOriginY * recon_ptr->strideCr + tbOriginX;


                    for (j = 0; j < sb_height; j++)
                    {
                        for (k = 0; k < sb_width / 4; k++)
                        {

                            four2bitPels = inputBufferBitInc[k + j * inn_stride];

                            nBitPixel = (four2bitPels >> 6) & 3;
                            outPixel = inputBuffer[k * 4 + 0 + j * input_picture_ptr->strideCr] << 2;
                            outPixel = outPixel | nBitPixel;
                            residualDistortionV += (int64_t)SQR((int64_t)outPixel - (int64_t)reconCoeffBuffer[k * 4 + 0 + j * recon_ptr->strideCr]);


                            nBitPixel = (four2bitPels >> 4) & 3;
                            outPixel = inputBuffer[k * 4 + 1 + j * input_picture_ptr->strideCr] << 2;
                            outPixel = outPixel | nBitPixel;
                            residualDistortionV += (int64_t)SQR((int64_t)outPixel - (int64_t)reconCoeffBuffer[k * 4 + 1 + j * recon_ptr->strideCr]);


                            nBitPixel = (four2bitPels >> 2) & 3;
                            outPixel = inputBuffer[k * 4 + 2 + j * input_picture_ptr->strideCr] << 2;
                            outPixel = outPixel | nBitPixel;
                            residualDistortionV += (int64_t)SQR((int64_t)outPixel - (int64_t)reconCoeffBuffer[k * 4 + 2 + j * recon_ptr->strideCr]);

                            nBitPixel = (four2bitPels >> 0) & 3;
                            outPixel = inputBuffer[k * 4 + 3 + j * input_picture_ptr->strideCr] << 2;
                            outPixel = outPixel | nBitPixel;
                            residualDistortionV += (int64_t)SQR((int64_t)outPixel - (int64_t)reconCoeffBuffer[k * 4 + 3 + j * recon_ptr->strideCr]);

                        }
                    }




                }
            }


            sseTotal[0] = residualDistortion;
            sseTotal[1] = residualDistortionU;
            sseTotal[2] = residualDistortionV;
        }
        else {


            reconCoeffBuffer = (uint16_t*)(&((recon_ptr->buffer_y)[(recon_ptr->origin_x << is16bit) + (recon_ptr->origin_y << is16bit) * recon_ptr->stride_y]));
            inputBuffer = &((input_picture_ptr->buffer_y)[input_picture_ptr->origin_x + input_picture_ptr->origin_y * input_picture_ptr->stride_y]);
            inputBufferBitInc = &((input_picture_ptr->bufferBitIncY)[input_picture_ptr->origin_x + input_picture_ptr->origin_y * input_picture_ptr->strideBitIncY]);

            residualDistortion = 0;

            while (row_index < sequence_control_set_ptr->luma_height) {

                columnIndex = 0;
                while (columnIndex < sequence_control_set_ptr->luma_width) {
                    residualDistortion += (int64_t)SQR((int64_t)((((inputBuffer[columnIndex]) << 2) | ((inputBufferBitInc[columnIndex] >> 6) & 3))) - (reconCoeffBuffer[columnIndex]));

                    ++columnIndex;
                }

                inputBuffer += input_picture_ptr->stride_y;
                inputBufferBitInc += input_picture_ptr->strideBitIncY;
                reconCoeffBuffer += recon_ptr->stride_y;
                ++row_index;
            }

            sseTotal[0] = residualDistortion;

            reconCoeffBuffer = (uint16_t*)(&((recon_ptr->bufferCb)[(recon_ptr->origin_x << is16bit) / 2 + (recon_ptr->origin_y << is16bit) / 2 * recon_ptr->strideCb]));
            inputBuffer = &((input_picture_ptr->bufferCb)[input_picture_ptr->origin_x / 2 + input_picture_ptr->origin_y / 2 * input_picture_ptr->strideCb]);
            inputBufferBitInc = &((input_picture_ptr->bufferBitIncCb)[input_picture_ptr->origin_x / 2 + input_picture_ptr->origin_y / 2 * input_picture_ptr->strideBitIncCb]);

            residualDistortion = 0;
            row_index = 0;
            while (row_index < sequence_control_set_ptr->chroma_height) {

                columnIndex = 0;
                while (columnIndex < sequence_control_set_ptr->chroma_width) {
                    residualDistortion += (int64_t)SQR((int64_t)((((inputBuffer[columnIndex]) << 2) | ((inputBufferBitInc[columnIndex] >> 6) & 3))) - (reconCoeffBuffer[columnIndex]));
                    ++columnIndex;
                }

                inputBuffer += input_picture_ptr->strideCb;
                inputBufferBitInc += input_picture_ptr->strideBitIncCb;
                reconCoeffBuffer += recon_ptr->strideCb;
                ++row_index;
            }

            sseTotal[1] = residualDistortion;

            reconCoeffBuffer = (uint16_t*)(&((recon_ptr->bufferCr)[(recon_ptr->origin_x << is16bit) / 2 + (recon_ptr->origin_y << is16bit) / 2 * recon_ptr->strideCr]));
            inputBuffer = &((input_picture_ptr->bufferCr)[input_picture_ptr->origin_x / 2 + input_picture_ptr->origin_y / 2 * input_picture_ptr->strideCr]);
            inputBufferBitInc = &((input_picture_ptr->bufferBitIncCr)[input_picture_ptr->origin_x / 2 + input_picture_ptr->origin_y / 2 * input_picture_ptr->strideBitIncCr]);

            residualDistortion = 0;
            row_index = 0;

            while (row_index < sequence_control_set_ptr->chroma_height) {

                columnIndex = 0;
                while (columnIndex < sequence_control_set_ptr->chroma_width) {
                    residualDistortion += (int64_t)SQR((int64_t)((((inputBuffer[columnIndex]) << 2) | ((inputBufferBitInc[columnIndex] >> 6) & 3))) - (reconCoeffBuffer[columnIndex]));
                    ++columnIndex;
                }

                inputBuffer += input_picture_ptr->strideCr;
                inputBufferBitInc += input_picture_ptr->strideBitIncCr;
                reconCoeffBuffer += recon_ptr->strideCr;
                ++row_index;
            }

            sseTotal[2] = residualDistortion;

        }


        picture_control_set_ptr->parent_pcs_ptr->luma_sse = (uint32_t)sseTotal[0];
        picture_control_set_ptr->parent_pcs_ptr->cr_sse = (uint32_t)sseTotal[1];
        picture_control_set_ptr->parent_pcs_ptr->cb_sse = (uint32_t)sseTotal[2];
    }
}

void PadRefAndSetFlags(
    PictureControlSet_t    *picture_control_set_ptr,
    SequenceControlSet   *sequence_control_set_ptr
)
{

    EbReferenceObject   *referenceObject = (EbReferenceObject*)picture_control_set_ptr->parent_pcs_ptr->reference_picture_wrapper_ptr->object_ptr;
    EbPictureBufferDesc_t *refPicPtr = (EbPictureBufferDesc_t*)referenceObject->reference_picture;
    EbPictureBufferDesc_t *refPic16BitPtr = (EbPictureBufferDesc_t*)referenceObject->reference_picture16bit;
    EbBool                is16bit = (sequence_control_set_ptr->static_config.encoder_bit_depth > EB_8BIT);

    if (!is16bit) {
        // Y samples
        generate_padding(
            refPicPtr->buffer_y,
            refPicPtr->stride_y,
            refPicPtr->width,
            refPicPtr->height,
            refPicPtr->origin_x,
            refPicPtr->origin_y);

        // Cb samples
        generate_padding(
            refPicPtr->bufferCb,
            refPicPtr->strideCb,
            refPicPtr->width >> 1,
            refPicPtr->height >> 1,
            refPicPtr->origin_x >> 1,
            refPicPtr->origin_y >> 1);

        // Cr samples
        generate_padding(
            refPicPtr->bufferCr,
            refPicPtr->strideCr,
            refPicPtr->width >> 1,
            refPicPtr->height >> 1,
            refPicPtr->origin_x >> 1,
            refPicPtr->origin_y >> 1);
    }

    //We need this for MCP
    if (is16bit) {
        // Y samples
        generate_padding16_bit(
            refPic16BitPtr->buffer_y,
            refPic16BitPtr->stride_y << 1,
            refPic16BitPtr->width << 1,
            refPic16BitPtr->height,
            refPic16BitPtr->origin_x << 1,
            refPic16BitPtr->origin_y);

        // Cb samples
        generate_padding16_bit(
            refPic16BitPtr->bufferCb,
            refPic16BitPtr->strideCb << 1,
            refPic16BitPtr->width,
            refPic16BitPtr->height >> 1,
            refPic16BitPtr->origin_x,
            refPic16BitPtr->origin_y >> 1);

        // Cr samples
        generate_padding16_bit(
            refPic16BitPtr->bufferCr,
            refPic16BitPtr->strideCr << 1,
            refPic16BitPtr->width,
            refPic16BitPtr->height >> 1,
            refPic16BitPtr->origin_x,
            refPic16BitPtr->origin_y >> 1);

    }

    // set up TMVP flag for the reference picture

    referenceObject->tmvp_enable_flag = (picture_control_set_ptr->parent_pcs_ptr->is_used_as_reference_flag) ? EB_TRUE : EB_FALSE;

    // set up the ref POC
    referenceObject->ref_poc = picture_control_set_ptr->parent_pcs_ptr->picture_number;

    // set up the QP
#if ADD_DELTA_QP_SUPPORT
    uint16_t picture_qp = picture_control_set_ptr->parent_pcs_ptr->base_qindex;
    referenceObject->qp = (uint16_t)picture_qp;
#else
    referenceObject->qp = (uint8_t)picture_control_set_ptr->parent_pcs_ptr->picture_qp;
#endif

    // set up the Slice Type
    referenceObject->slice_type = picture_control_set_ptr->parent_pcs_ptr->slice_type;


}

void CopyStatisticsToRefObject(
    PictureControlSet_t    *picture_control_set_ptr,
    SequenceControlSet   *sequence_control_set_ptr
)
{
    picture_control_set_ptr->intra_coded_area = (100 * picture_control_set_ptr->intra_coded_area) / (sequence_control_set_ptr->luma_width * sequence_control_set_ptr->luma_height);
    if (picture_control_set_ptr->slice_type == I_SLICE)
        picture_control_set_ptr->intra_coded_area = 0;

    ((EbReferenceObject*)picture_control_set_ptr->parent_pcs_ptr->reference_picture_wrapper_ptr->object_ptr)->intra_coded_area = (uint8_t)(picture_control_set_ptr->intra_coded_area);

    uint32_t sb_index;
    for (sb_index = 0; sb_index < picture_control_set_ptr->sb_total_count; ++sb_index) {
        ((EbReferenceObject*)picture_control_set_ptr->parent_pcs_ptr->reference_picture_wrapper_ptr->object_ptr)->non_moving_index_array[sb_index] = picture_control_set_ptr->parent_pcs_ptr->non_moving_index_array[sb_index];
    }

    EbReferenceObject  * refObjL0, *refObjL1;
    ((EbReferenceObject*)picture_control_set_ptr->parent_pcs_ptr->reference_picture_wrapper_ptr->object_ptr)->penalize_skipflag = EB_FALSE;
    if (picture_control_set_ptr->slice_type == B_SLICE) {
        refObjL0 = (EbReferenceObject*)picture_control_set_ptr->ref_pic_ptr_array[REF_LIST_0]->object_ptr;
        refObjL1 = (EbReferenceObject*)picture_control_set_ptr->ref_pic_ptr_array[REF_LIST_1]->object_ptr;

        if (picture_control_set_ptr->temporal_layer_index == 0) {
            if (picture_control_set_ptr->parent_pcs_ptr->intra_coded_block_probability > 30) {
                ((EbReferenceObject*)picture_control_set_ptr->parent_pcs_ptr->reference_picture_wrapper_ptr->object_ptr)->penalize_skipflag = EB_TRUE;
            }
        }
        else {
            ((EbReferenceObject*)picture_control_set_ptr->parent_pcs_ptr->reference_picture_wrapper_ptr->object_ptr)->penalize_skipflag = (refObjL0->penalize_skipflag || refObjL1->penalize_skipflag) ? EB_TRUE : EB_FALSE;
        }
    }
    ((EbReferenceObject*)picture_control_set_ptr->parent_pcs_ptr->reference_picture_wrapper_ptr->object_ptr)->tmp_layer_idx = (uint8_t)picture_control_set_ptr->temporal_layer_index;
    ((EbReferenceObject*)picture_control_set_ptr->parent_pcs_ptr->reference_picture_wrapper_ptr->object_ptr)->is_scene_change = picture_control_set_ptr->parent_pcs_ptr->scene_change_flag;

    ((EbReferenceObject*)picture_control_set_ptr->parent_pcs_ptr->reference_picture_wrapper_ptr->object_ptr)->cdef_frame_strength = picture_control_set_ptr->parent_pcs_ptr->cdef_frame_strength;

    Av1Common* cm = picture_control_set_ptr->parent_pcs_ptr->av1_cm;
    ((EbReferenceObject*)picture_control_set_ptr->parent_pcs_ptr->reference_picture_wrapper_ptr->object_ptr)->sg_frame_ep = cm->sg_frame_ep;
}


EbErrorType QpmDeriveWeightsMinAndMax(
    PictureControlSet_t                    *picture_control_set_ptr,
    EncDecContext_t                        *context_ptr)
{
    EbErrorType                    return_error = EB_ErrorNone;
    uint32_t cu_depth;
    context_ptr->min_delta_qp_weight = encMinDeltaQpWeightTab[picture_control_set_ptr->temporal_layer_index];
    context_ptr->max_delta_qp_weight = encMaxDeltaQpWeightTab[picture_control_set_ptr->temporal_layer_index];
    //QpmDeriveDeltaQpMapWeights


    EbBool adjust_min_qp_flag = EB_FALSE;

    adjust_min_qp_flag = picture_control_set_ptr->adjust_min_qp_flag;
    context_ptr->min_delta_qp_weight = 100;
    context_ptr->max_delta_qp_weight = 100;

    {
        if (picture_control_set_ptr->slice_type == I_SLICE) {
            if (picture_control_set_ptr->parent_pcs_ptr->percentage_of_edgein_light_background > 0 && picture_control_set_ptr->parent_pcs_ptr->percentage_of_edgein_light_background <= 3
                && !adjust_min_qp_flag && picture_control_set_ptr->parent_pcs_ptr->dark_back_groundlight_fore_ground) {
                context_ptr->min_delta_qp_weight = 100;
            }
            else {
                if (adjust_min_qp_flag) {


                    context_ptr->min_delta_qp_weight = 250;

                }
                else if (picture_control_set_ptr->parent_pcs_ptr->pic_homogenous_over_time_sb_percentage > 30) {

                    context_ptr->min_delta_qp_weight = 150;
                    context_ptr->max_delta_qp_weight = 50;
                }
            }
        }
        else {
            if (adjust_min_qp_flag) {
                context_ptr->min_delta_qp_weight = 170;
            }
        }
        if (picture_control_set_ptr->parent_pcs_ptr->high_dark_area_density_flag) {
            context_ptr->min_delta_qp_weight = 25;
            context_ptr->max_delta_qp_weight = 25;
        }
    }

    // Refine max_delta_qp_weight; apply conservative max_degrade_weight when most of the picture is homogenous over time.
    if (picture_control_set_ptr->parent_pcs_ptr->pic_homogenous_over_time_sb_percentage > 90) {
        context_ptr->max_delta_qp_weight = context_ptr->max_delta_qp_weight >> 1;
    }


    for (cu_depth = 0; cu_depth < 4; cu_depth++) {
        context_ptr->min_delta_qp[cu_depth] = picture_control_set_ptr->slice_type == I_SLICE ? encMinDeltaQpISliceTab[cu_depth] : encMinDeltaQpTab[cu_depth][picture_control_set_ptr->temporal_layer_index];
        context_ptr->max_delta_qp[cu_depth] = encMaxDeltaQpTab[cu_depth][picture_control_set_ptr->temporal_layer_index];
    }

    return return_error;
}
/******************************************************
* Derive EncDec Settings for OQ
Input   : encoder mode and tune
Output  : EncDec Kernel signal(s)
******************************************************/
EbErrorType signal_derivation_enc_dec_kernel_oq(
    SequenceControlSet    *sequence_control_set_ptr,

    PictureControlSet_t     *picture_control_set_ptr,
    ModeDecisionContext_t   *context_ptr) {

    EbErrorType return_error = EB_ErrorNone;

    // NFL Level MD       Settings
    // 0                  MAX_NFL 40
    // 1                  30
    // 2                  12
    // 3                  10
    // 4                  8
    // 5                  6
    // 6                  4  
    // 7                  3 
#if SCENE_CONTENT_SETTINGS
    if (picture_control_set_ptr->parent_pcs_ptr->sc_content_detected) {
        
        if (picture_control_set_ptr->enc_mode == ENC_M0)
            context_ptr->nfl_level = 2;
        else if (picture_control_set_ptr->enc_mode <= ENC_M3)
            context_ptr->nfl_level = 4;
        else if (picture_control_set_ptr->enc_mode <= ENC_M7)
            context_ptr->nfl_level = 5;
        else
            if (picture_control_set_ptr->parent_pcs_ptr->slice_type == I_SLICE)
                context_ptr->nfl_level  = 5;
            else if (picture_control_set_ptr->parent_pcs_ptr->is_used_as_reference_flag)
                context_ptr->nfl_level  = 6;
            else
                context_ptr->nfl_level  = 7;

    }
    else {
#endif
    if (picture_control_set_ptr->enc_mode == ENC_M0)
        if (picture_control_set_ptr->parent_pcs_ptr->is_used_as_reference_flag)
            context_ptr->nfl_level = (sequence_control_set_ptr->input_resolution <= INPUT_SIZE_576p_RANGE_OR_LOWER) ? 0 : 1;
        else
            context_ptr->nfl_level = 2;
    else if (picture_control_set_ptr->enc_mode <= ENC_M1)
        if (picture_control_set_ptr->parent_pcs_ptr->is_used_as_reference_flag)
            context_ptr->nfl_level = 2;
        else
            context_ptr->nfl_level = 3;
    else if (picture_control_set_ptr->enc_mode <= ENC_M3)
        if (picture_control_set_ptr->parent_pcs_ptr->is_used_as_reference_flag)
            context_ptr->nfl_level = 2;
        else
            context_ptr->nfl_level = 4;
    else if (picture_control_set_ptr->enc_mode <= ENC_M5)
        if (picture_control_set_ptr->parent_pcs_ptr->is_used_as_reference_flag)
            context_ptr->nfl_level = 4;
        else
            context_ptr->nfl_level = 5;
    else if(picture_control_set_ptr->enc_mode <= ENC_M7)
        context_ptr->nfl_level = 5;
    else
        if (picture_control_set_ptr->parent_pcs_ptr->slice_type == I_SLICE)
            context_ptr->nfl_level  = 5;
        else if (picture_control_set_ptr->parent_pcs_ptr->is_used_as_reference_flag)
            context_ptr->nfl_level  = 6;
        else
            context_ptr->nfl_level  = 7;
#if SCENE_CONTENT_SETTINGS
    }
#endif
    // Set Chroma Mode
    // Level                Settings
    // CHROMA_MODE_0  0     Chroma @ MD
    // CHROMA_MODE_1  1     Chroma blind @ MD + CFL @ EP
    // CHROMA_MODE_2  2     Chroma blind @ MD + no CFL @ EP
    if (picture_control_set_ptr->enc_mode <= ENC_M4)
        context_ptr->chroma_level = CHROMA_MODE_0;
    else 
        context_ptr->chroma_level = (sequence_control_set_ptr->encoder_bit_depth == EB_8BIT) ?
            CHROMA_MODE_1 :
            CHROMA_MODE_2 ;

    
    // Set fast loop method
    // 1 fast loop: SSD_SEARCH not supported    
    // Level                Settings
    //  0                   Collapsed fast loop
    //  1                   Decoupled fast loops ( intra/inter) 

    if (picture_control_set_ptr->enc_mode == ENC_M0)
        context_ptr->decouple_intra_inter_fast_loop = 0;
    else
        context_ptr->decouple_intra_inter_fast_loop = 1;

    // Set the search method when decoupled fast loop is used 
    // Hsan: FULL_SAD_SEARCH not supported
#if SCENE_CONTENT_SETTINGS

    if (picture_control_set_ptr->parent_pcs_ptr->sc_content_detected){
	    if (picture_control_set_ptr->enc_mode <= ENC_M0)
	        context_ptr->decoupled_fast_loop_search_method = SSD_SEARCH;
	    else
	        context_ptr->decoupled_fast_loop_search_method = FULL_SAD_SEARCH;
	}else
#endif
    if (picture_control_set_ptr->enc_mode <= ENC_M5)
        context_ptr->decoupled_fast_loop_search_method = SSD_SEARCH;
    else
        context_ptr->decoupled_fast_loop_search_method = FULL_SAD_SEARCH;
    // Set the full loop escape level
    // Level                Settings
    // 0                    Off
    // 1                    On but only INTRA
    // 2                    On both INTRA and INTER
    if (picture_control_set_ptr->enc_mode <= ENC_M7)
        context_ptr->full_loop_escape = 0;
    else
        context_ptr->full_loop_escape = 1;


    // Set global MV injection
    // Level                Settings
    // 0                    Injection off (Hsan: but not derivation as used by MV ref derivation)
    // 1                    On
    if (picture_control_set_ptr->enc_mode <= ENC_M7)
        context_ptr->global_mv_injection = 1;
    else
        context_ptr->global_mv_injection = 0;

    
    // Set warped motion injection
    // Level                Settings
    // 0                    OFF
    // 1                    On
#if SCENE_CONTENT_SETTINGS
    if (picture_control_set_ptr->parent_pcs_ptr->sc_content_detected) 
        if (picture_control_set_ptr->enc_mode == ENC_M0)
            context_ptr->warped_motion_injection = 1;
        else
            context_ptr->warped_motion_injection = 0;
    else

#endif
    if (picture_control_set_ptr->enc_mode <= ENC_M5)
        context_ptr->warped_motion_injection = 1;
    else
        context_ptr->warped_motion_injection = 0;

    
    // Set unipred3x3 injection
    // Level                Settings
    // 0                    OFF
    // 1                    ON FULL
    // 2                    Reduced set
#if SCENE_CONTENT_SETTINGS
    if (picture_control_set_ptr->parent_pcs_ptr->sc_content_detected)
        if (picture_control_set_ptr->enc_mode == ENC_M0)
            context_ptr->unipred3x3_injection = 1;
        else
            context_ptr->unipred3x3_injection = 0;
    else

#endif
    if (picture_control_set_ptr->enc_mode == ENC_M0)
        context_ptr->unipred3x3_injection = 1;
    else if (picture_control_set_ptr->enc_mode <= ENC_M3)
        context_ptr->unipred3x3_injection = 2;
    else
        context_ptr->unipred3x3_injection = 0;

    
    // Set bipred3x3 injection
    // Level                Settings
    // 0                    OFF
    // 1                    ON FULL
    // 2                    Reduced set
#if SCENE_CONTENT_SETTINGS
    if (picture_control_set_ptr->parent_pcs_ptr->sc_content_detected)
        if (picture_control_set_ptr->enc_mode == ENC_M0)
            context_ptr->bipred3x3_injection = 1;
        else
            context_ptr->bipred3x3_injection = 0;
    else

#endif
    if (picture_control_set_ptr->enc_mode == ENC_M0)
        context_ptr->bipred3x3_injection = 1;
    else if (picture_control_set_ptr->enc_mode <= ENC_M3)
        context_ptr->bipred3x3_injection = 2;
    else
        context_ptr->bipred3x3_injection = 0;
      
    // Set interpolation filter search blk size
    // Level                Settings
    // 0                    ON for 8x8 and above
    // 1                    ON for 16x16 and above
    // 2                    ON for 32x32 and above
#if SCENE_CONTENT_SETTINGS
    if (picture_control_set_ptr->parent_pcs_ptr->sc_content_detected)
        context_ptr->interpolation_filter_search_blk_size = 0;
    else
#endif
    if (picture_control_set_ptr->enc_mode == ENC_M0)
        context_ptr->interpolation_filter_search_blk_size = 0;
    else if (picture_control_set_ptr->enc_mode <= ENC_M2)
        context_ptr->interpolation_filter_search_blk_size = 1;
    else        
        context_ptr->interpolation_filter_search_blk_size = 2;
    


    return return_error;
}
void move_cu_data(
    CodingUnit_t *src_cu,
    CodingUnit_t *dst_cu);

/******************************************************
 * EncDec Kernel
 ******************************************************/
void* EncDecKernel(void *input_ptr)
{
    // Context & SCS & PCS
    EncDecContext_t                         *context_ptr = (EncDecContext_t*)input_ptr;
    PictureControlSet_t                     *picture_control_set_ptr;
    SequenceControlSet                    *sequence_control_set_ptr;

    // Input
    EbObjectWrapper                       *encDecTasksWrapperPtr;
    EncDecTasks_t                           *encDecTasksPtr;

    // Output
    EbObjectWrapper                       *encDecResultsWrapperPtr;
    EncDecResults_t                         *encDecResultsPtr;
    // SB Loop variables
    LargestCodingUnit_t                     *sb_ptr;
    uint16_t                                 sb_index;
    uint8_t                                  sb_sz;
    uint8_t                                  lcuSizeLog2;
    uint32_t                                 xLcuIndex;
    uint32_t                                 yLcuIndex;
    uint32_t                                 sb_origin_x;
    uint32_t                                 sb_origin_y;
    EbBool                                   lastLcuFlag;
    EbBool                                   endOfRowFlag;
    uint32_t                                 lcuRowIndexStart;
    uint32_t                                 lcuRowIndexCount;
    uint32_t                                 picture_width_in_sb;
    MdcLcuData_t                            *mdcPtr;

    // Variables
    EbBool                                   is16bit;

    // Segments
    //EbBool                                 initialProcessCall;
    uint16_t                                 segment_index;
    uint32_t                                 xLcuStartIndex;
    uint32_t                                 yLcuStartIndex;
    uint32_t                                 lcuStartIndex;
    uint32_t                                 lcuSegmentCount;
    uint32_t                                 lcuSegmentIndex;
    uint32_t                                 segmentRowIndex;
    uint32_t                                 segmentBandIndex;
    uint32_t                                 segmentBandSize;
    EncDecSegments_t                        *segmentsPtr;
    for (;;) {

        // Get Mode Decision Results
        eb_get_full_object(
            context_ptr->mode_decision_input_fifo_ptr,
            &encDecTasksWrapperPtr);

        encDecTasksPtr = (EncDecTasks_t*)encDecTasksWrapperPtr->object_ptr;
        picture_control_set_ptr = (PictureControlSet_t*)encDecTasksPtr->picture_control_set_wrapper_ptr->object_ptr;
        sequence_control_set_ptr = (SequenceControlSet*)picture_control_set_ptr->sequence_control_set_wrapper_ptr->object_ptr;
        segmentsPtr = picture_control_set_ptr->enc_dec_segment_ctrl;
        lastLcuFlag = EB_FALSE;
        is16bit = (EbBool)(sequence_control_set_ptr->static_config.encoder_bit_depth > EB_8BIT);
        (void)is16bit;
        (void)endOfRowFlag;

        // EncDec Kernel Signal(s) derivation

        signal_derivation_enc_dec_kernel_oq(
            sequence_control_set_ptr,
            picture_control_set_ptr,
            context_ptr->md_context);

        // SB Constants
        sb_sz = (uint8_t)sequence_control_set_ptr->sb_size_pix;
        lcuSizeLog2 = (uint8_t)Log2f(sb_sz);
        context_ptr->sb_sz = sb_sz;
        picture_width_in_sb = (sequence_control_set_ptr->luma_width + sb_sz - 1) >> lcuSizeLog2;
        endOfRowFlag = EB_FALSE;
        lcuRowIndexStart = lcuRowIndexCount = 0;
        context_ptr->tot_intra_coded_area = 0;

        // Segment-loop
        while (AssignEncDecSegments(segmentsPtr, &segment_index, encDecTasksPtr, context_ptr->enc_dec_feedback_fifo_ptr) == EB_TRUE)
        {
            xLcuStartIndex = segmentsPtr->xStartArray[segment_index];
            yLcuStartIndex = segmentsPtr->yStartArray[segment_index];
            lcuStartIndex = yLcuStartIndex * picture_width_in_sb + xLcuStartIndex;
            lcuSegmentCount = segmentsPtr->validLcuCountArray[segment_index];

            segmentRowIndex = segment_index / segmentsPtr->segmentBandCount;
            segmentBandIndex = segment_index - segmentRowIndex * segmentsPtr->segmentBandCount;
            segmentBandSize = (segmentsPtr->lcuBandCount * (segmentBandIndex + 1) + segmentsPtr->segmentBandCount - 1) / segmentsPtr->segmentBandCount;

            // Reset Coding Loop State
            reset_mode_decision( // HT done
                context_ptr->md_context,
                picture_control_set_ptr,
                sequence_control_set_ptr,
                segment_index);

            // Reset EncDec Coding State
            ResetEncDec(    // HT done
                context_ptr,
                picture_control_set_ptr,
                sequence_control_set_ptr,
                segment_index);

            if (picture_control_set_ptr->parent_pcs_ptr->reference_picture_wrapper_ptr != NULL) {
                ((EbReferenceObject  *)picture_control_set_ptr->parent_pcs_ptr->reference_picture_wrapper_ptr->object_ptr)->average_intensity = picture_control_set_ptr->parent_pcs_ptr->average_intensity[0];
            }

            if (sequence_control_set_ptr->static_config.improve_sharpness) {
                QpmDeriveWeightsMinAndMax(
                    picture_control_set_ptr,
                    context_ptr);
            }
            for (yLcuIndex = yLcuStartIndex, lcuSegmentIndex = lcuStartIndex; lcuSegmentIndex < lcuStartIndex + lcuSegmentCount; ++yLcuIndex) {
                for (xLcuIndex = xLcuStartIndex; xLcuIndex < picture_width_in_sb && (xLcuIndex + yLcuIndex < segmentBandSize) && lcuSegmentIndex < lcuStartIndex + lcuSegmentCount; ++xLcuIndex, ++lcuSegmentIndex) {

                    sb_index = (uint16_t)(yLcuIndex * picture_width_in_sb + xLcuIndex);
                    sb_ptr = picture_control_set_ptr->sb_ptr_array[sb_index];
                    sb_origin_x = xLcuIndex << lcuSizeLog2;
                    sb_origin_y = yLcuIndex << lcuSizeLog2;
                    lastLcuFlag = (sb_index == sequence_control_set_ptr->sb_tot_cnt - 1) ? EB_TRUE : EB_FALSE;
                    endOfRowFlag = (xLcuIndex == picture_width_in_sb - 1) ? EB_TRUE : EB_FALSE;
                    lcuRowIndexStart = (xLcuIndex == picture_width_in_sb - 1 && lcuRowIndexCount == 0) ? yLcuIndex : lcuRowIndexStart;
                    lcuRowIndexCount = (xLcuIndex == picture_width_in_sb - 1) ? lcuRowIndexCount + 1 : lcuRowIndexCount;
                    mdcPtr = &picture_control_set_ptr->mdc_sb_array[sb_index];
                    context_ptr->sb_index = sb_index;
                    context_ptr->md_context->cu_use_ref_src_flag = (picture_control_set_ptr->parent_pcs_ptr->use_src_ref) && (picture_control_set_ptr->parent_pcs_ptr->edge_results_ptr[sb_index].edge_block_num == EB_FALSE || picture_control_set_ptr->parent_pcs_ptr->sb_flat_noise_array[sb_index]) ? EB_TRUE : EB_FALSE;

                    // Configure the LCU
                    ModeDecisionConfigureLcu(
                        context_ptr->md_context,
                        sb_ptr,
                        picture_control_set_ptr,
                        sequence_control_set_ptr,
                        (uint8_t)context_ptr->qp,
                        (uint8_t)sb_ptr->qp);

                    uint32_t lcuRow;
                    if (picture_control_set_ptr->parent_pcs_ptr->enable_in_loop_motion_estimation_flag) {

                        EbPictureBufferDesc_t       *input_picture_ptr;

                        input_picture_ptr = picture_control_set_ptr->parent_pcs_ptr->enhanced_picture_ptr;

                        // Load the SB from the input to the intermediate SB buffer
                        uint32_t bufferIndex = (input_picture_ptr->origin_y + sb_origin_y) * input_picture_ptr->stride_y + input_picture_ptr->origin_x + sb_origin_x;

                        // Copy the source superblock to the me local buffer
                        uint32_t sb_height = (sequence_control_set_ptr->luma_height - sb_origin_y) < MAX_SB_SIZE ? sequence_control_set_ptr->luma_height - sb_origin_y : MAX_SB_SIZE;
                        uint32_t sb_width = (sequence_control_set_ptr->luma_width - sb_origin_x) < MAX_SB_SIZE ? sequence_control_set_ptr->luma_width - sb_origin_x : MAX_SB_SIZE;
                        uint32_t is_complete_sb = sequence_control_set_ptr->sb_geom[sb_index].is_complete_sb;

                        if (!is_complete_sb) {
                            memset(context_ptr->ss_mecontext->sb_buffer, 0, MAX_SB_SIZE*MAX_SB_SIZE);
                        }
                        for (lcuRow = 0; lcuRow < sb_height; lcuRow++) {
                            EB_MEMCPY((&(context_ptr->ss_mecontext->sb_buffer[lcuRow * MAX_SB_SIZE])), (&(input_picture_ptr->buffer_y[bufferIndex + lcuRow * input_picture_ptr->stride_y])), sb_width * sizeof(uint8_t));
                        }

                        context_ptr->ss_mecontext->sb_src_ptr = &(context_ptr->ss_mecontext->sb_buffer[0]);
                        context_ptr->ss_mecontext->sb_src_stride = context_ptr->ss_mecontext->sb_buffer_stride;
                        // Set in-loop ME Search Area
                        int16_t mv_l0_x;
                        int16_t mv_l0_y;
                        int16_t mv_l1_x;
                        int16_t mv_l1_y;
                        uint32_t me_sb_addr;

                        if (sequence_control_set_ptr->sb_size == BLOCK_128X128) {

                            uint32_t me_sb_size = sequence_control_set_ptr->sb_sz;
                            uint32_t me_pic_width_in_sb = (sequence_control_set_ptr->luma_width + me_sb_size - 1) / me_sb_size;
                            uint32_t me_pic_height_in_sb = (sequence_control_set_ptr->luma_height + me_sb_size - 1) / me_sb_size;
                            uint32_t me_sb_x = (sb_origin_x / me_sb_size);
                            uint32_t me_sb_y = (sb_origin_y / me_sb_size);
                            uint32_t me_sb_addr_0 = me_sb_x + me_sb_y * me_pic_width_in_sb;
                            uint32_t me_sb_addr_1 = (me_sb_x + 1) < me_pic_width_in_sb ? (me_sb_x + 1) + ((me_sb_y + 0) * me_pic_width_in_sb) : me_sb_addr_0;
                            uint32_t me_sb_addr_2 = (me_sb_y + 1) < me_pic_height_in_sb ? (me_sb_x + 0) + ((me_sb_y + 1) * me_pic_width_in_sb) : me_sb_addr_0;
                            uint32_t me_sb_addr_3 = ((me_sb_x + 1) < me_pic_width_in_sb) && ((me_sb_y + 1) < me_pic_height_in_sb) ? (me_sb_x + 1) + ((me_sb_y + 1) * me_pic_width_in_sb) : me_sb_addr_0;

                            MeCuResults_t * me_block_results_0 = &picture_control_set_ptr->parent_pcs_ptr->me_results[me_sb_addr_0][0];
                            MeCuResults_t * me_block_results_1 = &picture_control_set_ptr->parent_pcs_ptr->me_results[me_sb_addr_1][0];
                            MeCuResults_t * me_block_results_2 = &picture_control_set_ptr->parent_pcs_ptr->me_results[me_sb_addr_2][0];
                            MeCuResults_t * me_block_results_3 = &picture_control_set_ptr->parent_pcs_ptr->me_results[me_sb_addr_3][0];

                            // Compute average open_loop 64x64 MVs
                            mv_l0_x = ((me_block_results_0->xMvL0 + me_block_results_1->xMvL0 + me_block_results_2->xMvL0 + me_block_results_3->xMvL0) >> 2) >> 2;
                            mv_l0_y = ((me_block_results_0->yMvL0 + me_block_results_1->yMvL0 + me_block_results_2->yMvL0 + me_block_results_3->yMvL0) >> 2) >> 2;
                            mv_l1_x = ((me_block_results_0->xMvL1 + me_block_results_1->xMvL1 + me_block_results_2->xMvL1 + me_block_results_3->xMvL1) >> 2) >> 2;
                            mv_l1_y = ((me_block_results_0->yMvL1 + me_block_results_1->yMvL1 + me_block_results_2->yMvL1 + me_block_results_3->yMvL1) >> 2) >> 2;

                        }
                        else {
                            me_sb_addr = sb_index;
                            MeCuResults_t * mePuResult = &picture_control_set_ptr->parent_pcs_ptr->me_results[me_sb_addr][0];

                            mv_l0_x = mePuResult->xMvL0 >> 2;
                            mv_l0_y = mePuResult->yMvL0 >> 2;
                            mv_l1_x = mePuResult->xMvL1 >> 2;
                            mv_l1_y = mePuResult->yMvL1 >> 2;
                        }


                        context_ptr->ss_mecontext->search_area_width = 64;
                        context_ptr->ss_mecontext->search_area_height = 64;

                        // perform in-loop ME
                        in_loop_motion_estimation_sblock(
                            picture_control_set_ptr,
                            sb_origin_x,
                            sb_origin_y,
                            mv_l0_x,
                            mv_l0_y,
                            mv_l1_x,
                            mv_l1_y,
                            context_ptr->ss_mecontext);
                    }

                    mode_decision_sb(
                        sequence_control_set_ptr,
                        picture_control_set_ptr,
                        mdcPtr,
                        sb_ptr,
                        sb_origin_x,
                        sb_origin_y,
                        sb_index,
                        context_ptr->ss_mecontext,
                        context_ptr->md_context);


                    // Configure the LCU
                    EncDecConfigureLcu(
                        context_ptr,
                        sb_ptr,
                        picture_control_set_ptr,
                        sequence_control_set_ptr,
                        (uint8_t)context_ptr->qp,
                        (uint8_t)sb_ptr->qp);

#if NO_ENCDEC
                    no_enc_dec_pass(
                        sequence_control_set_ptr,
                        picture_control_set_ptr,
                        sb_ptr,
                        sb_index,
                        sb_origin_x,
                        sb_origin_y,
                        sb_ptr->qp,
                        context_ptr);
#else
                    // Encode Pass
                    AV1EncodePass(
                        sequence_control_set_ptr,
                        picture_control_set_ptr,
                        sb_ptr,
                        sb_index,
                        sb_origin_x,
                        sb_origin_y,
                        sb_ptr->qp,
                        context_ptr);
#endif

                    if (picture_control_set_ptr->parent_pcs_ptr->reference_picture_wrapper_ptr != NULL) {
                        ((EbReferenceObject*)picture_control_set_ptr->parent_pcs_ptr->reference_picture_wrapper_ptr->object_ptr)->intra_coded_area_sb[sb_index] = (uint8_t)((100 * context_ptr->intra_coded_area_sb[sb_index]) / (64 * 64));
                    }

                }
                xLcuStartIndex = (xLcuStartIndex > 0) ? xLcuStartIndex - 1 : 0;
            }
        }

        eb_block_on_mutex(picture_control_set_ptr->intra_mutex);
        picture_control_set_ptr->intra_coded_area += (uint32_t)context_ptr->tot_intra_coded_area;
        eb_release_mutex(picture_control_set_ptr->intra_mutex);

        if (lastLcuFlag) {

            // Copy film grain data from parent picture set to the reference object for further reference
            if (sequence_control_set_ptr->film_grain_params_present)
            {

                if (picture_control_set_ptr->parent_pcs_ptr->is_used_as_reference_flag == EB_TRUE && picture_control_set_ptr->parent_pcs_ptr->reference_picture_wrapper_ptr) {

                    ((EbReferenceObject*)picture_control_set_ptr->parent_pcs_ptr->reference_picture_wrapper_ptr->object_ptr)->film_grain_params
                        = picture_control_set_ptr->parent_pcs_ptr->film_grain_params;
                }
            }

            EB_MEMCPY(picture_control_set_ptr->parent_pcs_ptr->av1x->sgrproj_restore_cost, context_ptr->md_rate_estimation_ptr->sgrprojRestoreFacBits, 2 * sizeof(int32_t));
            EB_MEMCPY(picture_control_set_ptr->parent_pcs_ptr->av1x->switchable_restore_cost, context_ptr->md_rate_estimation_ptr->switchableRestoreFacBits, 3 * sizeof(int32_t));
            EB_MEMCPY(picture_control_set_ptr->parent_pcs_ptr->av1x->wiener_restore_cost, context_ptr->md_rate_estimation_ptr->wienerRestoreFacBits, 2 * sizeof(int32_t));
            picture_control_set_ptr->parent_pcs_ptr->av1x->rdmult = context_ptr->full_lambda;


        }



        if (lastLcuFlag)
        {

            // Get Empty EncDec Results
            eb_get_empty_object(
                context_ptr->enc_dec_output_fifo_ptr,
                &encDecResultsWrapperPtr);
            encDecResultsPtr = (EncDecResults_t*)encDecResultsWrapperPtr->object_ptr;
            encDecResultsPtr->picture_control_set_wrapper_ptr = encDecTasksPtr->picture_control_set_wrapper_ptr;
            //CHKN these are not needed for DLF
            encDecResultsPtr->completedLcuRowIndexStart = 0;
            encDecResultsPtr->completedLcuRowCount = ((sequence_control_set_ptr->luma_height + sequence_control_set_ptr->sb_size_pix - 1) >> lcuSizeLog2);
            // Post EncDec Results
            eb_post_full_object(encDecResultsWrapperPtr);

        }
        // Release Mode Decision Results
        eb_release_object(encDecTasksWrapperPtr);

    }
    return EB_NULL;
}

void av1_add_film_grain(EbPictureBufferDesc_t *src,
    EbPictureBufferDesc_t *dst,
    aom_film_grain_t *film_grain_ptr) {
    uint8_t *luma, *cb, *cr;
    int32_t height, width, luma_stride, chroma_stride;
    int32_t use_high_bit_depth = 0;
    int32_t chroma_subsamp_x = 0;
    int32_t chroma_subsamp_y = 0;

    aom_film_grain_t params = *film_grain_ptr;

    switch (src->bit_depth) {
    case EB_8BIT:
        params.bit_depth = 8;
        use_high_bit_depth = 0;
        chroma_subsamp_x = 1;
        chroma_subsamp_y = 1;
        break;
    case EB_10BIT:
        params.bit_depth = 10;
        use_high_bit_depth = 1;
        chroma_subsamp_x = 1;
        chroma_subsamp_y = 1;
        break;
    default:  //todo: Throw an error if unknown format?
        params.bit_depth = 10;
        use_high_bit_depth = 1;
        chroma_subsamp_x = 1;
        chroma_subsamp_y = 1;
    }

    dst->maxWidth = src->maxWidth;
    dst->maxHeight = src->maxHeight;

    fgn_copy_rect(src->buffer_y + ((src->origin_y * src->stride_y + src->origin_x) << use_high_bit_depth), src->stride_y,
        dst->buffer_y + ((dst->origin_y * dst->stride_y + dst->origin_x) << use_high_bit_depth), dst->stride_y,
        dst->width, dst->height, use_high_bit_depth);

    fgn_copy_rect(src->bufferCb + ((src->strideCb * (src->origin_y >> chroma_subsamp_y)
        + (src->origin_x >> chroma_subsamp_x)) << use_high_bit_depth), src->strideCb,
        dst->bufferCb + ((dst->strideCb * (dst->origin_y >> chroma_subsamp_y)
            + (dst->origin_x >> chroma_subsamp_x)) << use_high_bit_depth), dst->strideCb,
        dst->width >> chroma_subsamp_x, dst->height >> chroma_subsamp_y,
        use_high_bit_depth);

    fgn_copy_rect(src->bufferCr + ((src->strideCr * (src->origin_y >> chroma_subsamp_y)
        + (src->origin_x >> chroma_subsamp_x)) << use_high_bit_depth), src->strideCr,
        dst->bufferCr + ((dst->strideCr * (dst->origin_y >> chroma_subsamp_y)
            + (dst->origin_x >> chroma_subsamp_x)) << use_high_bit_depth), dst->strideCr,
        dst->width >> chroma_subsamp_x, dst->height >> chroma_subsamp_y,
        use_high_bit_depth);

    luma = dst->buffer_y + ((dst->origin_y * dst->stride_y + dst->origin_x) << use_high_bit_depth);
    cb = dst->bufferCb + ((dst->strideCb * (dst->origin_y >> chroma_subsamp_y)
        + (dst->origin_x >> chroma_subsamp_x)) << use_high_bit_depth);
    cr = dst->bufferCr + ((dst->strideCr * (dst->origin_y >> chroma_subsamp_y)
        + (dst->origin_x >> chroma_subsamp_x)) << use_high_bit_depth);

    luma_stride = dst->stride_y;
    chroma_stride = dst->strideCb;

    width = dst->width;
    height = dst->height;

    av1_add_film_grain_run(&params, luma, cb, cr, height, width, luma_stride,
        chroma_stride, use_high_bit_depth, chroma_subsamp_y,
        chroma_subsamp_x);
    return;
}

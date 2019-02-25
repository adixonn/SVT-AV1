/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#ifndef EbModeDecision_h
#define EbModeDecision_h

#include "EbDefinitions.h"
#include "EbUtility.h"
#include "EbPictureControlSet.h"
#include "EbCodingUnit.h"
#include "EbPredictionUnit.h"
#include "EbSyntaxElements.h"
#include "EbPictureBufferDesc.h"
#include "EbAdaptiveMotionVectorPrediction.h"
#include "EbPictureOperators.h"
#include "EbNeighborArrays.h"
#ifdef __cplusplus
extern "C" {
#endif
#define ENABLE_AMVP_MV_FOR_RC_PU    0
#define MAX_MB_PLANE                3
#define MAX_MPM_CANDIDATES          3
#define MERGE_PENALTY                          10

    // Create incomplete struct definition for the following function pointer typedefs
    struct ModeDecisionCandidateBuffer;
    struct ModeDecisionContext;

    /**************************************
    * Function Ptrs Definitions
    **************************************/
    typedef EbErrorType(*EB_PREDICTION_FUNC)(
        struct ModeDecisionContext           *context_ptr,
#if !CHROMA_BLIND
        uint32_t                                component_mask,
#endif
        PictureControlSet_t                    *picture_control_set_ptr,
        struct ModeDecisionCandidateBuffer   *candidate_buffer_ptr,
        EbAsm                                   asm_type);

    typedef EbErrorType(*EB_FAST_COST_FUNC)(
        struct ModeDecisionContext           *context_ptr,
        CodingUnit                           *cu_ptr,
        struct ModeDecisionCandidateBuffer   *candidate_buffer_ptr,
        uint32_t                                qp,
        uint64_t                                luma_distortion,
        uint64_t                                chroma_distortion,
        uint64_t                                lambda,
        PictureControlSet_t                    *picture_control_set_ptr);

    typedef EbErrorType(*EB_FULL_COST_FUNC)(
        LargestCodingUnit                    *sb_ptr,
        CodingUnit                           *cu_ptr,
        uint32_t                                cu_size,
        uint32_t                                cu_size_log2,
        struct ModeDecisionCandidateBuffer   *candidate_buffer_ptr,
        uint32_t                                qp,
        uint64_t                               *y_distortion,
        uint64_t                               *cb_distortion,
        uint64_t                               *cr_distortion,
        uint64_t                                lambda,
        uint64_t                                lambda_chroma,
        uint64_t                               *y_coeff_bits,
        uint64_t                               *cb_coeff_bits,
        uint64_t                               *cr_coeff_bits,
        uint32_t                                transform_size,
        uint32_t                                transform_chroma_size,
        PictureControlSet_t                    *picture_control_set_ptr);

    typedef EbErrorType(*EB_AV1_FULL_COST_FUNC)(
        PictureControlSet_t                    *picture_control_set_ptr,
        struct ModeDecisionContext           *context_ptr,
        struct ModeDecisionCandidateBuffer   *candidate_buffer_ptr,
        CodingUnit                           *cu_ptr,
        uint64_t                               *y_distortion,
        uint64_t                               *cb_distortion,
        uint64_t                               *cr_distortion,
        uint64_t                                lambda,
        uint64_t                               *y_coeff_bits,
        uint64_t                               *cb_coeff_bits,
        uint64_t                               *cr_coeff_bits,
        block_size                               bsize);

    typedef EbErrorType(*EB_FULL_LUMA_COST_FUNC)(
        CodingUnit                           *cu_ptr,
        uint32_t                                cu_size,
        uint32_t                                cu_size_log2,
        struct ModeDecisionCandidateBuffer   *candidate_buffer_ptr,
        uint64_t                               *y_distortion,
        uint64_t                                lambda,
        uint64_t                               *y_coeff_bits,
        uint32_t                                transform_size);

    /**************************************
    * Mode Decision Candidate
    **************************************/
    typedef struct ModeDecisionCandidate
    {
        // *Warning - this struct has been organized to be cache efficient when being
        //    constructured in the function GenerateAmvpMergeInterIntraMdCandidatesCU.
        //    Changing the ordering could affect performance
        union {
            struct {
                unsigned                        me_distortion    : 20;
                unsigned                        distortion_ready : 1;
                unsigned : 3;
                unsigned                        intra_luma_mode  : 8; // HEVC mode, use pred_mode for AV1
            };
            uint32_t ois_results;
        };
        union {
            struct {
                union {
                    struct {
                        int16_t              motionVector_x_L0;  //Note: Do not change the order of these fields
                        int16_t              motionVector_y_L0;
                    };
                    uint32_t MVsL0;
                };
                union {
                    struct {
                        int16_t              motionVector_x_L1;  //Note: Do not change the order of these fields
                        int16_t              motionVector_y_L1;
                    };
                    uint32_t MVsL1;
                };
            };
            uint64_t MVs;
        };

        uint8_t                                skip_flag;
        EbBool                                 merge_flag;
        uint8_t                                merge_index;
        uint16_t                               count_non_zero_coeffs;
        EbBool                                 prediction_is_ready_luma;
        uint8_t                                type;
        EbBool                                 mpm_flag;

        // MD Rate Estimation Ptr
        MdRateEstimationContext             *md_rate_estimation_ptr; // 64 bits
        uint64_t                               fast_luma_rate;
        uint64_t                               fast_chroma_rate;
        uint64_t                               chroma_distortion;
        uint64_t                               chroma_distortion_inter_depth;
        uint32_t                               luma_distortion;
        uint32_t                               full_distortion;

        EbPtr                                 prediction_context_ptr;
        PictureControlSet_t                   *picture_control_set_ptr;
        EbPredDirection                        prediction_direction[MAX_NUM_OF_PU_PER_CU]; // 2 bits

        int16_t                                motion_vector_pred_x[MAX_NUM_OF_REF_PIC_LIST]; // 16 bits
        int16_t                                motion_vector_pred_y[MAX_NUM_OF_REF_PIC_LIST]; // 16 bits
        uint8_t                                motion_vector_pred_idx[MAX_NUM_OF_REF_PIC_LIST]; // 2 bits
        uint8_t                                block_has_coeff;             // ?? bit - determine empirically
        uint8_t                                u_has_coeff;               // ?? bit
        uint8_t                                v_has_coeff;               // ?? bit
        uint32_t                               y_has_coeff;                // Issue, should be less than 32

        PredictionMode                         pred_mode; // AV1 mode, no need to convert
        uint8_t                                drl_index;

        // Intra Mode
        int32_t                                angle_delta[PLANE_TYPES];
        EbBool                                 is_directional_mode_flag;
        EbBool                                 is_directional_chroma_mode_flag;
        EbBool                                 use_angle_delta;
        uint32_t                               intra_chroma_mode; // AV1 mode, no need to convert

        // Index of the alpha Cb and alpha Cr combination
        int32_t                                cfl_alpha_idx;
        // Joint sign of alpha Cb and alpha Cr
        int32_t                                cfl_alpha_signs;

        // Inter Mode
        PredictionMode                         inter_mode;
        EbBool                                 is_compound;
        uint32_t                               pred_mv_weight;
        uint8_t                                ref_frame_type;
        uint8_t                                ref_mv_index;
        EbBool                                 is_skip_mode_flag;
        EbBool                                 is_new_mv;
        EbBool                                 is_zero_mv;
        TxType                                 transform_type[PLANE_TYPES];
        MacroblockPlane                        candidate_plane[MAX_MB_PLANE];
        uint16_t                               eob[MAX_MB_PLANE][MAX_TXB_COUNT];
        int32_t                                quantized_dc[3];
        uint32_t                               interp_filters;
        uint8_t                                tu_width;
        uint8_t                                tu_height;
        MOTION_MODE                            motion_mode;
        uint16_t                               num_proj_ref;
        EbBool                                 local_warp_valid;
        EbWarpedMotionParams                   wm_params;
    } ModeDecisionCandidate;

    /**************************************
    * Mode Decision Candidate Buffer
    **************************************/
    typedef struct IntraChromaCandidateBuffer {
        uint32_t                                mode;
        uint64_t                                cost;
        uint64_t                                distortion;
        EbPictureBufferDesc                  *prediction_ptr;
        EbPictureBufferDesc                  *residual_ptr;
    } IntraChromaCandidateBuffer;

    /**************************************
    * Mode Decision Candidate Buffer
    **************************************/
    typedef struct ModeDecisionCandidateBuffer {
        // Candidate Ptr
        ModeDecisionCandidate                *candidate_ptr;

        // Video Buffers
        EbPictureBufferDesc                  *prediction_ptr;
        EbPictureBufferDesc                  *predictionPtrTemp;
        EbPictureBufferDesc                  *cflTempPredictionPtr;
        EbPictureBufferDesc                  *residualQuantCoeffPtr;// One buffer for residual and quantized coefficient
        EbPictureBufferDesc                  *reconCoeffPtr;
        EbPictureBufferDesc                  *residual_ptr;

        // *Note - We should be able to combine the reconCoeffPtr & recon_ptr pictures (they aren't needed at the same time)
        EbPictureBufferDesc                  *recon_ptr;

        // Distortion (SAD)
        uint64_t                                residual_luma_sad;
        uint64_t                                full_lambda_rate;
        uint64_t                                full_cost_luma;
                                               
        // Costs                               
        uint64_t                               *fast_cost_ptr;
        uint64_t                               *full_cost_ptr;
        uint64_t                               *full_cost_skip_ptr;
        uint64_t                               *full_cost_merge_ptr;
        //                                     
        uint64_t                                cb_coeff_bits;
        uint64_t                                cb_distortion[2];
        uint64_t                                cr_coeff_bits;
        uint64_t                                cr_distortion[2];
        EbBool                                  sub_sampled_pred; //do prediction every other line, duplicate the residual
        EbBool                                  sub_sampled_pred_chroma;
        uint64_t                                y_full_distortion[DIST_CALC_TOTAL];
        uint64_t                                y_coeff_bits;

    } ModeDecisionCandidateBuffer;

    /**************************************
    * Extern Function Declarations
    **************************************/
    extern EbErrorType mode_decision_candidate_buffer_ctor(
        ModeDecisionCandidateBuffer **buffer_dbl_ptr,
        uint16_t                        sb_max_size,
        EB_BITDEPTH                     max_bitdepth,
        uint64_t                       *fast_cost_ptr,
        uint64_t                       *full_cost_ptr,
        uint64_t                       *full_cost_skip_ptr,
        uint64_t                       *full_cost_merge_ptr
    );

    uint8_t product_full_mode_decision(
        struct ModeDecisionContext   *context_ptr,
        CodingUnit                   *cu_ptr,
        uint8_t                         bwidth,
        uint8_t                         bheight,
        ModeDecisionCandidateBuffer **buffer_ptr_array,
        uint32_t                        candidate_total_count,
        uint8_t                        *best_candidate_index_array,
        uint32_t                       *best_intra_mode);

    EbErrorType pre_mode_decision(
        CodingUnit                   *cu_ptr,
        uint32_t                        buffer_total_count,
        ModeDecisionCandidateBuffer **buffer_ptr_array,
        uint32_t                       *full_candidate_total_count_ptr,
        uint8_t                        *best_candidate_index_array,
        uint8_t                        *disable_merge_index,
#if TX_SEARCH_LEVELS
        uint64_t                       *ref_fast_cost,
#endif
        EbBool                          same_fast_full_candidate);

    typedef EbErrorType(*EB_INTRA_4x4_FAST_LUMA_COST_FUNC)(
        struct ModeDecisionContext           *context_ptr,
        uint32_t                                pu_index,
        struct ModeDecisionCandidateBuffer   *candidate_buffer_ptr,
        uint64_t                                luma_distortion,
        uint64_t                                lambda);

    typedef EbErrorType(*EB_INTRA_4x4_FULL_LUMA_COST_FUNC)(
        struct ModeDecisionCandidateBuffer   *candidate_buffer_ptr,
        uint64_t                               *y_distortion,
        uint64_t                                lambda,
        uint64_t                               *y_coeff_bits,
        uint32_t                                transform_size);

    typedef EbErrorType(*EB_FULL_NXN_COST_FUNC)(
        PictureControlSet_t                    *picture_control_set_ptr,
        struct ModeDecisionCandidateBuffer   *candidate_buffer_ptr,
        uint32_t                                qp,
        uint64_t                               *y_distortion,
        uint64_t                               *cb_distortion,
        uint64_t                               *cr_distortion,
        uint64_t                                lambda,
        uint64_t                                lambda_chroma,
        uint64_t                               *y_coeff_bits,
        uint64_t                               *cb_coeff_bits,
        uint64_t                               *cr_coeff_bits,
        uint32_t                                transform_size);

    struct CodingLoopContext_s;
#ifdef __cplusplus
}
#endif
#endif // EbModeDecision_h

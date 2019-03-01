/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#ifndef EbIntraPrediction_h
#define EbIntraPrediction_h

#include "EbIntraPrediction_SSE2.h"
#include "EbIntraPrediction_SSSE3.h"
#include "EbIntraPrediction_SSE4_1.h"
#include "EbIntraPrediction_AVX2.h"

#include "EbDefinitions.h"
#include "EbUtility.h"
#include "EbPictureBufferDesc.h"
#include "EbPictureControlSet.h"
#include "EbCodingUnit.h"
#include "EbPictureControlSet.h"
#include "EbModeDecision.h"
#include "EbNeighborArrays.h"
#include "EbMotionEstimationProcess.h"

#ifdef __cplusplus
extern "C" {
#endif
#define MAX_PU_SIZE                            64

    struct ModeDecisionContext;

    typedef void(*intra_pred_fn_c)(uint8_t *dst, ptrdiff_t stride, int32_t w, int32_t h,
        const uint8_t *above, const uint8_t *left);
    typedef void(*intra_highbd_pred_fn_c)(uint16_t *dst, ptrdiff_t stride, int32_t w, int32_t h,
        const uint16_t *above, const uint16_t *left, int32_t bd);

    typedef void(*intra_pred_fn)(uint8_t *dst, ptrdiff_t stride,
        const uint8_t *above, const uint8_t *left);

    typedef void(*intra_high_pred_fn)(uint16_t *dst, ptrdiff_t stride,
        const uint16_t *above, const uint16_t *left,
        int32_t bd);

    typedef struct IntraReferenceSamples 
    {
        uint8_t *y_intra_reference_array;
        uint8_t *cb_intra_reference_array;
        uint8_t *cr_intra_reference_array;
        uint8_t *y_intra_filtered_reference_array;
                
        uint8_t *y_intra_reference_array_reverse;
        uint8_t *y_intra_filtered_reference_array_reverse;
        uint8_t *cb_intra_reference_array_reverse;
        uint8_t *cr_intra_reference_array_reverse;

        // Scratch buffers used in the interpolaiton process
        uint8_t reference_above_line_y[(MAX_PU_SIZE << 2) + 1];
        uint8_t reference_left_line_y[(MAX_PU_SIZE << 2) + 1];
        EbBool  above_ready_flag_y;
        EbBool  left_ready_flag_y;
                
        uint8_t reference_above_line_cb[(MAX_PU_SIZE << 2) + 2];
        uint8_t reference_left_line_cb[(MAX_PU_SIZE << 2) + 2];
        EbBool  above_ready_flag_cb;
        EbBool  left_ready_flag_cb;
                
        uint8_t reference_above_line_cr[(MAX_PU_SIZE << 2) + 2];
        uint8_t reference_left_line_cr[(MAX_PU_SIZE << 2) + 2];
        EbBool  above_ready_flag_cr;
        EbBool  left_ready_flag_cr;

    } IntraReferenceSamples;

    typedef struct IntraReference16bitSamples 
    {

        uint16_t *y_intra_reference_array;
        uint16_t *cb_intra_reference_array;
        uint16_t *cr_intra_reference_array;
        uint16_t *y_intra_filtered_reference_array;
                 
        uint16_t *y_intra_reference_array_reverse;
        uint16_t *y_intra_filtered_reference_array_reverse;
        uint16_t *cb_intra_reference_array_reverse;
        uint16_t *cr_intra_reference_array_reverse;

        // Scratch buffers used in the interpolaiton process
        uint16_t  reference_above_line_y[(MAX_PU_SIZE << 2) + 1];
        uint16_t  reference_left_line_y[(MAX_PU_SIZE << 2) + 1];
        EbBool    above_ready_flag_y;
        EbBool    left_ready_flag_y;
                  
        uint16_t  reference_above_line_cb[(MAX_PU_SIZE << 2) + 2];
        uint16_t  reference_left_line_cb[(MAX_PU_SIZE << 2) + 2];
        EbBool    above_ready_flag_cb;
        EbBool    left_ready_flag_cb;
                  
        uint16_t  reference_above_line_cr[(MAX_PU_SIZE << 2) + 2];
        uint16_t  reference_left_line_cr[(MAX_PU_SIZE << 2) + 2];
        EbBool    above_ready_flag_cr;
        EbBool    left_ready_flag_cr;

    } IntraReference16bitSamples;

    extern EbErrorType intra_reference_samples_ctor(
        IntraReferenceSamples **context_dbl_ptr);

    extern EbErrorType intra_reference16bit_samples_ctor(
        IntraReference16bitSamples **context_dbl_ptr);

#define TOTAL_LUMA_MODES                   35
#define TOTAL_CHROMA_MODES                  5
#define TOTAL_INTRA_GROUPS                  5
#define INTRA_PLANAR_MODE                   0
#define INTRA_DC_MODE                       1
#define INTRA_HORIZONTAL_MODE              10
#define INTRA_VERTICAL_MODE                26
#define STRONG_INTRA_SMOOTHING_BLOCKSIZE   32
#define SMOOTHING_THRESHOLD                 8
#define SMOOTHING_THRESHOLD_10BIT          32

#if !QT_10BIT_SUPPORT

    extern EbErrorType generate_intra_reference_samples_encode_pass(

        EbBool                   *is_left_availble,
        EbBool                   *is_above_availble,

        EbBool                    constrained_intra_flag,   //input parameter, indicates if constrained intra is switched on/off
        EbBool                    strong_intra_smoothing_flag,
        uint32_t                  origin_x,
        uint32_t                  origin_y,
        uint32_t                  size,
        uint32_t                  cu_depth,
        NeighborArrayUnit        *mode_type_neighbor_array,
        NeighborArrayUnit        *luma_recon_neighbor_array,
        NeighborArrayUnit        *cb_recon_neighbor_array,
        NeighborArrayUnit        *cr_recon_neighbor_array,
        void                     *ref_wrapper_ptr,
        EbBool                    picture_left_boundary,
        EbBool                    picture_top_boundary,
        EbBool                    picture_right_boundary);
#endif




#if !QT_10BIT_SUPPORT

    extern EbErrorType generate_intra_reference16bit_samples_encode_pass(
        EbBool                         *is_left_availble,
        EbBool                         *is_above_availble,
        EbBool                     constrained_intra_flag,   //input parameter, indicates if constrained intra is switched on/off
        EbBool                     strong_intra_smoothing_flag,
        uint32_t                      origin_x,
        uint32_t                      origin_y,
        uint32_t                      size,
        uint32_t                      cu_depth,
        NeighborArrayUnit        *mode_type_neighbor_array,
        NeighborArrayUnit        *luma_recon_neighbor_array,
        NeighborArrayUnit        *cb_recon_neighbor_array,
        NeighborArrayUnit        *cr_recon_neighbor_array,
        void                       *ref_wrapper_ptr,
        EbBool                     picture_left_boundary,
        EbBool                     picture_top_boundary,
        EbBool                     picture_right_boundary);


    extern EbErrorType generate_luma_intra_reference16bit_samples_encode_pass(
        EbBool                     *is_left_availble,
        EbBool                     *is_above_availble,
        EbBool                     constrained_intra_flag,   //input parameter, indicates if constrained intra is switched on/off
        EbBool                     strong_intra_smoothing_flag,
        uint32_t                      origin_x,
        uint32_t                      origin_y,
        uint32_t                      size,
        uint32_t                      sb_sz,
        uint32_t                      cu_depth,
        NeighborArrayUnit        *mode_type_neighbor_array,
        NeighborArrayUnit        *luma_recon_neighbor_array,
        NeighborArrayUnit        *cb_recon_neighbor_array,
        NeighborArrayUnit        *cr_recon_neighbor_array,
        void                       *ref_wrapper_ptr,
        EbBool                     picture_left_boundary,
        EbBool                     picture_top_boundary,
        EbBool                     picture_right_boundary);


    extern EbErrorType generate_chroma_intra_reference16bit_samples_encode_pass(
        EbBool                     *is_left_availble,
        EbBool                     *is_above_availble,
        EbBool                     constrained_intra_flag,   //input parameter, indicates if constrained intra is switched on/off
        EbBool                     strong_intra_smoothing_flag,
        uint32_t                      origin_x,
        uint32_t                      origin_y,
        uint32_t                      size,
        uint32_t                      sb_sz,
        uint32_t                      cu_depth,
        NeighborArrayUnit        *mode_type_neighbor_array,
        NeighborArrayUnit        *luma_recon_neighbor_array,
        NeighborArrayUnit        *cb_recon_neighbor_array,
        NeighborArrayUnit        *cr_recon_neighbor_array,
        void                       *ref_wrapper_ptr,
        EbBool                     picture_left_boundary,
        EbBool                     picture_top_boundary,
        EbBool                     picture_right_boundary);


    extern EbErrorType intra_prediction_cl(
        struct ModeDecisionContext           *context_ptr,
        uint32_t                                  component_mask,
        PictureControlSet                    *picture_control_set_ptr,
        ModeDecisionCandidateBuffer           *candidate_buffer_ptr,
        EbAsm                                  asm_type);
#endif

    extern EbErrorType av1_intra_prediction_cl(
        struct ModeDecisionContext  *context_ptr,
 #if !CHROMA_BLIND
        uint32_t                     component_mask,
#endif
        PictureControlSet           *picture_control_set_ptr,
        ModeDecisionCandidateBuffer *candidate_buffer_ptr,
        EbAsm                        asm_type);

#if !QT_10BIT_SUPPORT
    extern EbErrorType encode_pass_intra_prediction(

        uint8_t                         upsample_left,
        uint8_t                         upsample_above,
        uint8_t                          upsample_left_chroma,
        uint8_t                          upsample_above_chroma,

        EbBool                         is_left_availble,
        EbBool                         is_above_availble,
        void                                   *ref_samples,
        uint32_t                                  origin_x,
        uint32_t                                  origin_y,
        uint32_t                                  pu_size,
        EbPictureBufferDesc                  *prediction_ptr,
        uint32_t                                  luma_mode,
        uint32_t                                  chroma_mode,
        int32_t                                  angle_delta,
        uint16_t                                  bitdepth,
        EbAsm                                  asm_type);
    extern EbErrorType encode_pass_intra_prediction16bit(

        uint8_t                         upsample_left,
        uint8_t                         upsample_above,
        uint8_t                          upsample_left_chroma,
        uint8_t                          upsample_above_chroma,

        EbBool                         is_left_availble,
        EbBool                         is_above_availble,
        void                                   *ref_samples,
        uint32_t                                  origin_x,
        uint32_t                                  origin_y,
        uint32_t                                  pu_size,
        EbPictureBufferDesc                  *prediction_ptr,
        uint32_t                                  luma_mode,
        uint32_t                                  chroma_mode,
        int32_t                                  angle_delta,
        uint16_t                                  bitdepth,
        EbAsm                                  asm_type);

    extern EbErrorType encode_pass_intra4x4_prediction(
        uint8_t                         upsample_left,
        uint8_t                         upsample_above,
        uint8_t                          upsample_left_chroma,
        uint8_t                          upsample_above_chroma,

        EbBool                         is_left_availble,
        EbBool                         is_above_availble,
        IntraReferenceSamples                *referenceSamples,
        uint32_t                                  origin_x,
        uint32_t                                  origin_y,
        uint32_t                                  pu_size,
        uint32_t                                  chromaPuSize,
        EbPictureBufferDesc                  *prediction_ptr,
        uint32_t                                  luma_mode,
        uint32_t                                  chroma_mode,
        uint32_t                                  component_mask,
        EbAsm                                  asm_type);

    extern EbErrorType encode_pass_intra4x4_prediction16bit(

        uint8_t                         upsample_left,
        uint8_t                         upsample_above,
        uint8_t                          upsample_left_chroma,
        uint8_t                          upsample_above_chroma,

        EbBool                         is_left_availble,
        EbBool                         is_above_availble,
        IntraReference16bitSamples           *referenceSamples,
        uint32_t                                  origin_x,
        uint32_t                                  origin_y,
        uint32_t                                  pu_size,
        uint32_t                                  chromaPuSize,
        EbPictureBufferDesc                  *prediction_ptr,
        uint32_t                                  luma_mode,
        uint32_t                                  chroma_mode,
        uint32_t                                  component_mask,
        uint16_t                                  bitdepth,
        EbAsm                                  asm_type);

#endif

    extern void intra_mode_angular_horizontal_kernel_ssse3_intrin(
        uint32_t      size,
        uint8_t      *ref_samp_main,
        uint8_t      *prediction_ptr,
        uint32_t      prediction_buffer_stride,
        const EbBool  skip,
        int32_t       intra_pred_angle);

    extern EbErrorType intra_open_loop_reference_samples_ctor(
        IntraReferenceSamplesOpenLoop **context_dbl_ptr);

    extern EbErrorType update_neighbor_samples_array_open_loop(
        IntraReferenceSamplesOpenLoop *intra_ref_ptr,
        EbPictureBufferDesc           *input_ptr,
        uint32_t                       stride,
        uint32_t                       src_origin_x,
        uint32_t                       src_origin_y,
        uint32_t                       block_size);

    extern EbErrorType intra_prediction_open_loop(
        uint32_t                 cu_size,
        MotionEstimationContext *context_ptr,
        uint32_t                 open_loop_intra_candidate,
        EbAsm                    asm_type);

#if !QT_10BIT_SUPPORT
    extern EbErrorType intra4x4_intra_prediction_cl(
        uint32_t                     pu_index,
        uint32_t                     pu_origin_x,
        uint32_t                     pu_origin_y,
        uint32_t                     pu_width,
        uint32_t                     pu_height,
        uint32_t                     sb_sz,
        uint32_t                     component_mask,
        PictureControlSet           *picture_control_set_ptr,
        ModeDecisionCandidateBuffer *candidate_buffer_ptr,
        EbPtr                        prediction_context_ptr,
        EbAsm                        asm_type);
#endif                              

    /***************************************
    * Function Ptr Types
    ***************************************/
    typedef void(*EbIntraNoangType)(
        const uint32_t  size,
        uint8_t        *ref_samples,
        uint8_t        *prediction_ptr,
        const uint32_t  prediction_buffer_stride,
        const EbBool    skip);

    typedef void(*EbIntraDcAv1Type)(
        EbBool         is_left_availble,
        EbBool         is_above_availble,
        const uint32_t   size,                       //input parameter, denotes the size of the current PU
        uint8_t         *ref_samples,                 //input parameter, pointer to the reference samples
        uint8_t         *dst,              //output parameter, pointer to the prediction
        const uint32_t   prediction_buffer_stride,     //input parameter, denotes the stride for the prediction ptr
        const EbBool   skip);                       //skip half rows

    typedef uint32_t(*EbNeighborDcIntraType)(
        MotionEstimationContext       *context_ptr,
        EbPictureBufferDesc           *input_ptr,
        uint32_t                           src_origin_x,
        uint32_t                           src_origin_y,
        uint32_t                           block_size,
        EbAsm                              asm_type);

    typedef void(*EbIntraNoAng16BitType)(
        const uint32_t   size,
        uint16_t         *ref_samples,
        uint16_t         *prediction_ptr,
        const uint32_t   prediction_buffer_stride,
        const EbBool  skip);


    typedef void(*EbIntraAngType)(
        uint32_t            size,
        uint8_t            *ref_samp_main,
        uint8_t            *prediction_ptr,
        uint32_t            prediction_buffer_stride,
        const EbBool     skip,
        int32_t            intra_pred_angle);

    extern void intra_mode_planar(
        const uint32_t   size,                       //input parameter, denotes the size of the current PU
        uint8_t         *ref_samples,                 //input parameter, pointer to the reference samples
        uint8_t         *dst,              //output parameter, pointer to the prediction
        const uint32_t   prediction_buffer_stride,     //input parameter, denotes the stride for the prediction ptr
        const EbBool  skip);
#if !QT_10BIT_SUPPORT
    extern void highbd_smooth_v_predictor(
        const uint32_t   size,                       //input parameter, denotes the size of the current PU
        uint16_t         *ref_samples,                 //input parameter, pointer to the reference samples
        uint16_t         *dst,              //output parameter, pointer to the prediction
        const uint32_t   prediction_buffer_stride,     //input parameter, denotes the stride for the prediction ptr
        const EbBool  skip);
#endif
    extern void ebav1_smooth_v_predictor(
        const uint32_t   size,                       //input parameter, denotes the size of the current PU
        uint8_t         *ref_samples,                 //input parameter, pointer to the reference samples
        uint8_t         *dst,              //output parameter, pointer to the prediction
        const uint32_t   prediction_buffer_stride,     //input parameter, denotes the stride for the prediction ptr
        const EbBool  skip);
#if !QT_10BIT_SUPPORT
    extern void highbd_smooth_h_predictor(
        const uint32_t   size,                       //input parameter, denotes the size of the current PU
        uint16_t         *ref_samples,                 //input parameter, pointer to the reference samples
        uint16_t         *dst,              //output parameter, pointer to the prediction
        const uint32_t   prediction_buffer_stride,     //input parameter, denotes the stride for the prediction ptr
        const EbBool  skip);
#endif
    extern void ebav1_smooth_h_predictor(
        const uint32_t   size,                       //input parameter, denotes the size of the current PU
        uint8_t         *ref_samples,                 //input parameter, pointer to the reference samples
        uint8_t         *dst,              //output parameter, pointer to the prediction
        const uint32_t   prediction_buffer_stride,     //input parameter, denotes the stride for the prediction ptr
        const EbBool  skip);
#if !QT_10BIT_SUPPORT
    extern void highbd_dc_predictor(
        EbBool                         is_left_availble,
        EbBool                         is_above_availble,
        const uint32_t   size,                       //input parameter, denotes the size of the current PU
        uint8_t         *ref_samples,                 //input parameter, pointer to the reference samples
        uint8_t         *dst,              //output parameter, pointer to the prediction
        const uint32_t   prediction_buffer_stride,     //input parameter, denotes the stride for the prediction ptr
        const EbBool  skip);                     //skip half rows
#endif

    void intra_mode_angular_av1_z1_16bit(
        const uint32_t   size,                    //input parameter, denotes the size of the current PU
        uint16_t         *ref_samples,             //input parameter, pointer to the reference samples
        uint16_t         *dst,                    //output parameter, pointer to the prediction
        const uint32_t   prediction_buffer_stride,  //input parameter, denotes the stride for the prediction ptr
        const EbBool  skip,
        uint16_t          dx,                     //output parameter, pointer to the prediction
        uint16_t          dy,                      //output parameter, pointer to the prediction
        uint16_t          bd);

    void intra_mode_angular_av1_z2_16bit(
        const uint32_t   size,                       //input parameter, denotes the size of the current PU
        uint16_t         *ref_samples,                 //input parameter, pointer to the reference samples
        uint16_t         *dst,              //output parameter, pointer to the prediction
        const uint32_t   prediction_buffer_stride,     //input parameter, denotes the stride for the prediction ptr
        const EbBool  skip,
        uint16_t          dx,              //output parameter, pointer to the prediction
        uint16_t          dy,              //output parameter, pointer to the prediction
        uint16_t          bd);

    void intra_mode_angular_av1_z3_16bit(
        const uint32_t   size,                       //input parameter, denotes the size of the current PU
        uint16_t         *ref_samples,                 //input parameter, pointer to the reference samples
        uint16_t         *dst,              //output parameter, pointer to the prediction
        const uint32_t   prediction_buffer_stride,     //input parameter, denotes the stride for the prediction ptr
        const EbBool  skip,
        uint16_t          dx,              //output parameter, pointer to the prediction
        uint16_t          dy,              //output parameter, pointer to the prediction
        uint16_t          bd);


    /***************************************
    * Function Ptrs
    ***************************************/
    static EbIntraNoangType FUNC_TABLE intra_vertical_luma_func_ptr_array[ASM_TYPE_TOTAL] = {
        // NON_AVX2
        intra_mode_vertical_luma_sse2_intrin,
        // AVX2
        intra_mode_vertical_luma_avx2_intrin,

    };


    static EbIntraNoangType FUNC_TABLE intra_vertical_chroma_func_ptr_array[ASM_TYPE_TOTAL] = {
        // NON_AVX2
        intra_mode_vertical_chroma_sse2_intrin,
        // AVX2
        intra_mode_vertical_chroma_sse2_intrin,
    };


    static EbIntraNoangType FUNC_TABLE intra_horz_luma_func_ptr_array[ASM_TYPE_TOTAL] = {
        // NON_AVX2
        intra_mode_horizontal_luma_sse2_intrin,
        // AVX2
        intra_mode_horizontal_luma_sse2_intrin,
    };


    static EbIntraNoangType FUNC_TABLE intra_horz_chroma_func_ptr_array[ASM_TYPE_TOTAL] = {
        // NON_AVX2
        intra_mode_horizontal_chroma_sse2_intrin,
        // AVX2
        intra_mode_horizontal_chroma_sse2_intrin,
    };

#if !QT_10BIT_SUPPORT
    static EbIntraDcAv1Type FUNC_TABLE intra_dc_av1_func_ptr_array[9][ASM_TYPE_TOTAL] = {

        // 4x4
        {
            // NON_AVX2
            highbd_dc_predictor,
            // AVX2
            intra_mode_dc_4x4_av1_sse2_intrin,
        },
        // 8x8
        {
            // NON_AVX2
            highbd_dc_predictor,
            // AVX2
            intra_mode_dc_8x8_av1_sse2_intrin,
        },
        // 16x16
        {
            // NON_AVX2
            highbd_dc_predictor,
            // AVX2
            intra_mode_dc_16x16_av1_sse2_intrin,

        },
        // NxN
        {
            // NON_AVX2
            highbd_dc_predictor,
            // AVX2
            highbd_dc_predictor,

        },
        // 32x32
        {
            // NON_AVX2
            highbd_dc_predictor,
            // AVX2
            intra_mode_dc_32x32_av1_avx2_intrin,

        } ,
        // NxN
        {
            // NON_AVX2
            highbd_dc_predictor,
            // AVX2
            highbd_dc_predictor,

        },
        // NxN
        {
            // NON_AVX2
            highbd_dc_predictor,
            // AVX2
            highbd_dc_predictor,

        },
        // NxN
        {
            // NON_AVX2
            highbd_dc_predictor,
            // AVX2
            highbd_dc_predictor,

        },
        // 64x64
        {
            // NON_AVX2
            highbd_dc_predictor,
            // AVX2

            intra_mode_dc_64x64_av1_avx2_intrin,

        }

    };
#endif
    static EbIntraNoangType FUNC_TABLE intra_dc_luma_func_ptr_array[ASM_TYPE_TOTAL] = {
        // NON_AVX2
        intra_mode_dc_luma_sse2_intrin,
        // AVX2
        intra_mode_dc_luma_avx2_intrin,

    };

    uint32_t update_neighbor_dc_intra_pred(
        MotionEstimationContext       *context_ptr,
        EbPictureBufferDesc           *input_ptr,
        uint32_t                           src_origin_x,
        uint32_t                           src_origin_y,
        uint32_t                           block_size,
        EbAsm                             asm_type);

    static EbIntraNoangType FUNC_TABLE intra_dc_chroma_func_ptr_array[ASM_TYPE_TOTAL] = {
        // NON_AVX2
        intra_mode_dc_chroma_sse2_intrin,
        // AVX2
        intra_mode_dc_chroma_sse2_intrin,
    };


    static EbIntraNoangType FUNC_TABLE intra_planar_func_ptr_array[ASM_TYPE_TOTAL] = {
        // NON_AVX2
        intra_mode_planar_sse2_intrin,
        // AVX2
        intra_mode_planar_avx2_intrin,
    };

    void smooth_v_predictor_c(
        uint8_t *dst, 
        ptrdiff_t stride, 
        int32_t bw,
        int32_t bh, 
        const uint8_t *above,
        const uint8_t *left);

    static EbIntraNoangType FUNC_TABLE intra_planar_av1_func_ptr_array[ASM_TYPE_TOTAL] = {
        // NON_AVX2
        intra_mode_planar,
        // AVX2
        intra_mode_planar_av1_avx2_intrin,
    };
#if !QT_10BIT_SUPPORT
    static EbIntraNoAng16BitType FUNC_TABLE intra_smooth_v_16bit_av1_func_ptr_array[ASM_TYPE_TOTAL] = {
        // NON_AVX2
        highbd_smooth_v_predictor,
        // AVX2
        highbd_smooth_v_predictor,
    };
#endif
    static EbIntraNoangType FUNC_TABLE intra_smooth_h_av1_func_ptr_array[ASM_TYPE_TOTAL] = {
        // NON_AVX2
        ebav1_smooth_h_predictor,
        // AVX2
        ebav1_smooth_h_predictor,
    };
#if !QT_10BIT_SUPPORT
    static EbIntraNoAng16BitType FUNC_TABLE intra_smooth_h_16bit_av1_func_ptr_array[ASM_TYPE_TOTAL] = {
        // NON_AVX2
        highbd_smooth_h_predictor,
        // AVX2
        highbd_smooth_h_predictor,
    };
#endif
    static EbIntraNoangType FUNC_TABLE intra_smooth_v_av1_func_ptr_array[ASM_TYPE_TOTAL] = {
        // NON_AVX2
        ebav1_smooth_v_predictor,
        // AVX2
        ebav1_smooth_v_predictor,
    };

    static EbIntraNoangType FUNC_TABLE intra_ang34_func_ptr_array[ASM_TYPE_TOTAL] = {
        // NON_AVX2
        intra_mode_angular_34_sse2_intrin,
        // AVX2
        intra_mode_angular_34_avx2_intrin,
    };


    static EbIntraNoangType FUNC_TABLE intra_ang18_func_ptr_array[ASM_TYPE_TOTAL] = {
        // NON_AVX2
        intra_mode_angular_18_sse2_intrin,
        // AVX2
        intra_mode_angular_18_avx2_intrin,

    };


    static EbIntraNoangType FUNC_TABLE intra_ang2_func_ptr_array[ASM_TYPE_TOTAL] = {
        // NON_AVX2
        intra_mode_angular_2_sse2_intrin,
        // AVX2
        intra_mode_angular_2_avx2_intrin,
    };


    static EbIntraAngType FUNC_TABLE intra_ang_vertical_func_ptr_array[ASM_TYPE_TOTAL] = {
        // NON_AVX2
        intra_mode_angular_vertical_kernel_ssse3_intrin,
        // AVX2
        intra_mode_angular_vertical_kernel_avx2_intrin,
    };


    static EbIntraAngType FUNC_TABLE intra_ang_horizontal_func_ptr_array[ASM_TYPE_TOTAL] = {
        // NON_AVX2
        intra_mode_angular_horizontal_kernel_ssse3_intrin,
        // AVX2
        intra_mode_angular_horizontal_kernel_avx2_intrin,
    };

    extern void cfl_luma_subsampling_420_lbd_c(
        uint8_t *input, // AMIR-> Changed to 8 bit
        int32_t input_stride, int16_t *output_q3,
        int32_t width, int32_t height);

    extern void cfl_luma_subsampling_420_hbd_c(
        const uint16_t *input,
        int32_t input_stride, int16_t *output_q3,
        int32_t width, int32_t height);

    extern void subtract_average_c(
        int16_t *pred_buf_q3,
        int32_t width,
        int32_t height,
        int32_t round_offset,
        int32_t num_pel_log2);


#define ROUND_POWER_OF_TWO_SIGNED(value, n)           \
  (((value) < 0) ? -ROUND_POWER_OF_TWO(-(value), (n)) \
                 : ROUND_POWER_OF_TWO((value), (n)))

    static INLINE int32_t get_scaled_luma_q0(int32_t alpha_q3, int16_t pred_buf_q3) {
        int32_t scaled_luma_q6 = alpha_q3 * pred_buf_q3;
        return ROUND_POWER_OF_TWO_SIGNED(scaled_luma_q6, 6);
    }

    //CFL_PREDICT_FN(c, lbd)



    void cfl_predict_lbd_c(
        const int16_t *pred_buf_q3,
        uint8_t *pred,// AMIR ADDED
        int32_t pred_stride,
        uint8_t *dst,// AMIR changed to 8 bit
        int32_t dst_stride,
        int32_t alpha_q3,
        int32_t bit_depth,
        int32_t width,
        int32_t height);

    void cfl_predict_hbd_c(
        const int16_t *pred_buf_q3,
        uint16_t *pred,// AMIR ADDED
        int32_t pred_stride,
        uint16_t *dst,// AMIR changed to 8 bit
        int32_t dst_stride,
        int32_t alpha_q3,
        int32_t bit_depth,
        int32_t width,
        int32_t height);


    static INLINE int32_t cfl_idx_to_alpha(int32_t alpha_idx, int32_t joint_sign,
        CflPredType pred_type) {
        const int32_t alpha_sign = (pred_type == CFL_PRED_U) ? CFL_SIGN_U(joint_sign)
            : CFL_SIGN_V(joint_sign);
        if (alpha_sign == CFL_SIGN_ZERO) return 0;
        const int32_t abs_alpha_q3 =
            (pred_type == CFL_PRED_U) ? CFL_IDX_U(alpha_idx) : CFL_IDX_V(alpha_idx);
        return (alpha_sign == CFL_SIGN_POS) ? abs_alpha_q3 + 1 : -abs_alpha_q3 - 1;
    }





#ifdef __cplusplus
}
#endif
#endif // EbIntraPrediction_h

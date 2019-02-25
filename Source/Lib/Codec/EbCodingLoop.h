/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#ifndef EbCodingLoop_h
#define EbCodingLoop_h

#include "EbDefinitions.h"
#include "EbCodingUnit.h"
#include "EbSequenceControlSet.h"
#include "EbModeDecisionProcess.h"
#include "EbEncDecProcess.h"
#ifdef __cplusplus
extern "C" {
#endif
    /*******************************************
     * ModeDecisionLCU
     *   performs CL (LCU)
     *******************************************/

    typedef EbErrorType(*EB_MODE_DECISION)(
        SequenceControlSet_t                *sequence_control_set_ptr,
        PictureControlSet_t                 *picture_control_set_ptr,
        const MdcLcuData_t * const           mdc_result_tb_ptr,
        LargestCodingUnit                 *sb_ptr,
        uint32_t                             sb_origin_x,
        uint32_t                             sb_origin_y,
        uint32_t                             lcu_addr,
        ModeDecisionContext               *context_ptr);


    extern EbErrorType in_loop_motion_estimation_sblock(
        PictureControlSet_t                 *picture_control_set_ptr,  // input parameter, Picture Control Set Ptr
        uint32_t                             sb_origin_x,            // input parameter, SB Origin X
        uint32_t                             sb_origin_y,            // input parameter, SB Origin X
        int16_t                              x_mv_l0,
        int16_t                              y_mv_l0,
        int16_t                              x_mv_l1,
        int16_t                              y_mv_l1,
        SsMeContext                        *context_ptr);          // input parameter, ME Context Ptr, used to store decimated/interpolated LCU/SR

    extern EbErrorType mode_decision_sb(
        SequenceControlSet_t                *sequence_control_set_ptr,
        PictureControlSet_t                 *picture_control_set_ptr,
        const MdcLcuData_t * const           mdc_result_tb_ptr,
        LargestCodingUnit                 *sb_ptr,
        uint16_t                             sb_origin_x,
        uint16_t                             sb_origin_y,
        uint32_t                             lcu_addr,
        SsMeContext                       *ss_mecontext,
        ModeDecisionContext               *context_ptr);

    extern EbErrorType qpm_derive_weights_min_and_max(
        PictureControlSet_t                    *picture_control_set_ptr,
        EncDecContext                        *context_ptr);

#if TX_SEARCH_LEVELS
    uint8_t get_skip_tx_search_flag(
        int32_t                  sq_size,
        uint64_t                 ref_fast_cost,
        uint64_t                 cu_cost,
        uint64_t                 weight);
#endif

    extern void av1_encode_pass(
        SequenceControlSet_t    *sequence_control_set_ptr,
        PictureControlSet_t     *picture_control_set_ptr,
        LargestCodingUnit     *sb_ptr,
        uint32_t                   tb_addr,
        uint32_t                   sb_origin_x,
        uint32_t                   sb_origin_y,
        uint32_t                   sb_qp,
        EncDecContext         *context_ptr);


#if NO_ENCDEC

    void no_enc_dec_pass(
        SequenceControlSet_t    *sequence_control_set_ptr,
        PictureControlSet_t     *picture_control_set_ptr,
        LargestCodingUnit     *sb_ptr,
        uint32_t                   tb_addr,
        uint32_t                   sb_origin_x,
        uint32_t                   sb_origin_y,
        uint32_t                   sb_qp,
        EncDecContext         *context_ptr);
#endif

#ifdef __cplusplus
}
#endif
#endif // EbCodingLoop_h

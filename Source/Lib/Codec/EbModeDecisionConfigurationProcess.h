/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#ifndef EbModeDecisionConfigurationProcess_h
#define EbModeDecisionConfigurationProcess_h

#include "EbSystemResourceManager.h"
#include "EbMdRateEstimation.h"
#include "EbDefinitions.h"
#include "EbRateControlProcess.h"
#include "EbSequenceControlSet.h"
#ifdef __cplusplus
extern "C" {
#endif

    /**************************************
     * Defines
     **************************************/
    static const uint8_t depth_offset[4] = { 85,21,5,1 };
    static const uint32_t ns_blk_offset[10] = { 0, 1, 3, 25, 5, 8, 11, 14 ,17, 21 };
    static const uint32_t ns_blk_num[10] = { 1, 2, 2, 4, 3, 3, 3, 3, 4, 4 };

    typedef struct MdcpLocalCodingUnit
    {
        uint64_t                          earlyCost;
        EbBool                         earlySplitFlag;
        uint32_t                          splitContext;
        EbBool                         slectedCu;
        EbBool                         stopSplit;
    } MdcpLocalCodingUnit;

    typedef struct ModeDecisionConfigurationContext
    {
        EbFifo                            *rate_control_input_fifo_ptr;
        EbFifo                            *mode_decision_configuration_output_fifo_ptr;

        MdRateEstimationContext           *md_rate_estimation_ptr;

        uint8_t                               qp;
        uint64_t                              lambda;
        MdcpLocalCodingUnit               local_cu_array[CU_MAX_COUNT];

        // Inter depth decision
        uint8_t                               group_of8x8_blocks_count;
        uint8_t                               group_of16x16_blocks_count;
        uint64_t                              interComplexityMinimum;
        uint64_t                              interComplexityMaximum;
        uint64_t                              interComplexityAverage;
        uint64_t                              intraComplexityMinimum;
        uint64_t                              intraComplexityMaximum;
        uint64_t                              intraComplexityAverage;
        int16_t                              min_delta_qp_weight;
        int16_t                              max_delta_qp_weight;
        int8_t                               min_delta_qp[4];
        int8_t                               max_delta_qp[4];



        // Budgeting
        uint32_t                             *lcuScoreArray;

        uint8_t                                costDepthMode[LCU_PRED_OPEN_LOOP_1_NFL_DEPTH_MODE];

        uint8_t                              *lcuCostArray;
        uint32_t                              predictedCost;
        uint32_t                              budget;

        int8_t                               scoreTh[MAX_SUPPORTED_SEGMENTS];
        uint8_t                               intervalCost[MAX_SUPPORTED_SEGMENTS];
        uint8_t                               numberOfSegments;

        uint32_t                              lcuMinScore;
        uint32_t                              lcuMaxScore;
        EbBool                             depthSensitivePictureFlag;
        EbBool                             performRefinement;


        uint8_t                               qp_index;



    } ModeDecisionConfigurationContext;



    /**************************************
     * Extern Function Declarations
     **************************************/
    extern EbErrorType mode_decision_configuration_context_ctor(
        ModeDecisionConfigurationContext **context_dbl_ptr,
        EbFifo                            *rate_control_input_fifo_ptr,
        EbFifo                            *mode_decision_configuration_output_fifo_ptr,
        uint16_t                                 sb_total_count);


    extern void* mode_decision_configuration_kernel(void *input_ptr);
#ifdef __cplusplus
}
#endif
#endif // EbModeDecisionConfigurationProcess_h
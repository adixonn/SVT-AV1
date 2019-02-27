/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#ifndef EbRateControl_h
#define EbRateControl_h

#include "EbDefinitions.h"
#include "EbSystemResourceManager.h"
#include "EbApi.h"
#include "EbPictureControlSet.h"

#define CCOEFF_INIT_FACT              2
#define SAD_CLIP_COEFF                5
// 88 + 3*16*8
#define SLICE_HEADER_BITS_NUM       104
#define PPS_BITS_NUM                 80
#define SPS_BITS_NUM                296
#define RC_PRECISION                16
#define RC_PRECISION_OFFSET         (1 << (RC_PRECISION - 1))

#define OVERSHOOT_STAT_PRINT             0 // Do not remove.
                                           // For printing overshooting percentages for both RC and fixed QP.
                                           // Target rate and and max buffer size should be set properly even for fixed QP.
                                           // Disabled by default.
#if OVERSHOOT_STAT_PRINT
#define CODED_FRAMES_STAT_QUEUE_MAX_DEPTH   10000
#endif

#define ADAPTIVE_PERCENTAGE 1
#define     RC_INTRA_QP_OFFSET              (-1)

#define     RC_DISABLE_FLRC                 0
#define     RC_DISABLE_FLRC_RATE_UPDATE     0
#define     RC_DISABLE_HLRC_RATE_INPUT      1
#define     RC_DISABLE_PRED_QP_LIMIT        1
#define     RC_DISABLE_EXTRA_BITS           0
#define     RC_NEW_EXTRA_BITS               1
#define     RC_UPDATE_TARGET_RATE           1

#define        RC_QPMOD_MAXQP                    42

static const uint32_t  rate_percentage_layer_array[EB_MAX_TEMPORAL_LAYERS][EB_MAX_TEMPORAL_LAYERS] = {
    {100,  0,  0,  0,  0,  0 },
    { 70, 30,  0,  0,  0,  0 },
    { 70, 15, 15,  0,  0,  0 },
    { 55, 15, 15, 15,  0,  0 },
    { 40, 15, 15, 15, 15,  0 },
    { 30, 10, 15, 15, 15, 15 }
};

// range from 0 to 51
// precision is 16 bits
static const uint64_t two_to_power_qp_over_three[] = {
         0x10000,      0x1428A,     0x19660,     0x20000,
         0x28514,      0x32CC0,     0x40000,     0x50A29,
         0x65980,      0x80000,     0xA1451,     0xCB2FF,
        0x100000,     0x1428A3,    0x1965FF,    0x200000,
        0x285146,     0x32CBFD,    0x400000,    0x50A28C,
        0x6597FB,     0x800000,    0xA14518,    0xCB2FF5,
       0x1000000,    0x1428A30,   0x1965FEA,   0x2000000,
       0x285145F,    0x32CBFD5,   0x4000000,   0x50A28BE,
       0x6597FA9,    0x8000000,   0xA14517D,   0xCB2FF53,
      0x10000000,   0x1428A2FA,  0x1965FEA5,  0x20000000,
      0x285145F3,   0x32CBFD4A,  0x40000000,  0x50A28BE6,
      0x6597FA95,   0x80000000,  0xA14517CC,  0xCB2FF52A,
     0x100000000,  0x1428A2F99, 0x1965FEA54, 0x200000000
};
// range is from -51 to 51 (0 to 102)
static const uint64_t two_to_power_x_over_six[] = {
         0xB5,       0xCB,        0xE4,        0xFF,        0x11F,        0x142,
        0x16A,      0x196,       0x1C8,       0x1FF,        0x23E,        0x285,
        0x2D4,      0x32C,       0x390,       0x3FF,        0x47D,        0x50A,
        0x5A8,      0x659,       0x720,       0x7FF,        0x8FA,        0xA14,
        0xB50,      0xCB2,       0xE41,       0xFFF,       0x11F5,       0x1428,
       0x16A0,     0x1965,      0x1C82,      0x1FFF,       0x23EB,       0x2851,
       0x2D41,     0x32CB,      0x3904,      0x3FFF,       0x47D6,       0x50A2,
       0x5A82,     0x6597,      0x7208,      0x7FFF,       0x8FAC,       0xA144,
       0xB504,     0xCB2F,      0xE411,      0xFFFF,      0x11F58,      0x14288,
      0x16A08,    0x1965E,     0x1C822,     0x1FFFE,      0x23EB1,    0x28511,
      0x2D410,    0x32CBC,     0x39044,     0x3FFFC,      0x47D62,    0x50A23,
      0x5A821,    0x65979,     0x72088,     0x7FFF8,      0x8FAC4,    0xA1447,
      0xB5043,    0xCB2F2,     0xE4110,     0xFFFF0,     0x11F588,   0x14288E,
     0x16A087,   0x1965E5,    0x1C8221,    0x1FFFE0,     0x23EB11,   0x28511D,
     0x2D410F,   0x32CBCA,    0x390443,    0x3FFFC0,     0x47D623,   0x50A23B,
     0x5A821F,   0x659794,    0x720886,    0x7FFF80,     0x8FAC46,   0xA14476,
     0xB5043E,   0xCB2F29,    0xE4110C,    0xFFFF00,    0x11F588C,  0x14288ED,
     0x16A087C
};
/**************************************
 * Input Port Types
 **************************************/
typedef enum RateControlInputPortTypes 
{
    RATE_CONTROL_INPUT_PORT_PICTURE_MANAGER = 0,
    RATE_CONTROL_INPUT_PORT_PACKETIZATION = 1,
    RATE_CONTROL_INPUT_PORT_ENTROPY_CODING = 2,
    RATE_CONTROL_INPUT_PORT_TOTAL_COUNT = 3,
    RATE_CONTROL_INPUT_PORT_INVALID = ~0,
} RateControlInputPortTypes;

/**************************************
 * Input Port Config
 **************************************/
typedef struct RateControlPorts 
{
    RateControlInputPortTypes type;
    uint32_t                  count;
} RateControlPorts;

/**************************************
 * Coded Frames Stats
 **************************************/
typedef struct CodedFramesStatsEntry 
{
    uint64_t picture_number;
    int64_t  frame_total_bit_actual;
    EbBool   end_of_sequence_flag;
} CodedFramesStatsEntry;
/**************************************
 * Context
 **************************************/
typedef struct RateControlLayerContext
{
    uint64_t                  previous_frame_sad_me;
    uint64_t                  previous_frame_bit_actual;
    uint64_t                  previous_frame_quantized_coeff_bit_actual;
    EbBool                    feedback_arrived;

    uint64_t                  target_bit_rate;
    uint64_t                  frame_rate;
    uint64_t                  channel_bit_rate;

    uint64_t                  previous_bit_constraint;
    uint64_t                  bit_constraint;
    uint64_t                  ec_bit_constraint;
    int64_t                   dif_total_and_ec_bits;

    uint32_t                  coeff_averaging_weight1;
    uint32_t                  coeff_averaging_weight2; // coeff_averaging_weight2 = 16- coeff_averaging_weight1
    //c_coeffs have 2*RC_PRECISION precision
    int64_t                  c_coeff;
    int64_t                  previous_c_coeff;
    //k_coeffs have RC_PRECISION precision
    uint64_t                  k_coeff;
    uint64_t                  previous_k_coeff;

    //delta_qp_fraction has RC_PRECISION precision
    int64_t                   delta_qp_fraction;
    uint32_t                  previous_frame_qp;
    uint32_t                  calculated_frame_qp;
    uint32_t                  previous_calculated_frame_qp;
    uint32_t                  area_in_pixel;
    uint32_t                  previous_frame_average_qp;

    //total_mad has RC_PRECISION precision
    uint64_t                  total_mad;

    uint32_t                  first_frame;
    uint32_t                  first_non_intra_frame;
    uint32_t                  same_sad_count;
    uint32_t                  frame_same_sad_min_qp_count;
    uint32_t                  critical_states;

    uint32_t                  max_qp;
    uint32_t                  temporal_index;

    uint64_t                  alpha;

} RateControlLayerContext;

typedef struct RateControlIntervalParamContext
{
    uint64_t                  first_poc;
    uint64_t                  last_poc;
    EbBool                    in_use;
    EbBool                    was_used;
    uint64_t                  processed_frames_number;
    EbBool                    last_gop;
    RateControlLayerContext **rate_control_layer_array;

    int64_t                   virtual_buffer_level;
    int64_t                   previous_virtual_buffer_level;
    uint32_t                  intra_frames_qp;

    uint32_t                  next_gop_intra_frame_qp;
    uint64_t                  first_pic_pred_bits;
    uint64_t                  first_pic_actual_bits;
    uint16_t                  first_pic_pred_qp;
    uint16_t                  first_pic_actual_qp;
    EbBool                    first_pic_actual_qp_assigned;
    EbBool                    scene_change_in_gop;
    EbBool                    min_target_rate_assigned;
    int64_t                   extra_ap_bit_ratio_i;

} RateControlIntervalParamContext;

typedef struct HighLevelRateControlContext
{

    uint64_t                  target_bit_rate;
    uint64_t                  frame_rate;
    uint64_t                  channel_bit_rate_per_frame;
    uint64_t                  channel_bit_rate_per_sw;
    uint64_t                  bit_constraint_per_sw;
    uint64_t                  pred_bits_ref_qp_per_sw[MAX_REF_QP_NUM];
#if RC_UPDATE_TARGET_RATE     
    uint32_t                  prev_intra_selected_ref_qp;
    uint32_t                  prev_intra_org_selected_ref_qp;
    uint64_t                  previous_updated_bit_constraint_per_sw;
#endif


} HighLevelRateControlContext;

typedef struct RateControlContext
{
    EbFifo                           *rate_control_input_tasks_fifo_ptr;
    EbFifo                           *rate_control_output_results_fifo_ptr;

    HighLevelRateControlContext      *high_level_rate_control_ptr;

    RateControlIntervalParamContext **rateControlParamQueue;
    uint64_t                          rateControlParamQueueHeadIndex;

    uint64_t                          frame_rate;
                                      
    uint64_t                          virtual_buffer_size;
                                      
    int64_t                           virtual_buffer_level_initial_value;
    int64_t                           previous_virtual_buffer_level;
                                      
    int64_t                           virtual_buffer_level;

    //Virtual Buffer Thresholds
    int64_t                           vb_fill_threshold1;
    int64_t                           vb_fill_threshold2;

    // Rate Control Previous Bits Queue
#if OVERSHOOT_STAT_PRINT
    CodedFramesStatsEntry        **coded_frames_stat_queue;
    uint32_t                       coded_frames_stat_queue_head_index;
    uint32_t                       coded_frames_stat_queue_tail_index;

    uint64_t                       total_bit_actual_per_sw;
    uint64_t                       max_bit_actual_per_sw;
    uint64_t                       max_bit_actual_per_gop;

#endif


    uint64_t                       rate_average_periodin_frames;
    uint32_t                       base_layer_frames_avg_qp;
    uint32_t                       base_layer_intra_frames_avg_qp;

    EbBool                         end_of_sequence_region;

    uint32_t                       intra_coef_rate;

    uint64_t                       frames_in_interval[EB_MAX_TEMPORAL_LAYERS];
    int64_t                        extra_bits;
    int64_t                        extra_bits_gen;
    int16_t                        max_rate_adjust_delta_qp;


} RateControlContext;

/**************************************
 * Extern Function Declarations
 **************************************/
extern EbErrorType rate_control_layer_context_ctor(
    RateControlLayerContext **entry_dbl_ptr);

extern EbErrorType rate_control_interval_param_context_ctor(
    RateControlIntervalParamContext **entry_dbl_ptr);

extern EbErrorType rate_control_coded_frames_stats_context_ctor(
    CodedFramesStatsEntry **entry_dbl_ptr,
    uint64_t                picture_number);

extern EbErrorType rate_control_context_ctor(
    RateControlContext **context_dbl_ptr,
    EbFifo              *rate_control_input_tasks_fifo_ptr,
    EbFifo              *rate_control_output_results_fifo_ptr,
    int32_t              intra_period_length);

extern void* rate_control_kernel(void *input_ptr);

#endif // EbRateControl_h
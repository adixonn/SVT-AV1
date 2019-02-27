/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#ifndef EbEncDecSegments_h
#define EbEncDecSegments_h

#include "EbDefinitions.h"
#include "EbThreads.h"
#ifdef __cplusplus
extern "C" {
#endif
    /**************************************
     * Defines
     **************************************/
#define ENCDEC_SEGMENTS_MAX_COL_COUNT  60
#define ENCDEC_SEGMENTS_MAX_ROW_COUNT  37
#define ENCDEC_SEGMENTS_MAX_BAND_COUNT ENCDEC_SEGMENTS_MAX_COL_COUNT + ENCDEC_SEGMENTS_MAX_ROW_COUNT
#define ENCDEC_SEGMENTS_MAX_COUNT      ENCDEC_SEGMENTS_MAX_BAND_COUNT * ENCDEC_SEGMENTS_MAX_ROW_COUNT
#define ENCDEC_SEGMENT_INVALID         0xFFFF

     /**************************************
      * Macros
      **************************************/
#define BAND_TOTAL_COUNT(lcu_row_total_count, lcu_col_total_count) \
    ((lcu_row_total_count) + (lcu_col_total_count) - 1)
#define ROW_INDEX(y_lcu_index, segment_row_count, lcu_row_total_count) \
    (((y_lcu_index) * (segment_row_count)) / (lcu_row_total_count))
#define BAND_INDEX(x_lcu_index, y_lcu_index, segment_band_count, lcu_band_total_count) \
    ((((x_lcu_index) + (y_lcu_index)) * (segment_band_count)) / (lcu_band_total_count))
#define SEGMENT_INDEX(row_index, bandIndex, segment_band_count) \
    (((row_index) * (segment_band_count)) + (bandIndex))

      /**************************************
       * Member definitions
       **************************************/
    typedef struct EncDecSegDependencyMap 
    {
        uint8_t      *dependencyMap;
        EbHandle   updateMutex;
    } EncDecSegDependencyMap;

    typedef struct EncDecSegSegmentRow 
    {
        uint16_t      startingSegIndex;
        uint16_t      endingSegIndex;
        uint16_t      currentSegIndex;
        EbHandle   assignmentMutex;
    } EncDecSegSegmentRow;

    /**************************************
     * ENCDEC Segments
     **************************************/
    typedef struct EncDecSegments
    {
        EncDecSegDependencyMap  depMap;
        EncDecSegSegmentRow    *rowArray;

        uint16_t                   *xStartArray;
        uint16_t                   *yStartArray;
        uint16_t                   *validLcuCountArray;

        uint32_t                    segment_band_count;
        uint32_t                    segment_row_count;
        uint32_t                    segmentTotalCount;
        uint32_t                    lcuBandCount;
        uint32_t                    lcuRowCount;

        uint32_t                    segmentMaxBandCount;
        uint32_t                    segmentMaxRowCount;
        uint32_t                    segmentMaxTotalCount;

    } EncDecSegments;

    /**************************************
     * Extern Function Declarations
     **************************************/
    extern EbErrorType enc_dec_segments_ctor(
        EncDecSegments **segments_dbl_ptr,
        uint32_t             segment_col_count,
        uint32_t             segment_row_count);


    extern void enc_dec_segments_init(
        EncDecSegments *segments_ptr,
        uint32_t            col_count,
        uint32_t            row_count,
        uint32_t            pic_width_lcu,
        uint32_t            pic_height_lcu);
#ifdef __cplusplus
}
#endif
#endif // EbEncDecResults_h
/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#include <stdlib.h>
#include <string.h>

#include "EbEncDecSegments.h"
#include "EbThreads.h"

EbErrorType enc_dec_segments_ctor(
    EncDecSegments_t **segments_dbl_ptr,
    uint32_t             segment_col_count,
    uint32_t             segment_row_count)
{
    uint32_t row_index;
    EncDecSegments_t *segments_ptr;
    EB_MALLOC(EncDecSegments_t*, segments_ptr, sizeof(EncDecSegments_t), EB_N_PTR);

    *segments_dbl_ptr = segments_ptr;

    segments_ptr->segmentMaxRowCount = segment_row_count;
    segments_ptr->segmentMaxBandCount = segment_row_count + segment_col_count;
    segments_ptr->segmentMaxTotalCount = segments_ptr->segmentMaxRowCount * segments_ptr->segmentMaxBandCount;

    // Start Arrays
    EB_MALLOC(uint16_t*, segments_ptr->xStartArray, sizeof(uint16_t) * segments_ptr->segmentMaxTotalCount, EB_N_PTR);

    EB_MALLOC(uint16_t*, segments_ptr->yStartArray, sizeof(uint16_t) * segments_ptr->segmentMaxTotalCount, EB_N_PTR);

    EB_MALLOC(uint16_t*, segments_ptr->validLcuCountArray, sizeof(uint16_t) * segments_ptr->segmentMaxTotalCount, EB_N_PTR);

    // Dependency map
    EB_MALLOC(uint8_t*, segments_ptr->depMap.dependencyMap, sizeof(uint8_t) * segments_ptr->segmentMaxTotalCount, EB_N_PTR);

    EB_CREATEMUTEX(EbHandle, segments_ptr->depMap.updateMutex, sizeof(EbHandle), EB_MUTEX);

    // Segment rows
    EB_MALLOC(EncDecSegSegmentRow_t*, segments_ptr->rowArray, sizeof(EncDecSegSegmentRow_t) * segments_ptr->segmentMaxRowCount, EB_N_PTR)

        for (row_index = 0; row_index < segments_ptr->segmentMaxRowCount; ++row_index) {
            EB_CREATEMUTEX(EbHandle, segments_ptr->rowArray[row_index].assignmentMutex, sizeof(EbHandle), EB_MUTEX);
        }

    return EB_ErrorNone;
}



void enc_dec_segments_init(
    EncDecSegments_t *segments_ptr,
    uint32_t            segColCount,
    uint32_t            segRowCount,
    uint32_t            pic_width_lcu,
    uint32_t            pic_height_lcu)
{
    unsigned x, y, yLast;
    unsigned row_index, bandIndex, segment_index;

    segments_ptr->lcuRowCount = pic_height_lcu;
    segments_ptr->lcuBandCount = BAND_TOTAL_COUNT(pic_height_lcu, pic_width_lcu);
    segments_ptr->segment_row_count = segRowCount;
    segments_ptr->segment_band_count = BAND_TOTAL_COUNT(segRowCount, segColCount);
    segments_ptr->segmentTotalCount = segments_ptr->segment_row_count * segments_ptr->segment_band_count;

    //EB_MEMSET(segments_ptr->inputMap.inputDependencyMap, 0, sizeof(uint16_t) * segments_ptr->segmentTotalCount);
    EB_MEMSET(segments_ptr->validLcuCountArray, 0, sizeof(uint16_t) * segments_ptr->segmentTotalCount);
    EB_MEMSET(segments_ptr->xStartArray, -1, sizeof(uint16_t) * segments_ptr->segmentTotalCount);
    EB_MEMSET(segments_ptr->yStartArray, -1, sizeof(uint16_t) * segments_ptr->segmentTotalCount);

    // Initialize the per-LCU input availability map & Start Arrays
    for (y = 0; y < pic_height_lcu; ++y) {
        for (x = 0; x < pic_width_lcu; ++x) {
            bandIndex = BAND_INDEX(x, y, segments_ptr->segment_band_count, segments_ptr->lcuBandCount);
            row_index = ROW_INDEX(y, segments_ptr->segment_row_count, segments_ptr->lcuRowCount);
            segment_index = SEGMENT_INDEX(row_index, bandIndex, segments_ptr->segment_band_count);

            //++segments_ptr->inputMap.inputDependencyMap[segment_index];
            ++segments_ptr->validLcuCountArray[segment_index];
            segments_ptr->xStartArray[segment_index] = (segments_ptr->xStartArray[segment_index] == (uint16_t)-1) ?
                (uint16_t)x :
                segments_ptr->xStartArray[segment_index];
            segments_ptr->yStartArray[segment_index] = (segments_ptr->yStartArray[segment_index] == (uint16_t)-1) ?
                (uint16_t)y :
                segments_ptr->yStartArray[segment_index];
        }
    }

    // Initialize the row-based controls
    for (row_index = 0; row_index < segments_ptr->segment_row_count; ++row_index) {
        y = ((row_index * segments_ptr->lcuRowCount) + (segments_ptr->segment_row_count - 1)) / segments_ptr->segment_row_count;
        yLast = ((((row_index + 1) * segments_ptr->lcuRowCount) + (segments_ptr->segment_row_count - 1)) / segments_ptr->segment_row_count) - 1;
        bandIndex = BAND_INDEX(0, y, segments_ptr->segment_band_count, segments_ptr->lcuBandCount);

        segments_ptr->rowArray[row_index].startingSegIndex = (uint16_t)SEGMENT_INDEX(row_index, bandIndex, segments_ptr->segment_band_count);
        bandIndex = BAND_INDEX(pic_width_lcu - 1, yLast, segments_ptr->segment_band_count, segments_ptr->lcuBandCount);
        segments_ptr->rowArray[row_index].endingSegIndex = (uint16_t)SEGMENT_INDEX(row_index, bandIndex, segments_ptr->segment_band_count);
        segments_ptr->rowArray[row_index].currentSegIndex = segments_ptr->rowArray[row_index].startingSegIndex;
    }

    // Initialize the per-segment dependency map
    EB_MEMSET(segments_ptr->depMap.dependencyMap, 0, sizeof(uint8_t) * segments_ptr->segmentTotalCount);
    for (row_index = 0; row_index < segments_ptr->segment_row_count; ++row_index) {
        for (segment_index = segments_ptr->rowArray[row_index].startingSegIndex; segment_index <= segments_ptr->rowArray[row_index].endingSegIndex; ++segment_index) {

            // Check that segment is valid
            if (segments_ptr->validLcuCountArray[segment_index]) {
                // Right Neighbor
                if (segment_index < segments_ptr->rowArray[row_index].endingSegIndex) {
                    ++segments_ptr->depMap.dependencyMap[segment_index + 1];
                }
                // Bottom Neighbor
                if (row_index < segments_ptr->segment_row_count - 1 && segment_index + segments_ptr->segment_band_count >= segments_ptr->rowArray[row_index + 1].startingSegIndex) {
                    ++segments_ptr->depMap.dependencyMap[segment_index + segments_ptr->segment_band_count];
                }
            }
        }
    }

    return;
}


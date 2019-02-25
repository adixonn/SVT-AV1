/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#ifndef EbPictureManagerQueue_h
#define EbPictureManagerQueue_h

#include "EbDefinitions.h"
#include "EbDefinitions.h"
#include "EbSystemResourceManager.h"
#include "EbPredictionStructure.h"
#include "EbApiSei.h"

#ifdef __cplusplus
extern "C" {
#endif
    /************************************************
     * Input Queue Entry
     ************************************************/
    struct ReferenceQueueEntry;   // empty struct definition

    typedef struct InputQueueEntry {
        EbObjectWrapper              *inputObjectPtr;
        uint32_t                          dependentCount;
        uint32_t                          referenceEntryIndex;
        ReferenceList                *list0Ptr;
        ReferenceList                *list1Ptr;
        uint32_t                          useCount;
        EbBool                         memoryMgmtLoopDone;
        EbBool                         rateControlLoopDone;
        EbBool                         encodingHasBegun;

    } InputQueueEntry;

    /************************************************
     * Reference Queue Entry
     ************************************************/
    typedef struct ReferenceQueueEntry {

        uint64_t                          picture_number;
        uint64_t                          decode_order;
        EbObjectWrapper              *referenceObjectPtr;
        uint32_t                          dependentCount;
        EbBool                         releaseEnable;
        EbBool                         referenceAvailable;
        uint32_t                          depList0Count;
        uint32_t                          depList1Count;
        DependentList                 list0;
        DependentList                 list1;
        EbBool                         is_used_as_reference_flag;

        uint64_t                          rcGroupIndex;

    } ReferenceQueueEntry_t;

    /************************************************
     * Rate Control Input Queue Entry
     ************************************************/

    typedef struct RcInputQueueEntry {
        uint64_t                          picture_number;
        EbObjectWrapper              *inputObjectPtr;

        EbBool                         isPassed;
        EbBool                         releaseEnabled;
        uint64_t                          groupId;
        uint64_t                          gopFirstPoc;
        uint32_t                          gopIndex;


    } RcInputQueueEntry;

    /************************************************
     * Rate Control FeedBack  Queue Entry
     ************************************************/
    typedef struct RcFeedbackQueueEntry {

        uint64_t                          picture_number;
        EbObjectWrapper              *feedbackObjectPtr;

        EbBool                         isAvailable;
        EbBool                         isUpdated;
        EbBool                         releaseEnabled;
        uint64_t                          groupId;
        uint64_t                          gopFirstPoc;
        uint32_t                          gopIndex;

    } RcFeedbackQueueEntry;

    extern EbErrorType input_queue_entry_ctor(
        InputQueueEntry **entry_dbl_ptr);



    extern EbErrorType reference_queue_entry_ctor(
        ReferenceQueueEntry_t  **entry_dbl_ptr);


#ifdef __cplusplus
}
#endif
#endif // EbPictureManagerQueue_h
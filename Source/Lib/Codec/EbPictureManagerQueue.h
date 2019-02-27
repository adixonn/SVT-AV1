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
    struct ReferenceQueueEntry; // empty struct definition

    typedef struct InputQueueEntry 
    {
        EbObjectWrapper *input_object_ptr;
        uint32_t         dependent_count;
        uint32_t         reference_entry_index;
        ReferenceList   *list0_ptr;
        ReferenceList   *list1_ptr;
    } InputQueueEntry;

    /************************************************
     * Reference Queue Entry
     ************************************************/
    typedef struct ReferenceQueueEntry 
    {
        uint64_t           picture_number;
        uint64_t           decode_order;
        EbObjectWrapper   *reference_object_ptr;
        uint32_t           dependent_count;
        EbBool             release_enable;
        EbBool             referenceAvailable;
        uint32_t           dep_list0_count;
        uint32_t           dep_list1_count;
        DependentList      list0;
        DependentList      list1;
        EbBool             is_used_as_reference_flag;
    } ReferenceQueueEntry;

    extern EbErrorType input_queue_entry_ctor(
        InputQueueEntry **entry_dbl_ptr);

    extern EbErrorType reference_queue_entry_ctor(
        ReferenceQueueEntry **entry_dbl_ptr);


#ifdef __cplusplus
}
#endif
#endif // EbPictureManagerQueue_h
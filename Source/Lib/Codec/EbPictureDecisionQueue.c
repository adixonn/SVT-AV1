/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#include <stdlib.h>
#include "EbPictureDecisionQueue.h"


EbErrorType pa_reference_queue_entry_ctor(
    PaReferenceQueueEntry   **entry_dbl_ptr)
{
    PaReferenceQueueEntry *entryPtr;
    EB_MALLOC(PaReferenceQueueEntry*, entryPtr, sizeof(PaReferenceQueueEntry), EB_N_PTR);
    *entry_dbl_ptr = entryPtr;

    entryPtr->inputObjectPtr = (EbObjectWrapper_t*)EB_NULL;
    entryPtr->picture_number = 0;
    entryPtr->referenceEntryIndex = 0;
    entryPtr->dependentCount = 0;
    entryPtr->list0Ptr = (ReferenceList*)EB_NULL;
    entryPtr->list1Ptr = (ReferenceList*)EB_NULL;
    EB_MALLOC(int32_t*, entryPtr->list0.list, sizeof(int32_t) * (1 << MAX_TEMPORAL_LAYERS), EB_N_PTR);

    EB_MALLOC(int32_t*, entryPtr->list1.list, sizeof(int32_t) * (1 << MAX_TEMPORAL_LAYERS), EB_N_PTR);

    return EB_ErrorNone;
}



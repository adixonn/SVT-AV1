/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#ifndef EbEncDecTasks_h
#define EbEncDecTasks_h

#include "EbDefinitions.h"
#include "EbSystemResourceManager.h"
#ifdef __cplusplus
extern "C" {
#endif
#define ENCDEC_TASKS_MDC_INPUT      0
#define ENCDEC_TASKS_ENCDEC_INPUT   1
#define ENCDEC_TASKS_CONTINUE       2

    /**************************************
     * Process Results
     **************************************/
    typedef struct EncDecTasks
    {
        EbObjectWrapper_t            *pictureControlSetWrapperPtr;
        uint32_t                        inputType;
        int16_t                        enc_dec_segment_row;

    } EncDecTasks;

    typedef struct EncDecTasksInitData {
        unsigned encDecSegmentRowCount;
    } EncDecTasksInitData;

    /**************************************
     * Extern Function Declarations
     **************************************/
    extern EbErrorType enc_dec_tasks_ctor(
        EbPtr *object_dbl_ptr,
        EbPtr object_init_data_ptr);


#ifdef __cplusplus
}
#endif
#endif // EbEncDecTasks_h

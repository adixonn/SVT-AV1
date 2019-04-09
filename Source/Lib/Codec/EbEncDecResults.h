/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#ifndef EbEncDecResults_h
#define EbEncDecResults_h

#include "EbDefinitions.h"
#include "EbSystemResourceManager.h"
#ifdef __cplusplus
extern "C" {
#endif
    /**************************************
     * Process Results
     **************************************/
    typedef struct EncDecResults
    {
        EbObjectWrapper *picture_control_set_wrapper_ptr;
        uint32_t         completed_lcu_row_index_start;
        uint32_t         completed_lcu_row_count;

    } EncDecResults;

#if FILT_PROC
    typedef struct DlfResults
    {
        EbObjectWrapper *picture_control_set_wrapper_ptr;
      
#if CDEF_M
        uint32_t         segment_index;
#endif

    } DlfResults;

    typedef struct CdefResults
    {
        EbObjectWrapper      *picture_control_set_wrapper_ptr;
       
#if REST_M
        uint32_t          segment_index;
#endif

    } CdefResults;

    typedef struct RestResults
    {
        EbObjectWrapper      *picture_control_set_wrapper_ptr;
        uint32_t              completed_lcu_row_index_start;
        uint32_t              completed_lcu_row_count;

    } RestResults;
#endif
    typedef struct EncDecResultsInitData {
        uint32_t junk;
    } EncDecResultsInitData;

    /**************************************
     * Extern Function Declarations
     **************************************/
    extern EbErrorType enc_dec_results_ctor(
        EbPtr *object_dbl_ptr,
        EbPtr  object_init_data_ptr);


#ifdef __cplusplus
}
#endif
#endif // EbEncDecResults_h